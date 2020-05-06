#pragma once
#include <include/OIS/OIS.h>
#include <r2/managers/memman.h>

namespace r2 {
	class input_man : public OIS::KeyListener, public OIS::JoyStickListener, public OIS::MouseListener {
		public:
			input_man();

			~input_man();

			void scan_devices();

			void poll();

			virtual bool keyPressed(const OIS::KeyEvent& arg);

			virtual bool keyReleased(const OIS::KeyEvent& arg);

			virtual bool mouseMoved(const OIS::MouseEvent& arg);

			virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

			virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

			virtual bool buttonPressed(const OIS::JoyStickEvent& arg, int button);

			virtual bool buttonReleased(const OIS::JoyStickEvent& arg, int button);

			virtual bool axisMoved(const OIS::JoyStickEvent& arg, int axis);

			virtual bool sliderMoved(const OIS::JoyStickEvent& arg, int index);

			virtual bool povMoved(const OIS::JoyStickEvent& arg, int index);

			virtual bool vector3Moved(const OIS::JoyStickEvent& arg, int index); 

			OIS::Keyboard* keyboard() const { return m_keyboard; }
			OIS::Mouse* mouse() const { return m_mouse; }
			u8 joystick_count() const { return (u8)m_joysticks.size(); }
			OIS::JoyStick* joystick(u8 idx) const { return m_joysticks[idx]; }

		protected:
			OIS::InputManager* m_im;
			OIS::Keyboard* m_keyboard;
			OIS::Mouse* m_mouse;
			mvector<OIS::JoyStick*> m_joysticks;
			timer m_scanTimer;
	};
};