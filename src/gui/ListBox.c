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
 * @file ListBox.c
 * @author mslf
 * @date 13 Aug 2016
 * @brief File containing implementation of #ListBox.
 */
#include <assert.h>
#include "gui/ListBox.h"

static const struct ListBoxSceneNode_errorMessages {
    const char* const errNoTextBoxRes;
    const char* const errNoButtonRes;
    const char* const errNoContextMenuRes;
    const char* const errLabelLengthAboveMax;
}ListBoxSceneNode_errorMessages = {
    "ListBox_loadTextBoxResource: textBoxResource string haven't found!",
    "ListBox_loadButtonResource: buttonResource string haven't found!",
    "ListBox_loadContextMenuResource: contextMenuResource string haven't found!",
    "ListBox_loadContextMenuResource: menuOption string length in created ContextMenu > max string length in created TextBox!"};

    
/**
 * @brief Function for loading and constructing ListBox#contextMenu from #TextParser.
 * @param listBox Pointer to a #ListBox, where to construct #ContextMenu.
 * @param resourceManager Pointer to a #ResourceManager for loading #ContextMenu resources.
 * @param renderer Pointer to a #Renderer for constructing #ContextMenu.
 * @param sceneNodeTypesRegistrar Pointer to a #SceneNodeTypesRegistrar for constructing #ContextMenu.
 * @param textParser Pointer to a #TextParser with data strings.
 * @return #ListBoxSceneNode_errors value.
 * @see #ListBox
 * @see #ContextMenu
 * @see #ResourceManager
 * @see #Renderer
 * @see #SceneNodeTypesRegistrar
 * @see #TextParser
 * @see #ListBoxSceneNode_parserStrings
 * @see #ListBoxSceneNode_errors
 */
static enum ListBoxSceneNode_errors ListBox_loadContextMenuResource(struct ListBox* listBox,
                                               struct ResourceManager* resourceManager,
                                               struct Renderer* renderer,
                                               struct SceneNodeTypesRegistrar* sceneNodeTypesRegistrar,
                                               struct TextParser* textParser) {
    assert(listBox);
    assert(resourceManager);
    assert(renderer);
    assert(sceneNodeTypesRegistrar);
    assert(textParser);
    const char* tempResId = TextParser_getString(textParser, ListBoxSceneNode_parserStrings.contextMenuRes, 0);
    if (!tempResId) {
        Logger_log(renderer->logger, ListBoxSceneNode_errorMessages.errNoContextMenuRes);
        return LIST_BOX_ERR_NO_CONTEXT_MENU_RES;
    }
    listBox->contextMenu = (struct ContextMenu*)SceneNodeTypesRegistrar_constructSceneNode(resourceManager,
                                                                renderer,
                                                                sceneNodeTypesRegistrar,
                                                                tempResId,
                                                                ContextMenuSceneNode_parserStrings.type);
    if (!listBox->contextMenu)
        return LIST_BOX_ERR_CONTEXT_MENU_CONSTRUCTING;
    if (listBox->textBox) {
        size_t i;
        for (i = 0; i < listBox->contextMenu->menuOptionsCount; i++)
            if (strlen(listBox->contextMenu->menuOptionsList[i]->label->text) > listBox->textBox->maxLength) {
                Logger_log(renderer->logger, "%s MenuOption string: %s",
                           ListBoxSceneNode_errorMessages.errLabelLengthAboveMax,
                           listBox->contextMenu->menuOptionsList[i]->label->text);
                return LIST_BOX_ERR_CONTEXT_MENU_LABEL_LENGTH_ABOVE_MAX;
            }
    }
    return LIST_BOX_NO_ERRORS;
}            

/**
 * @brief Function for loading and constructing ListBox#button from #TextParser.
 * @param listBox Pointer to a #ListBox, where to construct #Button.
 * @param resourceManager Pointer to a #ResourceManager for loading #Button resources.
 * @param renderer Pointer to a #Renderer for constructing #Button.
 * @param sceneNodeTypesRegistrar Pointer to a #SceneNodeTypesRegistrar for constructing #Button.
 * @param textParser Pointer to a #TextParser with data strings.
 * @return #ListBoxSceneNode_errors value.
 * @see #ListBox
 * @see #Button
 * @see #ResourceManager
 * @see #Renderer
 * @see #SceneNodeTypesRegistrar
 * @see #TextParser
 * @see #ListBoxSceneNode_parserStrings
 * @see #ListBoxSceneNode_errors
 */
