#include <r2/bindings/bindings.h>
#include <r2/utilities/average.h>

#include <v8pp/context.hpp>
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>

#include <functional>

using namespace v8;
using namespace v8pp;
using namespace std;

namespace r2 {
	Vector2::Vector2(f32 _x, f32 _y) : x(_x), y(_y) { }
	Vector2::~Vector2() { }
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
			ctx->set("Vector4", s);
		}
		
		//average
		{
			class_<average, v8pp::raw_ptr_traits> s(global->GetIsolate());
			s.ctor<u16>();
			s.set("sample", &average::operator+=);
			s.set("reset", &average::reset);
			s.set("max_samples", v8pp::property(&average::get_max_samples));
			s.set("sample_count", v8pp::property(&average::get_sample_count));
			s.set("average", v8pp::property(&average::operator f32));
			ctx->set("RollingAverage", s);
		}
	}

	void bind_math(context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();
		register_structs(ctx);
	}
};