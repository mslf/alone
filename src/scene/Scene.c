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
 * @file Scene.c
 * @author mslf
 * @date 11 Aug 2016
 * @brief File containing implementation os #Scene.
 */
#include "scene/Scene.h"
#include "textParser/TextParser.h"

static const struct Scene_errorMessages{
    const char* const errNoType;
    /**< Will be displayed when Scene_parserStrings#type string haven't found in #TextParser. */
    const char* const errNoDef;
    /**< Will be displayed when string with definition of #SceneNode from Scene_parserStrings#nodes 
     *haven't found in #TextParser. */
}Scene_errorMessages = {
    "Scene_constructSceneNode: suitable SceneNode type haven't detected or constructing SceneNode failed!",
    "Scene_init: definition of SceneNode haven't found!"};

/**
 * @brief Reallocates memory for Scene#sceneNodesList, increases Scene#sceneNodesCount 
 * by SCENE_SCENE_NODES_REALLOCATION_STEP.
 * @param scene Pointer to a #Scene, where function will reallocate Scene#sceneNodesList. Can be NULL.
 * @return #Scene_errors value.
 * @see #Scene_errors
 */
static enum Scene_errors Scene_reallocateSceneNodesList(struct Scene* scene) {
    if (!scene)
        return SCENE_ERR_NULL_ARGUMENT;
    struct SceneNode** sceneNodesList = NULL;
    size_t newSize = scene->allocatedSceneNodesCount + SCENE_SCENE_NODES_REALLOCATION_STEP;
    if (!(sceneNodesList = (struct SceneNode**)malloc(sizeof(struct SceneNode*) * newSize))) {
        free(sceneNodesList);
        return SCENE_ERR_ALLOC_NODES_LIST;
    }
    for (size_t i = 0; i < newSize; i++)
        sceneNodesList[i] = NULL;
    for (size_t i = 0; i < scene->sceneNodesCount; i++)
        sceneNodesList[i] = scene->sceneNodesList[i];
    free(scene->sceneNodesList);
    scene->sceneNodesList = sceneNodesList;
    scene->allocatedSceneNodesCount = newSize;
    return SCENE_NO_ERRORS;
}

/**
 * @brief Reallocates memory for Scene#eventControllersList, increases Scene#eventControllersCount
 * by SCENE_EVENT_CONTROLLERS_REALLOOCATION_STEP.
 * @param scene Pointer to a #Scene, where function will reallocate Scene#eventControllersList. Can be NULL.
 * @return #Scene_errors value.
 * @see #Scene_errors
 */
static enum Scene_errors Scene_reallocateEventControllersList(struct Scene* scene) {
    if (!scene)
        return SCENE_ERR_NULL_ARGUMENT;
    struct ScriptResource** eventControllersList = NULL;
    size_t newSize = scene->allocatedEventControllersCount + SCENE_EVENT_CONTROLLERS_REALLOOCATION_STEP;
    if (!(eventControllersList = (struct ScriptResource**)malloc(sizeof(struct ScriptResource*) * newSize))) {
        free(eventControllersList);
        return SCENE_ERR_ALLOC_CONTROLLERS_LIST;
    }
    for (size_t i = 0; i < newSize; i++)
        eventControllersList[i] = NULL;
    for (size_t i = 0; i < scene->eventControllersCount; i++)
        eventControllersList[i] = scene->eventControllersList[i];
    free(scene->eventControllersList);
    scene->eventControllersList = eventControllersList;
    scene->allocatedEventControllersCount = newSize;
    return SCENE_NO_ERRORS;
}

/**
 * @brief Inits #SceneNode from #TextParser.
 * @param scene Pointer to a #Scene where last added #SceneNode will be initialized. Can be NULL.
 * @param sceneNode String with name of #SceneNode to find it in #TextParser. Can be NULL.
 * @param sceneTextParser Pointer to a #TextParser with data of #SceneNode. Can be NULL.
 * @return #Scene_errors value.
 * @see #Scene_errors
 * @see #TextParser
 * @see #SceneNode
 */
