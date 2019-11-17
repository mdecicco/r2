#pragma once

#include <r2/config.h>
#include <r2/utilities/imgui/imgui.h>
#include <r2/utilities/imgui_impl_glfw_gl3.h>

#include <GLFW/glfw3.h>
#include <string>

using namespace std;
namespace r2 {
	class window {
		public:
			window();
			~window();
			bool create(i32 Resx, i32 Resy, string Title, bool Resizable = false, i32 Major = 3, i32 Minor = 2, bool Fullstreen = false);
			void make_current();
			bool get_close_requested();
			void swap_buffers();
			void poll();
			void destroy();

			operator GLFWwindow*() { return m_window; }
			f64 elapsed();
			vec2i get_size() const;
			void get_max_resolution(u32& width, u32& height) const;

		protected:
			GLFWwindow* m_window;
	};
};