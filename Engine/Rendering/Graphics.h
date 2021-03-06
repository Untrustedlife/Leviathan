// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GraphicalInputEntity.h"
#include "Application/AppDefine.h"
#include "OgreFrameListener.h"

namespace Leviathan{
namespace Rendering{
        
class ShaderManager;
class FontManager;
}

class Graphics : Ogre::FrameListener{
    friend GraphicalInputEntity;
public:
    DLLEXPORT Graphics();
    DLLEXPORT ~Graphics();

    DLLEXPORT bool Init(AppDef* appdef);
    DLLEXPORT void Release();

    DLLEXPORT bool Frame();

    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    DLLEXPORT inline Rendering::FontManager* GetFontManager(){
        return Fonts.get();
    }
    DLLEXPORT inline AppDef* GetDefinitionObject(){
        return AppDefinition;
    }
    DLLEXPORT inline Ogre::Root* GetOgreRoot(){
        return ORoot.get();
    }

    DLLEXPORT static Graphics* Get();
private:

    bool InitializeOgre(AppDef* appdef);

    //! \brief Load all the new required hlms stuff
    //! \warning This must be called after an Ogre window has been created
    //!
    //! This is called by the first created GraphicalInputEntity
    void _LoadOgreHLMS();

private:
    bool Initialized = false;

    AppDef* AppDefinition = nullptr;

    // OGRE //
    std::unique_ptr<Ogre::Root> ORoot;
    Ogre::Log* OLog = nullptr;
    std::unique_ptr<Rendering::FontManager> Fonts;

    // static //
    static Graphics* Staticaccess;
};
}

