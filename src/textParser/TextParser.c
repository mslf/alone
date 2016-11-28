/*
	Copyright 2016 Golikov Vitaliy

	This file is part of Alone.

	Alone is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Alone is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Alone. If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file TextParser.c
 * @author mslf
 * @date 6 Oct 2016
 * @brief File containing implementation of #TextParser.
 */
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "textParser/TextParser.h"

#define TEXT_PARSER_HEADER_COMMENT "This text resource was generated by TextParser from Alone."

/**
 * @brief Error message strings for #TextParser.
 */
static const struct TextParser_errorMessages {
    const char* const errValueStringAlloc;
    /**< Will be displayed when allocating memory for left or right value strings while parsing #TextResource failed. */
    const char* const errDeletingComments;
    /**< Will be displayed when syntax error or unexpected EOF happened while deleting comment blocks. */
    const char* const errUnexpectedEofWhileSplittingAssigment;
    /**< Will be displayed when unexpected EOF happened while spliting assigment expression. */
    const char* const errSpliting;
    /**< Will be displayed when syntax error happened while spliting assigment expression. */
    const char* const errPairsRealloc;
    /**< Will be displayed when allocating memory for TextParser#pairsList failed. */
    const char* const errItemsRealloc;
    /**< Will be displayed when allocating memory for RightValue#itemsList failed. */
    const char* const errStringAlloc;
    /**< Will be displayed when reallocating memory for some string failed. */
    const char* const errOddQuotesCount;
    /**< Will be displayed when right value string has odd count of quotes. */
    const char* const errUnexpectedEofWhileSplittingArray;
    /**< Will be displayed when unexpected EOF happened while spliting items array for #RightValue. */
    const char* const errBrackets;
    /**< Will be displayed when opening or closing square brackets haven't found while 
     *spliting items array for #RightValue. */
    const char* const errAddingItem;
    /**< Will be displayed when adding item string to #RightValue failed. */
}TextParser_errorMessages = {
    "TextParser_parseTextResource: allocating memory for left or right value strings failed!",
    "TextParser_parseTextResource: syntax error or unexpected EOF while deleting comments!",
    "TextParser_parseTextResource: unexpected EOF while splitting assigment!",
    "TextParser_parseTextResource: syntax error while splitting assigment!",
    "TextParser_reallocatePairsList: allocating memory for pairsList failed!",
    "TextParser_reallocateItemsList: allocating memory for itemsList failed!",
    "TextParser_reallocateString: allocating memory for string failed!",
    "TextParser_addPair: syntax error -  odd count of quotes!",
    "TextParser_addPair: unexpected EOF while parsing items array string!",
    "TextParser_addPair: syntax error - opening or closing square-brackets haven't found!",
    "TextParser_addItemToRightValue: adding item failed!"};

/**
 * @brief Reallocates TextParser#pairsList and increases TextParser#allocatedPairsCount 
 * by TextParser_constants#TP_INITIAL_NUMBER_ALLOCATED_PAIRS.
 * @param logger Pointer to a #Logger for logging purpose. Can be NULL.
 * @param textParser Pointer to a #TextParser where TextParser#pairsList will be reallocated.
 * @return #TextParser_errors value.
 * @see #TextParser_errors
 * @see #TextParser_constants
 * @see #TextParser_errorMesssages
 */
static enum TextParser_errors TextParser_reallocatePairsList(struct Logger* logger, struct TextParser* textParser) {
    assert(textParser);
    struct Pair* newPairsList = NULL;
    size_t newSize = textParser->allocatedPairsCount + TP_INITIAL_NUMBER_ALLOCATED_PAIRS;
    // Not realloc because allocating of pair is a two-step process.
    newPairsList = (struct Pair*)malloc(sizeof(struct Pair) * newSize);
    if (!newPairsList) {
        Logger_log(logger, TextParser_errorMessages.errPairsRealloc);
        return TEXT_PARSER_ERR_REALLOC_PAIRS_LIST;
    }
    for (size_t i = 0; i < textParser->pairsCount; i++)
        newPairsList[i] = textParser->pairsList[i];
    for (size_t i = textParser->allocatedPairsCount; i < newSize; i++){
        newPairsList[i].rightValue.itemsCount = 0;
        if (!(newPairsList[i].rightValue.itemsList =
                    (char**)malloc(sizeof(char*) * TP_INITIAL_NUMBER_ALLOCATED_ITEMS))) {
            free(newPairsList);
            return TEXT_PARSER_ERR_ALLOC_STRING;
        }
        newPairsList[i].rightValue.allocatedItemsCount = TP_INITIAL_NUMBER_ALLOCATED_ITEMS;
    }
    free(textParser->pairsList);
    textParser->pairsList = newPairsList;
    textParser->allocatedPairsCount = newSize;
    return TEXT_PARSER_NO_ERRORS;
}

