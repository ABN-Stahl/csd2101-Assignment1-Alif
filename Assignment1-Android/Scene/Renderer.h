#pragma once

/*!***************************************************************************
 * \file   Renderer.h
 * \author Alif Bin Norazhar(alif.binnorazhar@digipen.edu)
 * \par    CSD2101
 * \date   2026
 * \brief  Singleton controller that owns all renderable Models and drives
 *         the frame lifecycle (init, resize, render).  The only file that
 *         knows about concrete Model subclasses is Renderer.cpp.
 *****************************************************************************/

#include "Model.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer
{
public:
    static Renderer& Instance();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    std::vector<Model*> models;
};