static enum Scene_errors Scene_initSceneNode(struct Scene* scene,
                                             const char* const sceneNode,
                                             struct TextParser* sceneTextParser) {
    if (!scene || !sceneTextParser || !sceneNode)
        return SCENE_ERR_NULL_ARGUMENT;
    TextParser_getItemsCount(sceneTextParser, sceneNode);
    if (sceneTextParser->lastError)
        return SCENE_ERR_NO_NODE_DEF;
    size_t index = scene->sceneNodesCount - 1;
    // Don't care about errors here, because these setters are optional
    //? You could make a code easier to read if you use C99 struct initializers
    /*struct SceneNode tempSceneNode = {
        .coordinates.x = (int)TextParser_getInt(sceneTextParser, sceneNode, 1),
        .coordinates.y = (int)TextParser_getInt(sceneTextParser, sceneNode, 2),
        .rotatePointCoordinates.x = (int)TextParser_getInt(sceneTextParser, sceneNode, 3),
        .rotatePointCoordinates.y = (int)TextParser_getInt(sceneTextParser, sceneNode, 4),
        .flip = (SDL_RendererFlip)TextParser_getInt(sceneTextParser, sceneNode, 5),
        .angle = TextParser_getDouble(sceneTextParser, sceneNode, 6),
        .scaleX = TextParser_getDouble(sceneTextParser, sceneNode, 7),
        .scaleY = TextParser_getDouble(sceneTextParser, sceneNode, 8)
    };
    SceneNode_partiallyInitFrom(scene->sceneNodesList[index], &tempSceneNode);*/
    // It doesn't becomes easier to read. Or I can't.
    scene->sceneNodesList[index]->coordinates.x = (int)TextParser_getInt(sceneTextParser, sceneNode, 1);
    scene->sceneNodesList[index]->coordinates.y = (int)TextParser_getInt(sceneTextParser, sceneNode, 2);
    scene->sceneNodesList[index]->rotatePointCoordinates.x = (int)TextParser_getInt(sceneTextParser, sceneNode, 3);
    scene->sceneNodesList[index]->rotatePointCoordinates.y = (int)TextParser_getInt(sceneTextParser, sceneNode, 4);
    scene->sceneNodesList[index]->flip = (SDL_RendererFlip)TextParser_getInt(sceneTextParser, sceneNode, 5);
    scene->sceneNodesList[index]->angle = TextParser_getDouble(sceneTextParser, sceneNode, 6);
    scene->sceneNodesList[index]->scaleX = TextParser_getDouble(sceneTextParser, sceneNode, 7);
    scene->sceneNodesList[index]->scaleY = TextParser_getDouble(sceneTextParser, sceneNode, 8);
    return SCENE_NO_ERRORS;
}

/**
 * @brief Destructs #SceneNode using SceneNode#destruct.
 * @param sceneNode Pointer to a #SceneNode to be destructed. Can be NULL.
 */
void Scene_destructSceneNode(struct SceneNode* sceneNode) {
    if (!sceneNode)
        return;
    if (!sceneNode->destruct)
        return;
    sceneNode->destruct(sceneNode);
}

/**
 * @brief Inits (adds #SceneNode and #ScriptResource) #Scene from #TextParser.
 * @param scene Pointer to a #Scene to be initialized. Can be NULL.
 * @param resourceManager Pointer to a #ResourceManager where #Scene will get all needed resources. Can be NULL.
 * @param renderer Pointer to a #Renderer for constructing #SceneNode. Can be NULL.
 * @param sceneNodeTypesRegistrar Pointer to a #SceneNodeTypesRegistrar to construct #SceneNode. Can be NULL.
 * @param sceneTextParser Pointer to a #TextParser where #Scene will get all settings. Can be NULL.
 * @return #Scene_errors value.
 * @see #Scene_errors
 * @see #TextParser
 * @see #SceneNode
 * @see #ResourceManager
 * @see #Renderer
 * @see #SceneNodeTypesRegistrar
 */