/**
 * @brief Reallocates RightValue#itemsList and increases RightValue#itemsCount 
 * by TextParser_constants#TP_INITIAL_NUMBER_ALLOCATED_ITEMS.
 * @param logger Pointer to a #Logger for logging purpose. Can be NULL.
 * @param rightValue Pointer to a #RightValue where RightValue#itemsList will be reallocated.
 * @return #TextParser_errors value.
 * @see #TextParser_errors
 * @see #TextParser_constants
 * @see #TextParser_errorMesssages
 */
static enum TextParser_errors TextParser_reallocateItemsList(struct Logger* logger, struct RightValue* rightValue) {
    assert(rightValue);
    char** newItemsList = NULL;
    size_t newSize = rightValue->allocatedItemsCount + TP_INITIAL_NUMBER_ALLOCATED_ITEMS;
    newItemsList = (char**)realloc(rightValue->itemsList, sizeof(char*) * newSize);
    if (!newItemsList) {
        Logger_log(logger, TextParser_errorMessages.errItemsRealloc);
        return TEXT_PARSER_ERR_REALLOC_ITEMS_LIST;
    }
    rightValue->itemsList = newItemsList;
    rightValue->allocatedItemsCount = newSize;
    return TEXT_PARSER_NO_ERRORS;
}

/**
 * @brief Reallocates string and updates its length counter by step.
 * @param logger Pointer to a #Logger for logging purpose. Can be NULL.
 * @param string Pointer to a string (char**) to be reallocated.
 * @param oldLength Pointer to a number, which is an old string length. Will be updated.
 * @param step Reallocation step.
 * @return #TextParser_errors value.
 * @see #TextParser_errors
 * @see #TextParser_constants
 * @see #TextParser_errorMesssages
 */
static enum TextParser_errors TextParser_reallocateString(struct Logger* logger, char** string, size_t* oldLength, size_t step) {
    assert(string);
    assert(oldLength);
    assert(step != 0);
    char* newString = NULL;
    size_t newLength = (*oldLength) + step;
    newString = (char*)realloc((*string), sizeof(char) * newLength);
    if (!newString) {
        Logger_log(logger, TEXT_PARSER_ERR_STRING_REALLOC);
        return TEXT_PARSER_ERR_ALLOC_STRING;
    }
    (*string) = newString;
    (*oldLength) = newLength;
    return TEXT_PARSER_NO_ERRORS;
}

/**
 * @brief Splits ';'-terminated assigment for two (left and right) parts.
 * Sets given start index (where function starts to split assigment) to the position of the last readed symbol + 1.
 * Also, sets left and right value string's lengths (and their allocated counter, if needed).
 * @param logger Pointer to a #Logger for logging purpose. Can be NULL.
 * @param string Given string, which will be parsed for assigment.
 * @param startIndex Pointer to a number, from which position this function will start parsing string. 
 * Will be updated while parsing.
 * @param leftValueString Pointer to a string (char**), where this function will store parsed left value from assigment. 
 * Can be reallocated, if needed.
 * @param leftCounter Pointer to a number, which is represents current existing chars in leftValueString. 
 * Will be updated while parsing.
 * @param allocatedCharsForLeftValue Pointer to a value, which is represents current allocated chars in leftValueString. 
 * Will be updated while reallocating leftValueString (if needed).
 * @param rightValueString Pointer to a string (char**), where this function will store parsed right value from assigment. 
 * Can be reallocated, if needed.
 * @param rightCounter Pointer to a number, which is represents current existing chars in rightValueString. 
 * Will be updated while parsing.
 * @param allocatedCharsForRightValue Pointer to a value, which is represents current allocated chars in rightValueString. 
 * Will be updated while reallocating rightValueString (if needed).
 * @return #TextParser_errors value.
 * @see #TextParser_errors
 * @see #TextParser_reallocateString
 * @see #TextParser_constants
 * @see #TextParser_errorMesssages
 */
