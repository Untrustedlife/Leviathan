// ------------------------------------ //
#ifndef LEVIATHAN_LEAPLISTENER
#include "LeapListener.h"
#endif
#include "Leap.h"
#include "Application/Application.h"
using namespace Leviathan;
using namespace Leap;
// ------------------------------------ //
#include "LeapManager.h"

Leviathan::LeapListener::LeapListener(LeapManager* owner) : Owner(owner), HandledFrames(0){
    
	// Set everything to default //
	Connected = false;
	Focused = false;
}

Leviathan::LeapListener::~LeapListener(){
}
// ------------------------------------ //
void Leviathan::LeapListener::onInit(const Leap::Controller &control){

	Logger::Get()->Info(L"LeapListener: initialized");
}

void Leviathan::LeapListener::onConnect(const Leap::Controller &control){

	control.enableGesture(Gesture::TYPE_CIRCLE);
	control.enableGesture(Gesture::TYPE_KEY_TAP);
	control.enableGesture(Gesture::TYPE_SCREEN_TAP);
	control.enableGesture(Gesture::TYPE_SWIPE);
    
	Connected = true;

    if(!control.isGestureEnabled(Gesture::TYPE_SWIPE)){

        assert(0 && "leap gesture fail");
    }

	Logger::Get()->Info(L"LeapListener: connected");
}

void Leviathan::LeapListener::onDisconnect(const Leap::Controller &control){
	// SDK Note: not dispatched when running in a debugger
	Connected = false;

    Logger::Get()->Info(L"LeapListener: disconnected");
}

void Leviathan::LeapListener::onExit(const Leap::Controller &control){
	Connected = false;
}

void Leviathan::LeapListener::onFrame(const Leap::Controller &control){
	// get most recent frame //
	const Frame frame = control.frame();

    HandleFrame(frame, control);
}

void Leviathan::LeapListener::onFocusGained(const Leap::Controller &control){

    Logger::Get()->Info("LeapListener: gained focus");
	Focused = true;
}

void Leviathan::LeapListener::onFocusLost(const Leap::Controller &control){

    Logger::Get()->Info("LeapListener: lost focus");
	Focused = false;
}
// ------------------------------------ //
void Leviathan::LeapListener::HandleFrame(const Leap::Frame &frame,
    const Leap::Controller &control)
{
    ++HandledFrames;

    // TODO: report these frames to someplace

    

	// process the frame gestures //
	const GestureList gestures = frame.gestures();
	for(int i = 0; i < gestures.count(); i++){
		// get current gesture //
		Gesture gesture = gestures[i];
		// switch based on type and process //
		switch(gesture.type()) {
            case Gesture::TYPE_CIRCLE:
			{
				// instantiate correct gesture subclass //
				CircleGesture circle = gesture;
				wstring datastr;
				// check direction //
				if(circle.pointable().direction().angleTo(circle.normal()) <= PI/4) {
					// clockwise rotation //
					datastr += L"clockwise ";
				} else {
					// counterclockwise //
					datastr += L"counterclockwise ";
				}

				// Calculate angle difference since last frame //
				float sweptAngle = 0;
				if(circle.state() != Gesture::STATE_START){
					// retrieve this gesture in last frame based on id //
					CircleGesture previousgesturestate = CircleGesture(control.frame(1).gesture(circle.id()));
					// get progress change and change it to a radian angle //
					sweptAngle = (float)((circle.progress() - previousgesturestate.progress())*2*PI);
				}

				datastr += L"Circle id: "+Convert::ToWstring(circle.id());
				datastr += L", state: " +Convert::ToWstring(circle.state());
				datastr += L", progress: " +Convert::ToWstring(circle.progress());
				datastr += L", radius: " +Convert::ToWstring(circle.radius());
				datastr += L", angle " +Convert::ToWstring(sweptAngle * RAD_TO_DEG);

			}
			break;
            case Gesture::TYPE_SWIPE:
			{
				// instantiate correct gesture subclass //
				const SwipeGesture swipe = gesture;


                // Check for shutdown left swipe //
                if(swipe.direction().x < -0.6){
                    if(swipe.durationSeconds() >= 0.04f){

                        auto change = swipe.startPosition()-swipe.position();

                        if(change.x >= 40){

                            Logger::Get()->Info(L"LeapManager: Input: swipe threshold passed, shutting down");

                            Leviathan::LeviathanApplication::GetApp()->MarkAsClosing();
                        }

                    }
                }
			}
			break;
            case Gesture::TYPE_KEY_TAP:
			{
				// instantiate correct gesture subclass //
				KeyTapGesture tap = gesture;
				wstring datastr;

				datastr += L"Key Tap id: "+Convert::ToWstring(tap.id());
				datastr += L", state: " +Convert::ToWstring(tap.state());
				datastr += L", position: " +Convert::StringToWstring(Convert::ToString(tap.position()));
				datastr += L", direction: " +Convert::StringToWstring(Convert::ToString(tap.direction()));

			}
			break;
            case Gesture::TYPE_SCREEN_TAP:
			{
				// instantiate correct gesture subclass //
				ScreenTapGesture screentap = gesture;
				wstring datastr;

				datastr += L"Key Tap id: "+Convert::ToWstring(screentap.id());
				datastr += L", state: " +Convert::ToWstring(screentap.state());
				datastr += L", position: " +Convert::StringToWstring(Convert::ToString(screentap.position()));
				datastr += L", direction: " +Convert::StringToWstring(Convert::ToString(screentap.direction()));
			}
			break;
            default:
                Logger::Get()->Error(L"LeapListener: unknown gesture type: "+Convert::ToWstring(gesture.type()));
                break;
		}
	}    
}


// ------------------------------------ //