static enum Scene_errors Scene_init(struct Scene* scene, struct ResourceManager* resourceManager,
                         struct Renderer* renderer, struct SceneNodeTypesRegistrar* sceneNodeTypesRegistrar, 
                         struct TextParser* sceneTextParser) {
    if (!scene || !resourceManager || !renderer || !sceneTextParser)
        return SCENE_ERR_NULL_ARGUMENT;
    size_t count;
    count = TextParser_getItemsCount(sceneTextParser, Scene_parserStrings.nodes);
    if (sceneTextParser->lastError)
        return SCENE_ERR_NO_NODES;
    scene->sceneNodesList = (struct SceneNode**)malloc(sizeof(struct SceneNode*) * count);
    if(!scene->sceneNodesList)
        return SCENE_ERR_ALLOC_NODES_LIST;
    scene->allocatedSceneNodesCount = count;
    scene->sceneNodesCount = 0;
    count = TextParser_getItemsCount(sceneTextParser, Scene_parserStrings.controllers);
    if (sceneTextParser->lastError)
        return SCENE_ERR_NO_CONTROLLERS;
    scene->eventControllersList = (struct ScriptResource**)malloc(sizeof(struct ScriptResource*) * count);
    if (!scene->eventControllersList)
        return SCENE_ERR_ALLOC_CONTROLLERS_LIST;
    scene->allocatedEventControllersCount = count;
    scene->eventControllersCount = 0;
    const char* tempString = NULL;
    for (size_t i = 0; i < scene->allocatedSceneNodesCount; i++) {
        tempString = TextParser_getString(sceneTextParser, Scene_parserStrings.nodes, i);
        if (tempString) {
            const char* tempSceneNodeRes = NULL;
            tempSceneNodeRes = TextParser_getString(sceneTextParser, tempString, 0);
            if (!tempSceneNodeRes) {
                Logger_log(resourceManager->logger, "%s Name: <%s>", Scene_errorMessages.errNoDef, tempString);
            }
            if (Scene_addSceneNode(scene, resourceManager, renderer, sceneNodeTypesRegistrar, tempSceneNodeRes) == 0)
                Scene_initSceneNode(scene, tempString, sceneTextParser);
        }
    }
    for (size_t i = 0; i < scene->allocatedEventControllersCount; i++) {
        tempString = TextParser_getString(sceneTextParser, Scene_parserStrings.controllers, i);
        if (tempString)
            Scene_addEventControllerScript(scene, resourceManager, tempString);
    }
    return SCENE_NO_ERRORS;
}

struct Scene* Scene_construct(struct ResourceManager* const resourceManager,
                              struct Renderer* renderer,
                              struct SceneNodeTypesRegistrar* sceneNodeTypesRegistrar, 
                              const char* const sceneResId) {
    struct Scene* scene = NULL;
    if (!resourceManager || !renderer || !sceneResId)
        return NULL;
    scene = (struct Scene*)malloc(sizeof(struct Scene));
    if (!scene)
        return NULL;
    scene->sceneNodesList = NULL;
    scene->eventControllersList = NULL;
    scene->sceneResource = NULL;
    scene->sceneResource = ResourceManager_loadTextResource(resourceManager, sceneResId, 1);
    if (!scene->sceneResource) {
        Scene_destruct(scene);
        return NULL;
    }
    struct TextParser* sceneTextParser = NULL;
    sceneTextParser = TextParser_constructFromTextResource(resourceManager->logger, scene->sceneResource);
    if (!sceneTextParser) {
        Scene_destruct(scene);
        return NULL;
    }
    const char* sceneTypeString = NULL;
    sceneTypeString = TextParser_getString(sceneTextParser, TEXT_PARSER_TYPE_STRING, 0);
    if (sceneTextParser->lastError) {
        Scene_destruct(scene);
        TextParser_destruct(sceneTextParser);
        return NULL;
    }
    if (strcmp(sceneTypeString, Scene_parserStrings.type) != 0) {
        Scene_destruct(scene);
        TextParser_destruct(sceneTextParser);
        return NULL;
    }
    if (Scene_init(scene, resourceManager, renderer, sceneNodeTypesRegistrar, sceneTextParser)) {
        Scene_destruct(scene);
        TextParser_destruct(sceneTextParser);
        return NULL;
    }
    TextParser_destruct(sceneTextParser);
    return scene;
}

