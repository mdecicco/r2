#include <r2/bindings/bindings.h>
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>
#include <v8pp/factory.hpp>
#include <r2/utilities/imgui/imgui.h>
#include <functional>

using namespace v8;
using namespace v8pp;
using namespace std;

#define v8str(str) v8::String::NewFromUtf8(isolate, str, v8::String::kNormalString, strlen(str))

namespace r2 {
	Vector2::Vector2(f32 _x, f32 _y) : x(_x), y(_y) { }
	Vector2::~Vector2() { }
	ImVec2 Vector2::to_imgui() const { return ImVec2(x, y); }
	bool Vector2::operator==(const Vector2& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool Vector2::operator!=(const Vector2& rhs) const {
		return x != rhs.x || y != rhs.y;
	}
	Vector3::Vector3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) { }
	Vector3::~Vector3() { }
	bool Vector3::operator==(const Vector3& rhs) const {
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}
	bool Vector3::operator!=(const Vector3& rhs) const {
		return x != rhs.x || y != rhs.y || z != rhs.z;
	}
	Vector4::Vector4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) { }
	Vector4::~Vector4() { }
	ImVec4 Vector4::to_imgui() const { return ImVec4(x, y, z, w); }
	bool Vector4::operator==(const Vector4& rhs) const {
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	}
	bool Vector4::operator!=(const Vector4& rhs) const {
		return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
	}

	void register_structs(context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		//Vector2
		{
			class_<Vector2, v8pp::raw_ptr_traits> s(global->GetIsolate());
			s.ctor<f32, f32>();
			s.set("x", &Vector2::x);
			s.set("y", &Vector2::y);
			s.set("toImGui", &Vector2::to_imgui);
			ctx->set("Vector2", s);
		}

		//Vector3
		{
			class_<Vector3, v8pp::raw_ptr_traits> s(global->GetIsolate());
			s.ctor<f32, f32, f32>();
			s.set("x", &Vector3::x);
			s.set("y", &Vector3::y);
			s.set("z", &Vector3::z);
			ctx->set("Vector3", s);
		}

		//Vector4
		{
			class_<Vector4, v8pp::raw_ptr_traits> s(global->GetIsolate());
			s.ctor<f32, f32, f32, f32>();
			s.set("x", &Vector4::x);
			s.set("y", &Vector4::y);
			s.set("z", &Vector4::z);
			s.set("w", &Vector4::w);
			s.set("toImGui", &Vector4::to_imgui);
			ctx->set("Vector4", s);
		}
	}

	void bind_math(context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();
		register_structs(ctx);
	}
};