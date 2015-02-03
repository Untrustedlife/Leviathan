#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GRAPHICALINPUTENTITY
#include "GraphicalInputEntity.h"
#endif
#include "OgreCommon.h"
#include "Rendering/Graphics.h"
#include "Entities/GameWorld.h"
#include "FileSystem.h"
#include "Engine.h"
#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "CEGUI/RendererModules/Ogre/Renderer.h"
#include "CEGUI/SchemeManager.h"
#include "GUI/FontManager.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Exceptions/ExceptionNULLPtr.h"
#include "boost/thread/lock_types.hpp"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GraphicalInputEntity::GraphicalInputEntity(Graphics* windowcreater, AppDef* windowproperties) :
    MouseCaptureState(false), CEGUIRenderer(NULL)
{

	// create window //

	const WindowDataDetails& WData = windowproperties->GetWindowDetails();

	// get vsync (this is rather expensive so it is stored) //
	bool vsync = windowproperties->GetVSync();

	// set some rendering specific parameters //
	Ogre::NameValuePairList WParams;

	// variables //
	int FSAA = 4;

	// get variables from engine configuration file //
	ObjectFileProcessor::LoadValueFromNamedVars<int>(windowproperties->GetValues(), L"FSAA", FSAA, 4, true,
        L"Graphics: Init:");

	Ogre::String fsaastr = Convert::ToString(FSAA);

	WParams["FSAA"] = fsaastr;
	WParams["vsync"] = vsync ? "true": "false";

	Ogre::String wcaption = Convert::WstringToString(WData.Title);
    
	// quicker access to the window //
	Ogre::RenderWindow* tmpwindow = windowcreater->GetOgreRoot()->createRenderWindow(wcaption, WData.Width,
        WData.Height, !WData.Windowed, &WParams);

    int windowsafter = 0;

    {
        boost::unique_lock<boost::mutex> lock(GlobalCountMutex);
        ++GlobalWindowCount;
        windowsafter = GlobalWindowCount;
    }

    // Do some first window initialization //
	if(windowsafter == 1){
        
		// Initialize the compositor //
        // This might not be required anymore...
		//windowcreater->GetOgreRoot()->initialiseCompositor();

		
		// Notify engine to register threads to work with Ogre //
		Engine::GetEngine()->_NotifyThreadsRegisterOgre();
		FileSystem::RegisterOGREResourceGroups();

		// Create the GUI system //
		CEGUI::OgreRenderer& guirenderer = CEGUI::OgreRenderer::bootstrapSystem(*tmpwindow);
		CEGUIRenderer = &guirenderer;

        FirstCEGUIRenderer = &guirenderer;

		// Print the used renderer //
		Logger::Get()->Info(L"GUI using CEGUI renderer: "+
            Convert::StringToWstring(guirenderer.getIdentifierString().c_str()));

		// Load the taharez look //
		CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");

		// Load the GUI fonts //
		windowcreater->GetFontManager()->LoadAllFonts();
        
	} else {

        // Wait for the first window to initialize //
        while(!FirstCEGUIRenderer){
            
            Logger::Get()->Info("GraphicalInputEntity: waiting for first window to initialize");
            
            boost::this_thread::sleep_for(MillisecondDuration(10));
        }
        
        // Create a new renderer //
        CEGUIRenderer = &CEGUI::OgreRenderer::registerWindow(*FirstCEGUIRenderer, *tmpwindow);
    }
	
	// Store this window's number
    {
        boost::unique_lock<boost::mutex> lock(TotalCountMutex);
        WindowNumber = ++TotalCreatedWindows;
    }

	// create the actual window //
	DisplayWindow = new Window(tmpwindow, this);
#ifdef _WIN32
	// apply style settings (mainly ICON) //
	WData.ApplyIconToHandle(DisplayWindow->GetHandle());
#else
	// \todo linux icon
#endif
	tmpwindow->setDeactivateOnFocusChange(false);


	// set the main window to be active //
	tmpwindow->setActive(true);


	// create GUI //
	WindowsGui = new Gui::GuiManager();
	if(!WindowsGui){
		throw ExceptionNULLPtr(L"cannot create GUI manager instance", 0, __WFUNCTION__, NULL);
	}

	if(!WindowsGui->Init(windowcreater, this, windowsafter == 1)){

		Logger::Get()->Error(L"GraphicalInputEntity: Gui init failed");
		throw ExceptionNULLPtr(L"invalid GUI manager", 0, __WFUNCTION__, WindowsGui);
	}


	// create receiver interface //
	TertiaryReceiver = shared_ptr<InputController>(new InputController());
}