static enum TextParser_errors TextParser_splitExpression(struct Logger* logger,
                                                         const char* const string,
                                                         size_t* const startIndex,
                                                         char** const leftValueString,
                                                         size_t* const leftCounter,
                                                         size_t* const allocatedCharsForLeftValue,
                                                         char** const rightValueString,
                                                         size_t* const rightCounter,
                                                         size_t* const allocatedCharsForRightValue) {
    assert(string);
    assert(startIndex);
    assert(leftValueString && leftCounter && allocatedCharsForLeftValue);
    assert(rightValueString && rightCounter && allocatedCharsForRightValue);
    enum SplittingStates {
        SPLITTING_STATE_START = 0,
        SPLITTING_ON_LEFT_PART = 1,
        SPLITTING_ON_RIGHT_PART = 2,
        SPLITTING_ON_END = 3,
        SPLITTING_ERR = 4
    } state = SPLITTING_STATE_START;
    while ((*startIndex) < strlen(string) && state != SPLITTING_ON_END && state != SPLITTING_ERR) {
        char c = string[*startIndex];
        if ((*leftCounter) >= (*allocatedCharsForLeftValue))
            if (TextParser_reallocateString(logger, leftValueString, allocatedCharsForLeftValue,
                                            TP_INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_LEFT_VALUE_STRING))
                return TEXT_PARSER_ERR_ALLOC_STRING;
        if ((*rightCounter) >= (*allocatedCharsForRightValue))
            if (TextParser_reallocateString(logger, rightValueString, allocatedCharsForRightValue,
                                            TP_INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_RIGHT_VALUE_STRING))
                return TEXT_PARSER_ERR_ALLOC_STRING;
        //? You can use goto with case labels.
        // FIXME hmmmm.
        switch (state) {
            case SPLITTING_STATE_START:
                if (c == ';'){
                    state = SPLITTING_ERR;
                    break;
                }
                if (c != ' ' && c!= '\t' && c!= '\n') {
                    state = SPLITTING_ON_LEFT_PART;
                    (*leftValueString)[*leftCounter] = c;
                    (*leftCounter)++;
                }
                break;
            case SPLITTING_ON_LEFT_PART:
                if (c == '=' && (*leftCounter) > 0) {
                    state = SPLITTING_ON_RIGHT_PART;
                    break;
                }
                if (c == '=' && (*leftCounter) == 0) {
                    state = SPLITTING_ERR;
                    break;
                }
                if (c == '\n' || c == ';') {
                    state = SPLITTING_ERR;
                    break;
                }
                if (c != ' ' && c != '\t') {
                    (*leftValueString)[*leftCounter] = c;
                    (*leftCounter)++;
                }
                break;
            case SPLITTING_ON_RIGHT_PART:
                if (c == ';' && (*rightCounter) > 0) {
                    state = SPLITTING_ON_END;
                    break;
                }
                if (c == ';' && (*rightCounter) == 0) {
                    state = SPLITTING_ERR;
                    break;
                }
                (*rightValueString)[*rightCounter] = c;
                (*rightCounter)++;
                break;
            case SPLITTING_ON_END:
                break;
            case SPLITTING_ERR:
                break;
        }
        (*startIndex)++;
    }
    if (state == SPLITTING_ON_END)
        return TEXT_PARSER_NO_ERRORS;
    if (state == SPLITTING_ERR)
        return TEXT_PARSER_ERR_SPLITTING_ASSIGMENT;
    return TEXT_PARSER_ERR_SPLITTING_ASSIGMENT_EOF;
}