void Scene_destruct (struct Scene* scene) {
    if (scene) {
        if (scene->eventControllersList) {
            for (size_t i = 0; i < scene->eventControllersCount; i++)
                ScriptResource_decreasePointersCounter(scene->eventControllersList[i]);
            free(scene->eventControllersList);
        }
        if (scene->sceneNodesList) {
            for (size_t i = 0; i < scene->sceneNodesCount; i++)
                Scene_destructSceneNode(scene->sceneNodesList[i]);
            free(scene->sceneNodesList);
        }
        TextResource_decreasePointersCounter(scene->sceneResource);
        free(scene);
    }
}

enum Scene_errors Scene_addSceneNode(struct Scene* scene, struct ResourceManager* const resourceManager,
                                 struct Renderer* renderer,
                                 struct SceneNodeTypesRegistrar* sceneNodeTypesRegistrar,
                                 const char* const sceneNodeResId) {
    if (!scene || !resourceManager || !sceneNodeResId)
        return SCENE_ERR_NULL_ARGUMENT;
    // Construct, then try to reallocate (if needed) and add sceneNode to the list
    struct SceneNode* sceneNode = NULL;
    // We want to construct every registered SceneNode, so requiredType is NULL
    sceneNode = SceneNodeTypesRegistrar_constructSceneNode(resourceManager, renderer,
                                                           sceneNodeTypesRegistrar,
                                                           sceneNodeResId,
                                                           NULL);
    if(!sceneNode) {
        return SCENE_ERR_CONSTRUCTING_NODE;
    }
    if (scene->sceneNodesCount >= scene->allocatedSceneNodesCount)
        if (Scene_reallocateSceneNodesList(scene)) {
            Scene_destructSceneNode(sceneNode);
            return SCENE_ERR_ALLOC_NODES_LIST;
        }
    scene->sceneNodesList[scene->sceneNodesCount] = sceneNode;
    scene->sceneNodesCount++;
    return SCENE_NO_ERRORS;
}

void Scene_removeSceneNode(
        struct Scene* const scene, size_t index) {
    if (scene && index < scene->sceneNodesCount) {
        Scene_destructSceneNode(scene->sceneNodesList[index]);
        scene->sceneNodesCount--;
        for (size_t i = index; i < scene->sceneNodesCount; i++)
            scene->sceneNodesList[i] = scene->sceneNodesList[i+1];
    }
}