DLLEXPORT Leviathan::GraphicalInputEntity::~GraphicalInputEntity(){
	GUARD_LOCK_THIS_OBJECT();

	// Mark the Window as unusable //
	DisplayWindow->InvalidateWindow();


	// GUI is very picky about delete order
	SAFE_RELEASEDEL(WindowsGui);

	// The window object has to be closed down to avoid errors and crashing //
	if(DisplayWindow)
		DisplayWindow->CloseDown();

	SAFE_DELETE(DisplayWindow);
	TertiaryReceiver.reset();

    int windowsafter = 0;

    {
        boost::unique_lock<boost::mutex> lock(GlobalCountMutex);
        --GlobalWindowCount;
        windowsafter = GlobalWindowCount;
    }

    // Destory CEGUI if we are the last window //
    if(windowsafter == 0){

        FirstCEGUIRenderer = NULL;
        CEGUI::OgreRenderer::destroySystem();

        Logger::Get()->Info("GraphicalInputEntity: all windows have been closed, should quit soon");
    }

    CEGUIRenderer = NULL;
}

GraphicalInputEntity* Leviathan::GraphicalInputEntity::InputCapturer = NULL;

int Leviathan::GraphicalInputEntity::GlobalWindowCount = 0;

boost::mutex Leviathan::GraphicalInputEntity::GlobalCountMutex;

int Leviathan::GraphicalInputEntity::TotalCreatedWindows = 0;

boost::mutex Leviathan::GraphicalInputEntity::TotalCountMutex;

CEGUI::OgreRenderer* Leviathan::GraphicalInputEntity::FirstCEGUIRenderer = NULL;
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::ReleaseLinked(){
	// release world and object references //
	LinkedWorld.reset();
	LinkedCamera.reset();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GraphicalInputEntity::Render(int mspassed){
	GUARD_LOCK_THIS_OBJECT();
	if(LinkedWorld)
		LinkedWorld->UpdateCameraLocation(mspassed, LinkedCamera.get());

	// update input before each frame //
	WindowsGui->Render();

	// update window //
	Ogre::RenderWindow* tmpwindow = DisplayWindow->GetOgreWindow();

	// cancel actual rendering if window closed //
	if(tmpwindow->isClosed()){

		Logger::Get()->Warning(L"GraphicalInputEntity: Render: skipping render due to window being closed");
		return false;
	}

	// finish rendering the window //
	tmpwindow->swapBuffers();

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::LinkObjects(shared_ptr<ViewerCameraPos> camera, shared_ptr<GameWorld> world){
	LinkedCamera = camera;
	LinkedWorld = world;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::SetCustomInputController(shared_ptr<InputController> controller){

	GUARD_LOCK_THIS_OBJECT();
	
	TertiaryReceiver = controller;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::GraphicalInputEntity::GetWindowNumber() const{

	return WindowNumber;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::SaveScreenShot(const string &filename){
	// uses render target's capability to save it's contents //
	DisplayWindow->GetOgreWindow()->writeContentsToTimestampedFile(filename, "_window1.png");
}

DLLEXPORT void Leviathan::GraphicalInputEntity::Tick(int mspassed){
	// pass to GUI //
	WindowsGui->GuiTick(mspassed);
}

DLLEXPORT void Leviathan::GraphicalInputEntity::OnResize(int width, int height){
	// send to GUI //
	WindowsGui->OnResize();
}

DLLEXPORT void Leviathan::GraphicalInputEntity::UnlinkAll(){
	LinkedWorld.reset();
	LinkedCamera.reset();
}

DLLEXPORT bool Leviathan::GraphicalInputEntity::SetMouseCapture(bool state){
	if(MouseCaptureState == state)
		return true;

	GUARD_LOCK_THIS_OBJECT();

	MouseCaptureState = state;

	// handle changing state //
	if(!MouseCaptureState){

		// set mouse visible and disable capturing //
		WindowsGui->SetMouseFileVisibleState(true);
		DisplayWindow->SetCaptureMouse(false);

		// reset pointer to indicate that this object no longer captures mouse to this window //
		InputCapturer = NULL;

	} else {

		if(InputCapturer != this && InputCapturer != NULL){
			// another window has input //
			MouseCaptureState = false;
			return false;
		}

		// hide mouse and tell window to capture //
		WindowsGui->SetMouseFileVisibleState(false);
		DisplayWindow->SetCaptureMouse(true);
		DisplayWindow->SetMouseToCenter();

		// set static ptr to this //
		InputCapturer = this;
	}
	return true;
}

DLLEXPORT void Leviathan::GraphicalInputEntity::OnFocusChange(bool focused){
	WindowsGui->OnFocusChanged(focused);
}

DLLEXPORT int Leviathan::GraphicalInputEntity::GetGlobalWindowCount(){
	return GlobalWindowCount;
}