static unsigned char TextParser_deleteNonQuotedSpaces(char* rightValueString, size_t* rightCounter) {
    if (!rightValueString || !rightCounter)
        return 2;
    unsigned char state = 0;
    size_t i = 0;
    size_t j = 0;
    size_t quotesCount = 0;
    while (i < (*rightCounter)) {
        char c = rightValueString[i];
        switch (state) {
            case 0: // notInQuotes
                if (c == '\"') {
                    state = 1;
                    quotesCount++;
                    break;
                }
                if (c == ' ' || c == '\t' || c == '\n') {
                    for (j = i; j < (*rightCounter) - 1; j++)
                        rightValueString[j] = rightValueString[j + 1];
                    (*rightCounter)--;
                    i--;
                }
                break;
            case 1: // inQuotes
                if (c == '\"') {
                    state = 0;
                    quotesCount++;
                    break;
                }
                if (c == '\n') {
                    for (j = i; j < (*rightCounter) - 1; j++)
                        rightValueString[j] = rightValueString[j + 1];
                    (*rightCounter)--;
                    i--;
                }
                break;
        }
        i++;
    }
    return (quotesCount % 2);
}

static unsigned char TextParser_addItemToRightValue(struct Logger* logger, struct Pair* pair,
                                                      const char* const itemString,
                                                      size_t itemStringLength) {
    if (!pair || !itemString || !itemStringLength)
        return 1;
    size_t i = 0;
    if (pair->rightValue.itemsCount >=
            pair->rightValue.allocatedItemsCount)
        if (TextParser_reallocateItemsList(logger, &(pair->rightValue)))
            return 2;
    pair->rightValue.itemsList[pair->rightValue.itemsCount] =
            (char*)malloc(sizeof(char) * (itemStringLength + 1));
    if (!pair->rightValue.itemsList[pair->rightValue.itemsCount]) {
        Logger_log(logger, TEXT_PARSER_ERR_ADD_ITEM);
        return 3;
    }
    for (i = 0; i < itemStringLength; i ++)
        pair->rightValue.itemsList[pair->rightValue.itemsCount][i] = itemString[i];
    pair->rightValue.itemsList[pair->rightValue.itemsCount][itemStringLength] = 0;
    pair->rightValue.itemsCount++;
    return 0;
}

static unsigned char TextParser_parseItemsArrayString(struct Logger* logger, struct Pair* pair, char* itemsString,
                                               size_t itemsStringLength) {
    if (!pair || !itemsString || !itemsStringLength)
        return 6;
    unsigned char state = 0;
    size_t i = 1; // start from first symbol, because zero-zymbol is '['
    char* tempItemString = NULL;
    tempItemString = (char*)malloc(sizeof(char) * INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_ITEM_STRING);
    if (!tempItemString)
        return 7;
    size_t allocatedCharsForTempItemString = INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_ITEM_STRING;
    size_t charsCounter = 0;
    while (i < itemsStringLength && state != 3 && state != 4) {
        char c = itemsString[i];
        if (charsCounter >= allocatedCharsForTempItemString)
            if (TextParser_reallocateString(logger, &tempItemString, &allocatedCharsForTempItemString,
                                            INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_ITEM_STRING)) {
                free(tempItemString);
                return 5;
            }
        switch (state) {
            case 0: // buildingItemString_NotInQuotes
                if (c == '\"') {
                    state = 1;
                    break;
                }
                if (c == ']') {
                    if (charsCounter > 0) {
                        TextParser_addItemToRightValue(logger, pair, tempItemString, charsCounter);
                        charsCounter = 0;
                        state = 3;
                    } else
                        state = 4;
                    break;
                }
                if (c == ',') {
                    TextParser_addItemToRightValue(logger, pair, tempItemString, charsCounter);
                    charsCounter = 0;
                    break;
                }
                tempItemString[charsCounter] = c;
                charsCounter++;
                break;
            case 1: // buildingItemString_InQuotes
                if (c == '\"') {
                    if (charsCounter > 0) {
                        TextParser_addItemToRightValue(logger, pair, tempItemString, charsCounter);
                        charsCounter = 0;
                        state = 2;
                    } else
                        state = 4;
                    break;
                }
                tempItemString[charsCounter] = c;
                charsCounter++;
                break;
            case 2: // checkingEnd
                if (c == ',') {
                    state = 0;
                    break;
                }
                if (c == ']') {
                    state = 3;
                    break;
                }
                state = 4;
                break;
            case 3: // end
                break;
            case 4: // syntax error
                break;
        }
        i++;
    }
    free(tempItemString);
    return state;
}

