#include <r2/utilities/window.h>
#include <r2/engine.h>

namespace r2 {
	window::window() : m_window(0) { }

	window::~window() {
		destroy();
	}

	bool window::create(i32 Resx, i32 Resy, const mstring& Title, bool Resizable, i32 Major, i32 Minor, bool Fullscreen) {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Minor);
		glfwWindowHint(GLFW_OPENGL_PROFILE       , GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_SAMPLES              , 4);
		glfwWindowHint(GLFW_RESIZABLE            , Resizable);
		glfwWindowHint(GLFW_DEPTH_BITS  		 , 24);

		m_window = glfwCreateWindow(Resx, Resy, Title.c_str(), Fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
		if(!m_window) {
			glfwTerminate();
			printf("Unable to open glfw window.\n");
			return false;
		}

		make_current();
		glGetError();

		glViewport(0, 0, Resx, Resy);

		glGetError();

		glfwSwapInterval(0);

		ImGui_ImplGlfwGL3_Init(m_window, true);

		r2Log("OpenGL Driver Initialized");
		r2Log("Vendor: %s", glGetString(GL_VENDOR));
		r2Log("Renderer: %s", glGetString(GL_RENDERER));
		r2Log("Version: %s", glGetString(GL_VERSION));

		return true;
	}

	void window::destroy() {
		if(!m_window) return;

		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}

	bool window::get_close_requested() {
		if(!m_window) return true;

		return glfwWindowShouldClose(m_window);
	}

	void window::make_current() {
		if(m_window) glfwMakeContextCurrent(m_window);
	}

	void window::poll() {
		glfwPollEvents();
		//Update Renderer res if we are resizable
	}

	void window::swap_buffers() {
		if(m_window) glfwSwapBuffers(m_window);
	}

	f64 window::elapsed() {
		return glfwGetTime();
	}

	vec2i window::get_size() const {
		vec2i out;
		glfwGetFramebufferSize(m_window, &out.x, &out.y);
		return out;
	}

	void window::get_max_resolution(u32& width, u32& height) const {
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		width = mode->width;
		height = mode->height;
	}
};