#include "audioapp.h"

#include <parameteroptionlist.h>

// Nap includes
#include <nap/core.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <renderwindow.h>
#include <renderservice.h>
#include <nap/logger.h>
#include <parametersimple.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioTestApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{

    void registerParameterEditors(ParameterGUIService& parameterGUIService)
    {
        parameterGUIService.registerParameterEditor(RTTI_OF(ParameterOptionList), [](Parameter& parameter)
        {
            ParameterOptionList* optionList = rtti_cast<ParameterOptionList>(&parameter);

            std::vector<rttr::string_view> items(optionList->getOptions().begin(), optionList->getOptions().end());

            int value = optionList->getValue();

            ImGui::PushID(&parameter);

            if (ImGui::Combo(optionList->getDisplayName().c_str(), &value,
                             [](void* data, int index, const char** out_text)
                             {
                                 std::vector<rttr::string_view>* items = (std::vector<rttr::string_view>*)data;
                                 *out_text = (*items)[index].data();
                                 return true;
                             },
                             &items, items.size()))
            {
                optionList->setValue(value);
            }

            ImGui::PopID();

        });

        parameterGUIService.registerParameterEditor(RTTI_OF(ParameterString), [](Parameter& parameter)
        {
            ParameterString* parameterString = rtti_cast<ParameterString>(&parameter);
            char buf[128];
            strcpy(buf, parameterString->mValue.c_str());
            ImGui::PushID(&parameter);
            if (ImGui::InputText(parameterString->getDisplayName().c_str(), buf, 128))
            {
                parameterString->mValue = buf;
            }
            ImGui::PopID();
        });
    }


	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool AudioTestApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
		mMidiService = getCore().getService<nap::MidiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile(appJson, error))
			return false;

		mParameterGroup = mResourceManager->findObject<ParameterGroup>("Parameters");
		if (mParameterGroup != nullptr)
		{
			registerParameterEditors(*getCore().getService<ParameterGUIService>());
		}

		mRenderWindow = std::make_unique<nap::RenderWindow>(getCore());
		mRenderWindow->mID = "Window0";
		mRenderWindow->mMode = RenderWindow::EPresentationMode::Immediate;
		if (!mRenderWindow->init(error))
		{
			error.fail("Fail to initialize window");
			return false;
		}

		mMidiInputPort = std::make_unique<MidiInputPort>();
		mMidiInputPort->mID = "MidiInputPort";
		if (!mMidiInputPort->init(error))
		{
			error.fail("Fail to initialize midi input port");
			return false;
		}

		mParameterGUI = std::make_unique<ParameterGUI>(getCore());
		mParameterGUI->mID = "ParameterGUI";
        mParameterGUI->mParameterGroup = mParameterGroup.get();
        if (!mParameterGUI->init(error))
        {
            error.fail("Fail to initialize GUI");
            return false;
        }

		// Find the audio entity
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		return true;
	}
	
	
	/**
	 */
	void AudioTestApp::update(double deltaTime)
	{
		if (mParameterGroup != nullptr)
		{
			if (mParameterGUI != nullptr)
				mParameterGUI->show(mParameterGroup.get());
		}
		ImGui::NewLine();
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
    }

	
	/**
	 */
	void AudioTestApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Render GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void AudioTestApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void AudioTestApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();
		}

		auto keyEvent = rtti_cast<KeyEvent>(inputEvent.get());
		if (keyEvent != nullptr)
		{
			auto midiEvent = mKeyToMidiConverter.processKeyEvent(*keyEvent);
			if (midiEvent != nullptr)
				mMidiService->enqueueEvent(std::move(midiEvent));
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	int AudioTestApp::shutdown()
	{
		mRenderWindow = nullptr;
		return 0;
	}
}