static void TextParser_destructTempOperandStrings(char* left, char* right) {
    if (left)
        free(left);
    if (right)
        free(right);
}

static unsigned char TextParser_addPair(struct Logger* logger, struct TextParser* textParser, char* leftValueString,
                                 size_t leftCounter, char* rightValueString, size_t rightCounter) {
    if (!textParser || !leftValueString || !leftCounter || !rightValueString || !rightCounter)
        return 1;
    size_t j;
    // adding leftValueString string as a leftValue to the pair
    if (textParser->pairsCount >= textParser->allocatedPairsCount)
        if (TextParser_reallocatePairsList(logger, textParser)) {
            TextParser_destructTempOperandStrings(leftValueString, rightValueString);
            return  2;
        }
    textParser->pairsList[textParser->pairsCount].leftValue = (char*)malloc(sizeof(char) * (leftCounter + 1));
    if (!textParser->pairsList[textParser->pairsCount].leftValue) {
        Logger_log(logger, TEXT_PARSER_ERR_LEFT_OPERAND_STRING_ALLOC);
        TextParser_destructTempOperandStrings(leftValueString, rightValueString);
        return 3;
    }
    for (j = 0; j < leftCounter; j ++)
        textParser->pairsList[textParser->pairsCount].leftValue[j] =  leftValueString[j];
    textParser->pairsList[textParser->pairsCount].leftValue[leftCounter] = 0;
    // Working with rightValue
    if (TextParser_deleteNonQuotedSpaces(rightValueString, &rightCounter)) {
        Logger_log(logger, TEXT_PARSER_ERR_SYNTAX_QUOTES);
        TextParser_destructTempOperandStrings(leftValueString, rightValueString);
        return 4;
    }
    if (rightValueString[0] == '[' && rightValueString[rightCounter - 1] == ']') {
        if (TextParser_parseItemsArrayString(logger, &(textParser->pairsList[textParser->pairsCount]),
                                             rightValueString, rightCounter) != 3) {
            Logger_log(logger, TEXT_PARSER_ERR_UNEXPEXTED_EOF_ITEMS_ARRAY_STRING);
            TextParser_destructTempOperandStrings(leftValueString, rightValueString);
            return 5;
        }
    } else { // adding rightValueString as a first item to the pair
        if (rightValueString[0] == '[' || rightValueString[rightCounter - 1] == ']') {
            Logger_log(logger, TEXT_PARSER_ERR_SYNTAX_BRACKETS);
            TextParser_destructTempOperandStrings(leftValueString, rightValueString);
            return 6;
        }
        if (TextParser_addItemToRightValue(logger, &(textParser->pairsList[textParser->pairsCount]),
                                             rightValueString, rightCounter)) {
            TextParser_destructTempOperandStrings(leftValueString, rightValueString);
            return 7;
        }
    }
    textParser->pairsCount++;
    return 0;
}

static unsigned char TextParser_deleteComments(char* const srcText, char** dstText) {
    if (!srcText || !dstText)
        return 4;
    size_t length = strlen(srcText);
    *(dstText) = (char*)malloc(sizeof(char) * (length + 1));
    if (!(*dstText))
        return 5;
    unsigned char state = 0;
    size_t counter = 0;
    size_t i = 0;
    while (i < length) {
        char c = srcText[i];
        switch (state) {
            case 0: // normal text
                if (c == '/') {
                    state = 1;
                    break;
                }
                (*dstText)[counter] = c;
                counter++;
                break;
            case 1: // on "/" (entering comment block)
                if (c == '*') {
                    state = 2;
                    break;
                }
                (*dstText)[counter] = '/';
                counter++;
                (*dstText)[counter] = c;
                counter++;
                state = 0;
                break;
            case 2: // in comment block
                if (c == '*')
                    state = 3;
                break;
            case 3: // on "*" (exiting comment block)
                if (c == '/') {
                    state = 0;
                    break;
                }
                state = 2;
                break;
        }
        i++;
    }
    (*dstText)[counter] = 0;
    return state;
}

