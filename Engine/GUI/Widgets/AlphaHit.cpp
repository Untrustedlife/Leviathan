// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
// ------------------------------------ //
#include "AlphaHit.h"

#include "../AlphaHitCache.h"

#include "CEGUI/CoordConverter.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
AlphaHitButton::AlphaHitButton(const CEGUI::String& type, const CEGUI::String& name) :
    CEGUI::PushButton(type, name)
{

}

AlphaHitButton::~AlphaHitButton(){

}

const CEGUI::String AlphaHitButton::WidgetTypeName = "Leviathan/AlphaHitButton";
// ------------------------------------ //
bool AlphaHitButton::isHit(const glm::vec2 &position, const bool allow_disabled /*= false*/)
    const
{
    if(!CEGUI::PushButton::isHit(position, allow_disabled))
        return false;

    // Retrieve the texture used for this window, if we don't have it already //
    if(!HitTestTexture){

        const auto imageProperty = this->getProperty("Image");

        if(imageProperty.empty()){

            // No image...
            // Our rect already matched so return true
            return true;
        }
    
        HitTestTexture = AlphaHitCache::Get()->GetDataForImageProperty(imageProperty.c_str());

        if(!HitTestTexture){

            LOG_ERROR(("AlphaHit: couldn't find source texture to use for checks, "
                    "image: " + imageProperty).c_str());
        }
    }

    // Loading the image has failed //
    if(!HitTestTexture)
        return true;

    // Read the pixel under mouse pos //
    const auto relativePos = CEGUI::CoordConverter::screenToWindow(*this, position); 

    const auto pixel = HitTestTexture->GetPixel(relativePos.x, relativePos.y);
        
    // If it isn't completely transparent, hit //
    // Range is 0-255. 0 is fully transparent
    return pixel > 5;
}