static enum ListBoxSceneNode_errors ListBox_loadButtonResource(struct ListBox* listBox,
                                               struct ResourceManager* resourceManager,
                                               struct Renderer* renderer,
                                               struct SceneNodeTypesRegistrar* sceneNodeTypesRegistrar,
                                               struct TextParser* textParser) {
    assert(listBox);
    assert(resourceManager);
    assert(renderer);
    assert(sceneNodeTypesRegistrar);
    assert(textParser);
    const char* tempResId = TextParser_getString(textParser, ListBoxSceneNode_parserStrings.buttonRes, 0);
    if (!tempResId) {
        Logger_log(renderer->logger, ListBoxSceneNode_errorMessages.errNoButtonRes);
        return LIST_BOX_ERR_NO_BUTTON_RES;
    }
    listBox->button = (struct Button*)SceneNodeTypesRegistrar_constructSceneNode(resourceManager,
                                                                renderer,
                                                                sceneNodeTypesRegistrar,
                                                                tempResId,
                                                                ButtonSceneNode_parserStrings.type);
    if (!listBox->button)
        return LIST_BOX_ERR_BUTTON_CONSTRUCTING;
    return LIST_BOX_NO_ERRORS;
}        

/**
 * @brief Function for loading and constructing ListBox#textBox from #TextParser.
 * @param listBox Pointer to a #ListBox, where to construct #TextBox.
 * @param resourceManager Pointer to a #ResourceManager for loading #TextBox resources.
 * @param renderer Pointer to a #Renderer for constructing #TextBox.
 * @param sceneNodeTypesRegistrar Pointer to a #SceneNodeTypesRegistrar for constructing #TextBox.
 * @param textParser Pointer to a #TextParser with data strings.
 * @return #ListBoxSceneNode_errors value.
 * @see #ListBox
 * @see #TextBox
 * @see #ResourceManager
 * @see #Renderer
 * @see #SceneNodeTypesRegistrar
 * @see #TextParser
 * @see #ListBoxSceneNode_parserStrings
 * @see #ListBoxSceneNode_errors
 */
static enum ListBoxSceneNode_errors ListBox_loadTextBoxResource(struct ListBox* listBox,
                                               struct ResourceManager* resourceManager,
                                               struct Renderer* renderer,
                                               struct SceneNodeTypesRegistrar* sceneNodeTypesRegistrar,
                                               struct TextParser* textParser) {
    assert(listBox);
    assert(resourceManager);
    assert(renderer);
    assert(sceneNodeTypesRegistrar);
    assert(textParser);
    const char* tempResId = TextParser_getString(textParser, ListBoxSceneNode_parserStrings.textBoxRes, 0);
    if (!tempResId) {
        Logger_log(renderer->logger, ListBoxSceneNode_errorMessages.errNoTextBoxRes);
        return LIST_BOX_ERR_NO_TEXT_BOX_RES;
    }
    listBox->textBox = (struct TextBox*)SceneNodeTypesRegistrar_constructSceneNode(resourceManager,
                                                                renderer,
                                                                sceneNodeTypesRegistrar,
                                                                tempResId,
                                                                TextBoxSceneNode_parserStrings.type);
    if (!listBox->textBox)
        return LIST_BOX_ERR_TEXT_BOX_CONSTRUCTING;
    return LIST_BOX_NO_ERRORS;
}