static unsigned char TextParser_parseTextResource(struct Logger* logger, struct TextParser* textParser,
                                           const struct TextResource* const textResource) {
    if (!textParser || !textResource)
        return 1;
    char* leftValueString = NULL;
    char* rightValueString = NULL;
    leftValueString = (char*)malloc(sizeof(char) * INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_LEFT_OPERAND_STRING);
    rightValueString = (char*)malloc(sizeof(char) * INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_RIGHT_OPERAND_STRING);
    if (!leftValueString || !rightValueString) {
        Logger_log(logger, "%s ResourceID: %s", TEXT_PARSER_ERR_OPERANDS_ALLOC, textResource->id);
        TextParser_destructTempOperandStrings(leftValueString, rightValueString);
        return 2;
    }
    size_t allocatedCharsForLeftOperand = INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_LEFT_OPERAND_STRING;
    size_t allocatedCharsForRightValue = INITIAL_NUMBER_ALLOCATED_SYMBOLS_FOR_RIGHT_OPERAND_STRING;
    size_t leftCounter = 0;
    size_t rightCounter = 0;
    size_t i = 0;
    size_t j = 0;
    unsigned char state = 0;
    char* tempText = NULL;
    if (TextParser_deleteComments(textResource->text, &tempText)) {
        Logger_log(logger, TEXT_PARSER_ERR_SYNTAX_DELETE_COMMENTS);
        return 3;
    }
    while (i < strlen(tempText)) {
        leftCounter = 0;
        rightCounter = 0;
        state = TextParser_splitExpression(logger, tempText, &i, &leftValueString, &leftCounter,
                                           &allocatedCharsForLeftOperand, &rightValueString, &rightCounter,
                                           &allocatedCharsForRightValue);
        if (state != 3 && state != 4 && state != 0) {
            Logger_log(logger, "%s ResourceID: %s", TEXT_PARSER_ERR_UNEXPEXTED_EOF_SPLIT_EXPRESSION, textResource->id);
            TextParser_destructTempOperandStrings(leftValueString, rightValueString);
            return 4;
        }
        if (state == 4) {
            Logger_log(logger, "%s ResourceID: %s", TEXT_PARSER_ERR_SYNTAX_SPLIT_EXPRESSION, textResource->id);
            TextParser_destructTempOperandStrings(leftValueString, rightValueString);
            return 5;
        }
        if (state == 3) {
            unsigned char result = TextParser_addPair(logger, textParser, leftValueString, leftCounter,
                                                      rightValueString, rightCounter);
            if (result) {
                Logger_log(logger, "\t in ResourceID: %s", textResource->id);
                return (result + 5);
            }

        }
    }
    free(tempText);
    TextParser_destructTempOperandStrings(leftValueString, rightValueString);
    return 0;
}

struct TextParser* TextParser_constructFromTextResource(struct Logger* logger,
                                                        const struct TextResource* const textResource) {
    if (!textResource)
        return NULL;
    struct TextParser* textParser = NULL;
    textParser = TextParser_constructEmpty();
    if (!textParser)
        return NULL;
    if (TextParser_parseTextResource(logger, textParser, textResource)) {
        TextParser_destruct(textParser);
        return  NULL;
    }
    return textParser;
}

struct TextParser* TextParser_constructEmpty() {
    struct TextParser* textParser = NULL;
    textParser = (struct TextParser*)calloc(1, sizeof(struct TextParser));
    if (!textParser)
        return NULL;
    textParser->pairsCount = 0;
    textParser->pairsList = (struct Pair*)malloc(sizeof(struct Pair) * INITIAL_NUMBER_ALLOCATED_PAIRS);
    if (!textParser->pairsList) {
        TextParser_destruct(textParser);
        return NULL;
    }
    textParser->allocatedPairsCount = INITIAL_NUMBER_ALLOCATED_PAIRS;
    size_t i;
    for (i = 0; i < textParser->allocatedPairsCount; i++){
        textParser->pairsList[i].rightValue.itemsCount = 0;
        if (!(textParser->pairsList[i].rightValue.itemsList =
                    (char**)malloc(sizeof(char*) * INITIAL_NUMBER_ALLOCATED_ITEMS))) {
            TextParser_destruct(textParser);
            return NULL;
        }
        textParser->pairsList[i].rightValue.allocatedItemsCount = INITIAL_NUMBER_ALLOCATED_ITEMS;
    }
    textParser->lastError = NoError;
    return textParser;
}