enum Scene_errors Scene_save(struct Scene* const scene,
                             struct ResourceManager* const resourceManager,
                             const char* const sceneResId) {
    if (!scene || !resourceManager || !sceneResId)
        return SCENE_ERR_NULL_ARGUMENT;
    struct TextParser* textParser = NULL;
    unsigned char result = 0;
    textParser = TextParser_constructEmpty();
    if (!textParser)
        return SCENE_ERR_CONSTRUCTING_TEXT_PARSER;
    result += TextParser_addString(textParser, TEXT_PARSER_TYPE_STRING, Scene_parserStrings.type);
    for (size_t i = 0; i < scene->sceneNodesCount; i++) {
        char tempSceeNodeName[600];
        sprintf(tempSceeNodeName, "%ld", i);
        result += TextParser_addString(textParser, Scene_parserStrings.nodes, tempSceeNodeName);
        result += TextParser_addString(textParser, tempSceeNodeName, scene->sceneNodesList[i]->sceneNodeTextResource->id);
        result += TextParser_addInt(textParser, tempSceeNodeName, scene->sceneNodesList[i]->coordinates.x);
        result += TextParser_addInt(textParser, tempSceeNodeName, scene->sceneNodesList[i]->coordinates.y);
        result += TextParser_addInt(textParser, tempSceeNodeName, scene->sceneNodesList[i]->rotatePointCoordinates.x);
        result += TextParser_addInt(textParser, tempSceeNodeName, scene->sceneNodesList[i]->rotatePointCoordinates.y);
        result += TextParser_addInt(textParser, tempSceeNodeName, (long)scene->sceneNodesList[i]->flip);
        result += TextParser_addDouble(textParser, tempSceeNodeName, scene->sceneNodesList[i]->angle);
        result += TextParser_addDouble(textParser, tempSceeNodeName, scene->sceneNodesList[i]->scaleX);
        result += TextParser_addDouble(textParser, tempSceeNodeName, scene->sceneNodesList[i]->scaleY);
    }
    for (size_t i = 0; i < scene->eventControllersCount; i++)
        result += TextParser_addString(textParser, Scene_parserStrings.controllers, scene->eventControllersList[i]->id);
    char* newText = NULL;
    newText = TextParser_convertToText(textParser);
    result += textParser->lastError;
    result += TextResource_updateContent(scene->sceneResource, newText);
    result += ResourceManager_saveTextResource(resourceManager, scene->sceneResource, sceneResId);
    TextParser_destruct(textParser);
    if(newText)
        free(newText);
    if (result)
        return SCENE_ERR_SAVING;
    return SCENE_NO_ERRORS;
}

enum Scene_errors Scene_addEventControllerScript(struct Scene* scene,
                                                 struct ResourceManager* resourceManager,
                                                 const char* const scriptResId) {
    if (!scene || !resourceManager || !scriptResId)
        return SCENE_ERR_NULL_ARGUMENT;
    struct ScriptResource* scriptResource = NULL;
    scriptResource = ResourceManager_loadScriptResource(resourceManager, scriptResId);
    if (!scriptResource)
        return SCENE_ERR_CONSTRUCTING_CONTROLLER;
    // Try to reallocate (if needed) and add scriptResource to the list
    if (scene->eventControllersCount >= scene->allocatedEventControllersCount)
        if (Scene_reallocateEventControllersList(scene))
            return SCENE_ERR_ALLOC_CONTROLLERS_LIST;
    scene->eventControllersList[scene->eventControllersCount] = scriptResource;
    scene->eventControllersCount++;
    return SCENE_NO_ERRORS;
}

void Scene_removeEventControllerScript(struct Scene* scene,
                                       const char* const scriptResId) {
    unsigned char found = 0;
    size_t foundIndex = 0;
    if (scene && scriptResId) {
        if (scene->eventControllersCount == 0)
            return; // There is no EventControllers
        for (size_t i = 0; i < scene->eventControllersCount; i++)
            if (strcmp(scene->eventControllersList[i]->id, scriptResId) == 0) {
                found = 1;
                foundIndex = i;
                break;
            }
        if (!found)
            return;
        ScriptResource_decreasePointersCounter(scene->eventControllersList[foundIndex]);
        scene->eventControllersCount--;
        for (size_t i = foundIndex; i < scene->eventControllersCount; i++)
            scene->eventControllersList[i] = scene->eventControllersList[i + 1];
    }
}

void Scene_update(struct Scene* scene) {
    if (!scene)
        return;
    for (size_t i = 0; i < scene->eventControllersCount; i++) {
        // We have sent pointers to ResourceManager, EventManager, GameManager to the script earlier
        // So we just exicute global code in script
        lua_getglobal(scene->eventControllersList[i]->luaState, "Global");
        lua_pcall(scene->eventControllersList[i]->luaState, 0, 0, 0);
    }
}
