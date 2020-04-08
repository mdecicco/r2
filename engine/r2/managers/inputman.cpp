#include <r2/managers/inputman.h>
#include <r2/engine.h>
#include <r2/utilities/utils.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace r2 {
	input_man::input_man() {
		HWND window = glfwGetWin32Window((GLFWwindow*)*r2engine::get()->window());
		m_im = OIS::InputManager::createInputSystem((size_t)window);
		m_keyboard = (OIS::Keyboard*)m_im->createInputObject(OIS::OISKeyboard, true);
		m_keyboard->setEventCallback(this);
		//m_mouse = (OIS::Mouse*)m_im->createInputObject(OIS::OISMouse, true);
		//m_mouse->setEventCallback(this);

		u8 joystick_count = m_im->getNumberOfDevices(OIS::OISJoyStick);
		for (u8 i = 0;i < joystick_count;i++) {
			OIS::JoyStick* js = (OIS::JoyStick*)m_im->createInputObject(OIS::OISJoyStick, true);
			js->setEventCallback(this);
			m_joysticks.push_back(js);
		}

		/*

		For some dumb ass reason this causes 5 memory leaks (according to the memory manager...)
		
		OIS::DeviceList l = m_im->listFreeDevices();
		for (auto& it = l.begin();it != l.end();it++) {
			if (it->first == OIS::OISJoyStick) {
				OIS::JoyStick* js = (OIS::JoyStick*)m_im->createInputObject(OIS::OISJoyStick, true, it->second);
				js->setEventCallback(this);
				m_joysticks.push_back(js);
			}
		}

		*/
	}
	input_man::~input_man() {
		m_keyboard->setEventCallback(nullptr);
		m_im->destroyInputObject(m_keyboard);
		//m_mouse->setEventCallback(nullptr);
		//m_im->destroyInputObject(m_mouse);
		for(auto js : m_joysticks) {
			js->setEventCallback(nullptr);
			m_im->destroyInputObject(js);
		}
		m_joysticks.clear();
		OIS::InputManager::destroyInputSystem(m_im);
		m_im = nullptr;
		m_keyboard = nullptr;
	}

	void input_man::poll() {
		m_keyboard->capture();
		//m_mouse->capture();
		for(auto js : m_joysticks) {
			js->capture();
		}
	}

	bool input_man::keyPressed(const OIS::KeyEvent& arg) {
		event e = evt(EVT_NAME_KEYBOARD_EVENT);
		e.set_json_from_str(format_string("{\"key\":%d,\"pressed\":true,\"released\":false}", arg.key));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::keyReleased(const OIS::KeyEvent& arg) {
		event e = evt(EVT_NAME_KEYBOARD_EVENT);
		e.set_json_from_str(format_string("{\"key\":%d,\"pressed\":false,\"released\":true}", arg.key));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::mouseMoved(const OIS::MouseEvent& arg) {
		event e = evt(EVT_NAME_MOUSE_EVENT);
		e.set_json_from_str(format_string(
			"{\"action\":\"move\",\"absolute\":{\"x\":%d,\"y\":%d,\"scroll\":%d},\"relative\":{\"x\":%d,\"y\":%d,\"scroll\":%d}}",
			arg.state.X.abs, arg.state.Y.abs, arg.state.Z.abs, arg.state.X.rel, arg.state.Y.rel, arg.state.Z.rel
		));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id) {
		event e = evt(EVT_NAME_MOUSE_EVENT);
		e.set_json_from_str(format_string("{\"action\":\"button\",\"buttonId\":%d,\"pressed\":true,\"released\":false}", id));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id) {
		event e = evt(EVT_NAME_MOUSE_EVENT);
		e.set_json_from_str(format_string("{\"action\":\"button\",\"buttonId\":%d,\"pressed\":false,\"released\":true}", id));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::buttonPressed(const OIS::JoyStickEvent& arg, int button) {
		event e = evt(EVT_NAME_JOYSTICK_EVENT);
		e.set_json_from_str(format_string("{\"action\":\"button\",\"joystickId\":%d,\"buttonId\":%d,\"pressed\":true,\"released\":false}", arg.device->getID(), button));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::buttonReleased(const OIS::JoyStickEvent& arg, int button) {
		event e = evt(EVT_NAME_JOYSTICK_EVENT);
		e.set_json_from_str(format_string("{\"action\":\"button\",\"joystickId\":%d,\"buttonId\":%d,\"pressed\":false,\"released\":true}", arg.device->getID(), button));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::axisMoved(const OIS::JoyStickEvent& arg, int axis) {
		event e = evt(EVT_NAME_JOYSTICK_EVENT);
		e.set_json_from_str(format_string(
			"{\"action\":\"axis\",\"joystickId\":%d,\"axisId\":%d,\"absolute\":%d,\"relative\":%d,\"has_relative\":%s}",
			arg.device->getID(), axis, arg.state.mAxes[axis].abs, arg.state.mAxes[axis].rel, arg.state.mAxes[axis].absOnly ? "false" : "true"
		));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::sliderMoved(const OIS::JoyStickEvent& arg, int index) {
		return true;
	}

	bool input_man::povMoved(const OIS::JoyStickEvent& arg, int index) {
		i8 dx = 0, dy = 0;
		if(arg.state.mPOV[index].direction != OIS::Pov::Centered) {
			if(arg.state.mPOV[index].direction & OIS::Pov::North) dy = 1;
			else if(arg.state.mPOV[index].direction & OIS::Pov::South) dy = -1;
			if(arg.state.mPOV[index].direction & OIS::Pov::East) dx = 1;
			else if(arg.state.mPOV[index].direction & OIS::Pov::West) dx = -1;
		}
		event e = evt(EVT_NAME_JOYSTICK_EVENT);
		e.set_json_from_str(format_string(
			"{\"action\":\"pov\",\"joystickId\":%d,\"povId\":%d,\"direction\":{\"x\":%d,\"y\":%d}}",
			arg.device->getID(), index, dx, dy
		));
		r2engine::get()->dispatch(&e);
		return true;
	}

	bool input_man::vector3Moved(const OIS::JoyStickEvent& arg, int index) {
		event e = evt(EVT_NAME_JOYSTICK_EVENT);
		e.set_json_from_str(format_string(
			"{\"action\":\"vec3\",\"joystickId\":%d,\"vec3Id\":%d,\"direction\":{\"x\":%f,\"y\":%f,\"z\":%f}}",
			arg.device->getID(), index, arg.state.mVectors[index].x, arg.state.mVectors[index].y, arg.state.mVectors[index].z
		));
		r2engine::get()->dispatch(&e);
		return true;
	}
};