void TextParser_destruct(struct TextParser* textParser) {
    if (!textParser)
        return;
    if (textParser->pairsList) {
        size_t i;
        size_t j;
        for (i = 0; i < textParser->allocatedPairsCount; i++){
            if (i < textParser->pairsCount)
                free(textParser->pairsList[i].leftValue);
            if (i < textParser->pairsCount)
                for (j = 0; j < textParser->pairsList[i].rightValue.itemsCount; j++)
                    free(textParser->pairsList[i].rightValue.itemsList[j]);
            free(textParser->pairsList[i].rightValue.itemsList);
        }
        free(textParser->pairsList);
    }
    free(textParser);
}

size_t TextParser_getItemsCount(struct TextParser* textParser, const char* const leftValue) {
    if (!textParser || !leftValue)
        return 0;
    size_t i = 0;
    for (i = 0; i < textParser->pairsCount; i++)
        if (strcmp(leftValue, textParser->pairsList[i].leftValue) == 0) {
            textParser->lastError = NoError;
            return textParser->pairsList[i].rightValue.itemsCount;
        }
    textParser->lastError = NoLeftOperandError;
    return 0;
}

const char* TextParser_getString(struct TextParser* textParser, const char* const leftValue, size_t index) {
    if (!textParser)
        return NULL;
    if (!leftValue) {
        textParser->lastError = NoLeftOperandError;
        return NULL;
    }
    size_t i = 0;
    unsigned char found = 0;
    size_t foundIndex = 0;
    for (i = 0; i < textParser->pairsCount; i++)
        if (strcmp(leftValue, textParser->pairsList[i].leftValue) == 0) {
            found = 1;
            foundIndex = i;
            break;
        }
    if (!found) {
        textParser->lastError = NoLeftOperandError;
        return NULL;
    }
    if (index >= textParser->pairsList[foundIndex].rightValue.itemsCount) {
        textParser->lastError = OutOfRangeError;
        return NULL;
    }
    textParser->lastError = NoError;
    return textParser->pairsList[foundIndex].rightValue.itemsList[index];
}

long int TextParser_getInt(struct TextParser* textParser, const char* const leftValue, size_t index) {
    const char* intString = TextParser_getString(textParser, leftValue, index);
    if (textParser->lastError)
        return 0;
    return strtol(intString, NULL, 10);
}

double TextParser_getDouble(struct TextParser* textParser, const char* leftValue, size_t index) {
    const char* intString = TextParser_getString(textParser, leftValue, index);
    if (textParser->lastError)
        return 0.0;
    return strtod(intString, NULL);
}

bool TextParser_getFlag(struct TextParser* textParser, const char* const leftValue, size_t index){
    const char* intString = TextParser_getString(textParser, leftValue, index);
    if (textParser->lastError)
        return false;
    if (strtol(intString, NULL, 10))
        return true;
    else
        return false;
}

unsigned char TextParser_addString(struct TextParser* textParser, const char* const leftValue, const char* const item) {
    if (!textParser || !leftValue || !item)
        return 1;
    TextParser_getItemsCount(textParser, leftValue); // later, we will use textParser->lastError. Don't remove this.
    size_t found = 0;
    size_t index = 0;
    size_t i = 0;
    if (textParser->lastError == NoLeftOperandError) {
        if (textParser->pairsCount >= textParser->allocatedPairsCount)
            if (TextParser_reallocatePairsList(NULL, textParser)) {
                textParser->lastError = MemoryAllocationError;
                return 2;
            }
        char* tempLeftOperandString = NULL;
        tempLeftOperandString = (char*)malloc(sizeof(char) * (strlen(leftValue) + 1));
        if (!tempLeftOperandString) {
            textParser->lastError = MemoryAllocationError;
            return 3;
        }
        strcpy(tempLeftOperandString, leftValue);
        textParser->pairsList[textParser->pairsCount].leftValue = tempLeftOperandString;
        index = textParser->pairsCount;
        textParser->pairsCount++;
    } else if (textParser->lastError == NoError)
        for (i = 0; i < textParser->pairsCount; i++)
            if (strcmp(leftValue, textParser->pairsList[i].leftValue) == 0) {
                found = 1;
                index = i;
                break;
            }
    if (TextParser_addItemToRightValue(NULL, &(textParser->pairsList[index]), item, strlen(item))) {
        textParser->lastError = MemoryAllocationError;
        if (!found) {
            free (textParser->pairsList[index].leftValue);
            textParser->pairsCount--;
        }
        return 4;
    }
    textParser->lastError = NoError;
    return 0;
}