/**
 * @brief Function for loading settings and initializing #ListBox from #TextParser.
 * @param listBox Pointer to a #ListBox which will be initialized.
 * @param resourceManager Pointer to a #ResourceManager for loading required resources.
 * @param renderer Pointer to a #Renderer for constructing ListBox#button, ListBox#textBox 
 * and ListBox#contextMenu.
 * @param sceneNodeTypesRegistrar Pointer to a #SceneNodeTypesRegistrar 
 * for constructing ListBox#button, ListBox#textBox and ListBox#contextMenu.
 * @param textParser Pointer to a #TextParser with data strings.
 * @return #ListBoxSceneNode_errors value.
 * @see #ListBox
 * @see #Button
 * @see #TextBox
 * @see #ContextMenu
 * @see #SceneNodeTypesRegistrar
 * @see #ResourceManager
 * @see #Renderer
 * @see #TextParser
 * @see #ListBoxSceneNode_parserStrings
 * @see #ListBoxSceneNode_errors
 */
static enum ListBoxSceneNode_errors ListBox_tryGetSettingsFromTextParser(struct ListBox* listBox,
                                                struct ResourceManager* resourceManager, struct Renderer* renderer,
                                                struct SceneNodeTypesRegistrar* sceneNodeTypesRegistrar,
                                                struct TextParser* textParser) {
    assert(listBox);
    assert(resourceManager);
    assert(renderer);
    assert(sceneNodeTypesRegistrar);
    assert(textParser);
    enum ListBoxSceneNode_errors resultLoadingTextBox = LIST_BOX_NO_ERRORS;
    enum ListBoxSceneNode_errors resultLoadingButton = LIST_BOX_NO_ERRORS;
    enum ListBoxSceneNode_errors resultLoadingContextMenu = LIST_BOX_NO_ERRORS;
    resultLoadingTextBox = ListBox_loadTextBoxResource(listBox, resourceManager,
                                                       renderer, sceneNodeTypesRegistrar, textParser);
    resultLoadingButton = ListBox_loadButtonResource(listBox, resourceManager,
                                                     renderer, sceneNodeTypesRegistrar, textParser);
    resultLoadingContextMenu = ListBox_loadContextMenuResource(listBox, resourceManager,
                                                               renderer, sceneNodeTypesRegistrar, textParser);
    if (resultLoadingTextBox || resultLoadingButton || resultLoadingContextMenu)
        return LIST_BOX_ERR_LOADING_SETTINGS;
    return LIST_BOX_NO_ERRORS;
}

struct SceneNode* ListBox_construct(struct ResourceManager* const resourceManager,
                                    struct Renderer* const renderer,
                                    struct SceneNodeTypesRegistrar* sceneNodeTypesRegistrar,
                                    struct TextParser* const textParser) {
    if (!resourceManager || !renderer || !sceneNodeTypesRegistrar || !textParser)
        return NULL;
    struct ListBox* listBox = NULL;
    listBox = (struct ListBox*)calloc(1, sizeof(struct ListBox));
    if (!listBox)
        return NULL;
    SceneNode_init(&(listBox->sceneNode));
    Logger_saveUsedFlagAndSetToFalse(renderer->logger);
    if (ListBox_tryGetSettingsFromTextParser(listBox, resourceManager, renderer, sceneNodeTypesRegistrar, textParser)) {
        if (renderer->logger->wasUsed)
            Logger_log(renderer->logger, "\tin file: %s", textParser->file);
        Logger_revertUsedFlag(renderer->logger);
        ListBox_destruct((struct SceneNode*)listBox);
        return NULL;
    }
    if (renderer->logger->wasUsed)
        Logger_log(renderer->logger, "\tin file: %s", textParser->file);
    Logger_revertUsedFlag(renderer->logger);
    listBox->sceneNode.control = ListBox_control;
    listBox->sceneNode.update = ListBox_update;
    listBox->sceneNode.render = ListBox_render;
    listBox->sceneNode.sound = ListBox_sound;
    listBox->sceneNode.destruct = ListBox_destruct;
    listBox->sceneNode.type = (char*)malloc(sizeof(char) * (strlen(ListBoxSceneNode_parserStrings.type) + 1));
    if (!listBox->sceneNode.type) {
        ListBox_destruct((struct SceneNode*)listBox);
        return NULL;
    }
    strcpy(listBox->sceneNode.type, ListBoxSceneNode_parserStrings.type);
    listBox->isGeometryChanged = true;
    return (struct SceneNode*)listBox;
}

