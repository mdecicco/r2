#include <r2/engine.h>

#include <r2/bindings/math_defs.h>
#include <r2/utilities/average.h>

#include <v8pp/context.hpp>
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>

#include <r2/bindings/math_converters.h>

#include <functional>

using namespace v8;
using namespace v8pp;
using namespace std;

namespace r2 {
/*
	typedef glm::mat<4, 4, i32, glm::packed_highp> glm_mat4i;
	typedef glm::mat<4, 4, u32, glm::packed_highp> glm_mat4ui;
	typedef glm::mat<4, 4, f32, glm::packed_highp> glm_mat4f;

	#define mat_transforms(t, mat_t, postfix) \
	mat_custom<4, t, vec4_custom<t>> translation##postfix(const vec3_custom<t>& v) { return glm::translate(mat_t(t(1)), v); } \
	mat_custom<4, t, vec4_custom<t>> scale##postfix(const vec3_custom<t>& v) { return glm::scale(mat_t(t(1)), v); }

	mat_transforms(i32, glm_mat4i, i);
	mat_transforms(u32, glm_mat4ui, ui);
	mat_transforms(f32, glm_mat4f, f);
	mat_custom<4, f32, vec4_custom<f32>> rotation(const vec3_custom<f32>& axis, f32 angle) { return glm::rotate(glm_mat4f(1.0f), glm::radians(angle), axis); } \

	#define mat_defs_t(l, v, t) \
	mat_custom<l, t, v<t>> mat_custom<l, t, v<t>>::inverse() { return *this; } \
	mat_custom<l, t, v<t>> mat_custom<l, t, v<t>>::invert() { return *this; }

	#define mat_defs(l, v) \
	mat_defs_t(l, v, i32); \
	mat_defs_t(l, v, u32); \
	mat_custom<l, f32, v<f32>> mat_custom<l, f32, v<f32>>::inverse() { return glm::inverse(*this); } \
	mat_custom<l, f32, v<f32>> mat_custom<l, f32, v<f32>>::invert() { return *this = glm::inverse(*this); } 

	mat_defs(2, vec2_custom);
	mat_defs(3, vec3_custom);
	mat_defs(4, vec4_custom);

	#define custom_vec(name, count) \
	name<i32> name<i32>::mulMatrix  (const mat_custom<count, i32, name<i32>>& rhs) { return *this * rhs; } \
	name<i32> name<i32>::mulEqMatrix(const mat_custom<count, i32, name<i32>>& rhs) { return *this = *this * rhs; } \
	i32	      name<i32>::dot        (const name<i32>& rhs) { return 0; } \
	void      name<i32>::normalize  () { } \
	name<i32> name<i32>::normalized () { return *this; } \
	i32       name<i32>::magnitude  () { return 0; } \
	i32       name<i32>::distance   (const name<i32>& rhs) { return 0; } \
	name<u32> name<u32>::mulMatrix  (const mat_custom<count, u32, name<u32>>& rhs) { return *this * rhs; } \
	name<u32> name<u32>::mulEqMatrix(const mat_custom<count, u32, name<u32>>& rhs) { return *this = *this * rhs; } \
	u32	      name<u32>::dot        (const name<u32>& rhs) { return 0; } \
	void      name<u32>::normalize  () { } \
	name<u32> name<u32>::normalized () { return *this; } \
	u32       name<u32>::magnitude  () { return 0; } \
	u32       name<u32>::distance   (const name<u32>& rhs) { return 0; } \
	name<f32> name<f32>::mulMatrix  (const mat_custom<count, f32, name<f32>>& rhs) { return *this * rhs; } \
	name<f32> name<f32>::mulEqMatrix(const mat_custom<count, f32, name<f32>>& rhs) { return *this = *this * rhs; } \
	f32	      name<f32>::dot        (const name<f32>& rhs) { return glm::dot<count, f32, glm::packed_highp>(*this, rhs); } \
	void      name<f32>::normalize  () { *this = glm::normalize(*this); } \
	name<f32> name<f32>::normalized () { return glm::normalize(*this); } \
	f32       name<f32>::magnitude  () { return glm::length<count, f32, glm::packed_highp>(*this); } \
	f32       name<f32>::distance   (const name<f32>& rhs) { return glm::length<count, f32, glm::packed_highp>(rhs - *this); }

	custom_vec(vec2_custom, 2);

	custom_vec(vec3_custom, 3);
	vec3_custom<i32> vec3_custom<i32>::cross(const vec3_custom<i32>& rhs) { return *this; }
	vec3_custom<u32> vec3_custom<u32>::cross(const vec3_custom<u32>& rhs) { return *this; }
	vec3_custom<f32> vec3_custom<f32>::cross(const vec3_custom<f32>& rhs) { return glm::cross(*this, rhs); }

	custom_vec(vec4_custom, 4);

	#define custom_vec_module_common() \
		s.set("add", &vec::add); \
		s.set("sub", &vec::sub); \
		s.set("mul", &vec::mul); \
		s.set("div", &vec::div); \
		s.set("addEq", &vec::addEq); \
		s.set("subEq", &vec::subEq); \
		s.set("mulEq", &vec::mulEq); \
		s.set("divEq", &vec::divEq); \
		s.set("addScalar", &vec::addScalar); \
		s.set("subScalar", &vec::subScalar); \
		s.set("mulScalar", &vec::mulScalar); \
		s.set("divScalar", &vec::divScalar); \
		s.set("addEqScalar", &vec::addEqScalar); \
		s.set("subEqScalar", &vec::subEqScalar); \
		s.set("mulEqScalar", &vec::mulEqScalar); \
		s.set("divEqScalar", &vec::divEqScalar); \
		s.set("mulMatrix", &vec::mulMatrix); \
		s.set("mulEqMatrix", &vec::mulEqMatrix); \
		if (postfix == "f") { \
			s.set("dot", &vec::dot); \
			s.set("normalize", &vec::normalize); \
			s.set("normalized", &vec::normalized); \
			s.set("magnitude", &vec::magnitude); \
			s.set("distance", &vec::distance); \
		}


	template<typename T>
	void register_vec2(const mstring& postfix, context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		using vec = vec2_custom<T>;
		class_<vec, v8pp::raw_ptr_traits> s(global->GetIsolate());
		s.auto_wrap_objects(true);
		s.ctor<T, T>();
		s.set("x", &vec::x);
		s.set("y", &vec::y);

		custom_vec_module_common();

		ctx->set(("vec2" + postfix).c_str(), s);
	}

	template<typename T>
	void register_vec3(const mstring& postfix, context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		using vec = vec3_custom<T>;
		class_<vec, v8pp::raw_ptr_traits> s(global->GetIsolate());
		register_class_state(s);
		s.auto_wrap_objects(true);
		s.ctor<T, T, T>();
		s.set("x", &vec::x); 
		s.set("y", &vec::y);
		s.set("z", &vec::z);
		
		custom_vec_module_common();

		if(postfix == "f") s.set("cross", &vec::cross);

		ctx->set(("vec3" + postfix).c_str(), s);
	}

	template<typename T>
	void register_vec4(const mstring& postfix, context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		using vec = vec4_custom<T>;
		class_<vec, v8pp::raw_ptr_traits> s(global->GetIsolate());
		register_class_state(s);
		s.auto_wrap_objects(true);
		s.ctor<T, T, T, T>();
		s.set("x", &vec::x);
		s.set("y", &vec::y);
		s.set("z", &vec::z);
		s.set("w", &vec::w);

		custom_vec_module_common();

		ctx->set(("vec4" + postfix).c_str(), s);
	}

	#define custom_mat_module_common() \
		s.set("mul", &mat::mul); \
		s.set("mulEq", &mat::mulEq); \
		s.set("mulVector", &mat::mulVector); \
		s.set("setIdentity", &mat::setIdentity); \
		s.set("transpose", &mat::transpose); \
		s.set("transposed", &mat::transposed); \
		if (postfix == "f") { \
			s.set("inverse", &mat::inverse); \
			s.set("invert", &mat::invert); \
		}

	template<typename T>
	void register_mat2(const mstring& postfix, context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		using mat = mat_custom<2, T, vec2_custom<T>>;
		class_<mat, v8pp::raw_ptr_traits> s(global->GetIsolate());
		register_class_state(s);
		s.auto_wrap_objects(true);
		s.ctor();
		s.set("xx", v8pp::property(&mat::get_mat_value<0, 0>, &mat::set_mat_value<0, 0>));
		s.set("xy", v8pp::property(&mat::get_mat_value<0, 1>, &mat::set_mat_value<0, 1>));
		s.set("yx", v8pp::property(&mat::get_mat_value<1, 0>, &mat::set_mat_value<1, 0>));
		s.set("yy", v8pp::property(&mat::get_mat_value<1, 1>, &mat::set_mat_value<1, 1>));

		custom_mat_module_common();

		ctx->set(("mat2" + postfix).c_str(), s);
	}

	template<typename T>
	void register_mat3(const mstring& postfix, context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		using mat = mat_custom<3, T, vec3_custom<T>>;
		class_<mat, v8pp::raw_ptr_traits> s(global->GetIsolate());
		register_class_state(s);
		s.auto_wrap_objects(true);
		s.ctor();
		s.set("xx", v8pp::property(&mat::get_mat_value<0, 0>, &mat::set_mat_value<0, 0>));
		s.set("xy", v8pp::property(&mat::get_mat_value<0, 1>, &mat::set_mat_value<0, 1>));
		s.set("xz", v8pp::property(&mat::get_mat_value<0, 2>, &mat::set_mat_value<0, 2>));
		s.set("yx", v8pp::property(&mat::get_mat_value<1, 0>, &mat::set_mat_value<1, 0>));
		s.set("yy", v8pp::property(&mat::get_mat_value<1, 1>, &mat::set_mat_value<1, 1>));
		s.set("yz", v8pp::property(&mat::get_mat_value<1, 2>, &mat::set_mat_value<1, 2>));
		s.set("zx", v8pp::property(&mat::get_mat_value<2, 0>, &mat::set_mat_value<2, 0>));
		s.set("zy", v8pp::property(&mat::get_mat_value<2, 1>, &mat::set_mat_value<2, 1>));
		s.set("zz", v8pp::property(&mat::get_mat_value<2, 2>, &mat::set_mat_value<2, 2>));

		custom_mat_module_common();

		ctx->set(("mat3" + postfix).c_str(), s);
	}

	template<typename T>
	void register_mat4(const mstring& postfix, context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		using mat = mat_custom<4, T, vec4_custom<T>>;
		class_<mat, v8pp::raw_ptr_traits> s(global->GetIsolate());
		register_class_state(s);
		s.auto_wrap_objects(true);
		s.ctor();
		s.set("xx", v8pp::property(&mat::get_mat_value<0, 0>, &mat::set_mat_value<0, 0>));
		s.set("xy", v8pp::property(&mat::get_mat_value<0, 1>, &mat::set_mat_value<0, 1>));
		s.set("xz", v8pp::property(&mat::get_mat_value<0, 2>, &mat::set_mat_value<0, 2>));
		s.set("xw", v8pp::property(&mat::get_mat_value<0, 2>, &mat::set_mat_value<0, 3>));
		s.set("yx", v8pp::property(&mat::get_mat_value<1, 0>, &mat::set_mat_value<1, 0>));
		s.set("yy", v8pp::property(&mat::get_mat_value<1, 1>, &mat::set_mat_value<1, 1>));
		s.set("yz", v8pp::property(&mat::get_mat_value<1, 2>, &mat::set_mat_value<1, 2>));
		s.set("yw", v8pp::property(&mat::get_mat_value<1, 3>, &mat::set_mat_value<1, 3>));
		s.set("zx", v8pp::property(&mat::get_mat_value<2, 0>, &mat::set_mat_value<2, 0>));
		s.set("zy", v8pp::property(&mat::get_mat_value<2, 1>, &mat::set_mat_value<2, 1>));
		s.set("zz", v8pp::property(&mat::get_mat_value<2, 2>, &mat::set_mat_value<2, 2>));
		s.set("zw", v8pp::property(&mat::get_mat_value<2, 3>, &mat::set_mat_value<2, 3>));
		s.set("wx", v8pp::property(&mat::get_mat_value<3, 0>, &mat::set_mat_value<3, 0>));
		s.set("wy", v8pp::property(&mat::get_mat_value<3, 1>, &mat::set_mat_value<3, 1>));
		s.set("wz", v8pp::property(&mat::get_mat_value<3, 2>, &mat::set_mat_value<3, 2>));
		s.set("ww", v8pp::property(&mat::get_mat_value<3, 3>, &mat::set_mat_value<3, 3>));

		custom_mat_module_common();

		ctx->set(("mat3" + postfix).c_str(), s);
	}
*/
	void register_structs(context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		/*
		register_vec2<i32>("i", ctx);
		register_vec2<u32>("ui", ctx);
		register_vec2<f32>("f", ctx);

		register_vec3<i32>("i", ctx);
		register_vec3<u32>("ui", ctx);
		register_vec3<f32>("f", ctx);

		register_vec4<i32>("i", ctx);
		register_vec4<u32>("ui", ctx);
		register_vec4<f32>("f", ctx);

		register_mat2<i32>("i", ctx);
		register_mat2<u32>("ui", ctx);
		register_mat2<f32>("f", ctx);

		register_mat3<i32>("i", ctx);
		register_mat3<u32>("ui", ctx);
		register_mat3<f32>("f", ctx);

		register_mat4<i32>("i", ctx);
		register_mat4<u32>("ui", ctx);
		register_mat4<f32>("f", ctx);
		*/
		
		//average
		{
			class_<average, v8pp::raw_ptr_traits> s(global->GetIsolate());
			register_class_state(s);
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

		/*
		module m(isolate);
		m.set("translationi", &translationi);
		m.set("translationui", &translationui);
		m.set("translationf", &translationf);
		m.set("rotation", &rotation);
		m.set("scalei", &scalei);
		m.set("scaleui", &scaleui);
		m.set("scalef", &scalef);
		global->Set(v8str("Transform"), m.new_instance());
		*/
	}

	void release_math_objects() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<average>()).remove_objects();
		/*
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<4, i32, vec4_custom<i32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<4, u32, vec4_custom<u32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<4, f32, vec4_custom<f32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<3, i32, vec3_custom<i32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<3, u32, vec3_custom<u32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<3, f32, vec3_custom<f32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<2, i32, vec2_custom<i32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<2, u32, vec2_custom<u32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<2, f32, vec2_custom<f32>>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec4_custom<i32>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec4_custom<u32>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec4_custom<f32>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec3_custom<i32>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec3_custom<u32>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec3_custom<f32>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec2_custom<i32>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec2_custom<u32>>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec2_custom<f32>>()).remove_objects();
		*/
	}

	void reset_math_object_storage() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<average>()).reset_objects_map();
		/*
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<4, i32, vec4_custom<i32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<4, u32, vec4_custom<u32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<4, f32, vec4_custom<f32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<3, i32, vec3_custom<i32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<3, u32, vec3_custom<u32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<3, f32, vec3_custom<f32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<2, i32, vec2_custom<i32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<2, u32, vec2_custom<u32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mat_custom<2, f32, vec2_custom<f32>>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec4_custom<i32>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec4_custom<u32>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec4_custom<f32>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec3_custom<i32>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec3_custom<u32>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec3_custom<f32>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec2_custom<i32>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec2_custom<u32>>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vec2_custom<f32>>()).reset_objects_map();
		*/
	}
};