unsigned char TextParser_addInt(struct TextParser* textParser, const char* const leftValue, long int item) {
    char tempString[100];
    sprintf(tempString, "%ld", item);
    return TextParser_addString(textParser, leftValue, tempString);
}

unsigned char TextParser_addDouble(struct TextParser* textParser, const char* leftValue, double item) {
    char tempString[100];
    sprintf(tempString, "%f", item);
    return TextParser_addString(textParser, leftValue, tempString);
}

unsigned char TextParser_addFlag(struct TextParser* textParser, const char* const leftValue, bool item) {
    char tempString[2];
    if (item)
        sprintf(tempString, "1");
    else
        sprintf(tempString, "0");
    return TextParser_addString(textParser, leftValue, tempString);
}

static unsigned char TextParser_checkWroteCounter(struct TextParser* textParser, size_t wrote, size_t* counter,
                                           size_t* allocatedLength, char** string) {
    if (!textParser || !counter || !allocatedLength || !string)
        return 1;
    if (wrote > 0)
        (*counter) += wrote;
    else {
        textParser->lastError = ConvertingError;
        free((*string));
        return 2;
    }
    if ((*counter) >= (*allocatedLength) / 2)
        if (TextParser_reallocateString(NULL, string, allocatedLength, (*allocatedLength))) {
            textParser->lastError = MemoryAllocationError;
            free((*string));
            return 3;
        }
    return 0;
}

char* TextParser_convertToText(struct TextParser* textParser) {
    if (!textParser)
        return NULL;
    char* tempString = NULL;
    size_t counter = 0;
    size_t allocatedLength = 5000;
    size_t i = 0;
    size_t j = 0;
    size_t wroteBuf = 0;
    tempString = (char*)malloc(sizeof(char) * allocatedLength);
    if (!tempString) {
        textParser->lastError = MemoryAllocationError;
        return NULL;
    }
    wroteBuf = sprintf(&(tempString[counter]), "/*%s*/\n", TEXT_PARSER_HEADER_COMMENT);
    if (TextParser_checkWroteCounter(textParser, wroteBuf, &counter, &allocatedLength, &tempString))
            return  NULL;
    for (i = 0; i < textParser->pairsCount; i++) {
        wroteBuf = sprintf(&(tempString[counter]), "%s = ", textParser->pairsList[i].leftValue);
        if (TextParser_checkWroteCounter(textParser, wroteBuf, &counter, &allocatedLength, &tempString))
            return  NULL;
        wroteBuf = sprintf(&(tempString[counter]), "[\n");
        if (TextParser_checkWroteCounter(textParser, wroteBuf, &counter, &allocatedLength, &tempString))
            return  NULL;
        for (j = 0; j < textParser->pairsList[i].rightValue.itemsCount; j++) {
            if (j == 0)
                wroteBuf = sprintf(&(tempString[counter]), "\t\"%s\"",
                                textParser->pairsList[i].rightValue.itemsList[j]);
            else
                wroteBuf = sprintf(&(tempString[counter]), ",\n\t\"%s\"",
                                textParser->pairsList[i].rightValue.itemsList[j]);
            if (TextParser_checkWroteCounter(textParser, wroteBuf, &counter, &allocatedLength, &tempString))
                return  NULL;
        }
        wroteBuf = sprintf(&(tempString[counter]), "\n];\n");
        if (TextParser_checkWroteCounter(textParser, wroteBuf, &counter, &allocatedLength, &tempString))
            return  NULL;
    }
    return tempString;
}