void ListBox_destruct(struct SceneNode* listBox) {
    if (!listBox)
        return;
    struct ListBox* tempListBox = (struct ListBox*)listBox;
    if (tempListBox->textBox)
        TextBox_destruct((struct SceneNode*)tempListBox->textBox);
    if (tempListBox->button)
        Button_destruct((struct SceneNode*)tempListBox->button);
    if (tempListBox->button)
        ContextMenu_destruct((struct SceneNode*)tempListBox->contextMenu);
    TextResource_decreasePointersCounter(listBox->sceneNodeTextResource);
    if (listBox->type)
        free(listBox->type);
    free(listBox);
}

enum ListBoxSceneNode_errors ListBox_save(const struct ListBox* const listBox,
                                          struct ResourceManager* const resourceManager,
                                          const char* const listBoxResId) {
    if (!listBox || !resourceManager || !listBoxResId)
        return LIST_BOX_ERR_NULL_ARGUMENT;
    enum ListBoxSceneNode_errors result = 0;
    struct TextParser* textParser = NULL;
    textParser = TextParser_constructEmpty();
    if (!textParser)
        return LIST_BOX_ERR_CONSTRUCTIG_TEXT_PARSER;
    result += (TextParser_addString(textParser, TextParser_standartTypeString, ListBoxSceneNode_parserStrings.type) != 0);
    result += (TextParser_addString(textParser, ListBoxSceneNode_parserStrings.textBoxRes,
                                   listBox->textBox->sceneNode.sceneNodeTextResource->id) != 0);
    result += (TextParser_addString(textParser, ListBoxSceneNode_parserStrings.buttonRes,
                                   listBox->button->sceneNode.sceneNodeTextResource->id) != 0);
    result += (TextParser_addString(textParser, ListBoxSceneNode_parserStrings.contextMenuRes,
                                   listBox->contextMenu->sceneNode.sceneNodeTextResource->id) != 0);
    char* tempString = TextParser_convertToText(textParser);
    result += (textParser->lastError != 0);
    result += (TextResource_updateContent(listBox->sceneNode.sceneNodeTextResource, tempString) != 0);
    result += (ResourceManager_saveTextResource(resourceManager,
                                               listBox->sceneNode.sceneNodeTextResource, listBoxResId) != 0);
    TextParser_destruct(textParser);
    if (tempString)
        free(tempString);
    if (result)
        return LIST_BOX_ERR_SAVING;
    return LIST_BOX_NO_ERRORS;
}

void ListBox_control(struct SceneNode* sceneNode, struct EventManager* eventManager) {
    if (!sceneNode || !eventManager || !eventManager)
        return;
    size_t i;
    struct ListBox* listBox = (struct ListBox*)sceneNode;
    TextBox_control((struct SceneNode*)listBox->textBox, eventManager);
    if (listBox->textBox->isStringChanged) {
        bool found = false;
        for (i = 0; i < listBox->contextMenu->menuOptionsCount; i++)
            if (strcmp(listBox->textBox->string, listBox->contextMenu->menuOptionsList[i]->label->text) == 0) {
                listBox->isStringExistInList = true;
                found = true;
                break;
            }
        if (!found) {
            listBox->isStringExistInList = false;
        }
    }
    if (listBox->textBox->haveFocus) {
        if (listBox->isStringExistInList) {
            listBox->textBox->box->state = BUTTON_STATE_PRESSED;
            listBox->textBox->box->isStateChanged = true;
        }
        else {
            listBox->textBox->box->state = BUTTON_STATE_FOCUSED;
            listBox->textBox->box->isStateChanged = true;
        }
    }
    if (listBox->isContextMenuShown) {
        ContextMenu_control((struct SceneNode*)listBox->contextMenu, eventManager);
        SDL_Point mouseCoordinates;
        SDL_GetMouseState(&mouseCoordinates.x, &mouseCoordinates.y);
        for (i = 0; i < listBox->contextMenu->menuOptionsCount; i++)
            if (listBox->contextMenu->menuOptionsList[i]->isStateChanged
                && listBox->contextMenu->menuOptionsList[i]->state == BUTTON_STATE_PRESSED) {
                char* newString = listBox->contextMenu->menuOptionsList[i]->label->text;
                if (TextBox_changeString(listBox->textBox, newString))
                    TextBox_changeString(listBox->textBox, "Err");
                    // FIXME Add checking ContextMenu's options labels length
                break;
            }
        for (i = 0; i < eventManager->sdlEventsCount; i++)
            // FIXME Rewrite this shit
            if (eventManager->sdlEventsList[i].type == SDL_MOUSEBUTTONDOWN && listBox->contextMenu->menuOptionsCount > 0) {
                if (!(mouseCoordinates.x >= listBox->contextMenu->menuOptionsList[0]->sprite->dstRect.x
                    && mouseCoordinates.x < (listBox->contextMenu->menuOptionsList[0]->sprite->dstRect.x 
                                                        + listBox->contextMenu->menuOptionsList[0]->sprite->dstRect.w) 
                    && mouseCoordinates.y >= listBox->contextMenu->menuOptionsList[0]->sprite->dstRect.y 
                    && mouseCoordinates.y < (listBox->contextMenu->menuOptionsList[0]->sprite->dstRect.x
                                                                + listBox->contextMenu->menuOptionsList[0]->sprite->dstRect.h 
                                                                            * (int)listBox->contextMenu->menuOptionsCount))) {
                    listBox->isContextMenuShown = false;
                }
            }
    }
    Button_control((struct SceneNode*)listBox->button, eventManager);
    if (listBox->button->isStateChanged && listBox->button->state == BUTTON_STATE_PRESSED) {
        if (listBox->isContextMenuShown)
            listBox->isContextMenuShown = false;
        else
            listBox->isContextMenuShown = true;
    }
}

void ListBox_update(struct SceneNode* sceneNode, struct EventManager* eventManager, struct Renderer* renderer) {
    if (!sceneNode || !eventManager || !renderer)
        return;
    struct ListBox* listBox = (struct ListBox*)sceneNode;
    if (listBox->isGeometryChanged) {
        listBox->textBox->sceneNode.coordinates = listBox->sceneNode.coordinates;
        listBox->textBox->isGeometryChanged = true;
        listBox->button->sceneNode.coordinates = listBox->sceneNode.coordinates;
        listBox->button->sceneNode.coordinates.x += listBox->textBox->box->sprite->virtualSize.x;
        listBox->button->isGeometryChanged = true;
        listBox->contextMenu->sceneNode.coordinates = listBox->sceneNode.coordinates;
        listBox->contextMenu->sceneNode.coordinates.y += listBox->textBox->box->sprite->virtualSize.y;
        listBox->contextMenu->isGeometryChanged = true;
        listBox->isGeometryChanged = false;
    }
    TextBox_update((struct SceneNode*)listBox->textBox, eventManager, renderer);
    Button_update((struct SceneNode*)listBox->button, eventManager, renderer);
    ContextMenu_update((struct SceneNode*)listBox->contextMenu, eventManager, renderer);
}

void ListBox_render(struct SceneNode* sceneNode, struct Renderer* renderer) {
    if (!sceneNode || !renderer)
        return;
    struct ListBox* listBox = (struct ListBox*)sceneNode;
    TextBox_render((struct SceneNode*)listBox->textBox, renderer);
    Button_render((struct SceneNode*)listBox->button, renderer);
    if (listBox->isContextMenuShown)
        ContextMenu_render((struct SceneNode*)listBox->contextMenu, renderer);
}

void ListBox_sound(struct SceneNode* sceneNode, struct Musican* musican) {
    if (!sceneNode || !musican)
        return;
    struct ListBox* listBox = (struct ListBox*)sceneNode;
    TextBox_sound((struct SceneNode*)listBox->textBox, musican);
    Button_sound((struct SceneNode*)listBox->button, musican);
    if (listBox->isContextMenuShown)
        ContextMenu_sound((struct SceneNode*)listBox->contextMenu, musican);
}
