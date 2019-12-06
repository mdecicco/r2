#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace r2 {
	template<int l, typename T, typename V>
	struct mat_custom;

	#define custom_vec_common(name, comp_count) \
		name() : glm_t() { } \
		name add(const name& rhs) { return *this + rhs; } \
		name sub(const name& rhs) { return *this - rhs; } \
		name mul(const name& rhs) { return *this * rhs; } \
		name div(const name& rhs) { return *this / rhs; } \
		name addEq(const name& rhs) { return *this = *this + rhs; } \
		name subEq(const name& rhs) { return *this = *this - rhs; } \
		name mulEq(const name& rhs) { return *this = *this * rhs; } \
		name divEq(const name& rhs) { return *this = *this / rhs; } \
		name addScalar(T rhs) { return *this + rhs; } \
		name subScalar(T rhs) { return *this - rhs; } \
		name mulScalar(T rhs) { return *this * rhs; } \
		name divScalar(T rhs) { return *this / rhs; } \
		name addEqScalar(T rhs) { return *this = *this + rhs; } \
		name subEqScalar(T rhs) { return *this = *this - rhs; } \
		name mulEqScalar(T rhs) { return *this = *this * rhs; } \
		name divEqScalar(T rhs) { return *this = *this / rhs; } \
		name mulMatrix(const mat_custom<comp_count, T, name<T>>& rhs); \
		name mulEqMatrix(const mat_custom<comp_count, T, name<T>>& rhs); \
		T dot(const name& rhs); \
		void normalize(); \
		name normalized(); \
		T magnitude(); \
		T distance(const name& rhs);


	template<typename T>
	struct vec2_custom : public glm::vec<2, T, glm::packed_highp> {
		using glm_t = glm::vec<2, T, glm::packed_highp>;
		vec2_custom(T _x, T _y) : glm_t(_x, _y) { }
		vec2_custom(const vec2_custom<T>& o) : glm_t(o) { }
		vec2_custom(const glm_t& o) : glm_t(o) { }
		custom_vec_common(vec2_custom, 2)
	};

	template<typename T>
	struct vec3_custom : public glm::vec<3, T, glm::packed_highp> {
		using glm_t = glm::vec<3, T, glm::packed_highp>;
		vec3_custom(T _x, T _y, T _z) : glm_t(_x, _y, _z) { }
		vec3_custom(const vec3_custom<T>& o) : glm_t(o) { }
		vec3_custom(const glm_t& o) : glm_t(o) { }
		custom_vec_common(vec3_custom, 3)

		vec3_custom<T> cross(const vec3_custom<T>& rhs);
	};

	template<typename T>
	struct vec4_custom : public glm::vec<4, T, glm::packed_highp> {
		using glm_t = glm::vec<4, T, glm::packed_highp>;
		vec4_custom(T _x, T _y, T _z, T _w) : glm_t(_x, _y, _z, _w) { }
		vec4_custom(const vec4_custom<T>& o) : glm_t(o) { }
		vec4_custom(const glm_t& o) : glm_t(o) { }
		custom_vec_common(vec4_custom, 4)
	};

	template<int l, typename T, typename V>
	struct mat_custom : public glm::mat<l, l, T, glm::packed_highp> {
		using glm_t = glm::mat<l, l, T, glm::packed_highp>;
		using this_t = mat_custom<l, T, V>;
		mat_custom() : glm_t(T(1)) { }
		mat_custom(const this_t& o) : glm_t(o) { }
		mat_custom(const glm_t& o) : glm_t(o) { }

		template <int r, int c>
		T get_mat_value() { return (*this)[r][c]; }

		template <int r, int c>
		void set_mat_value(T val) { (*this)[r][c] = val; }

		this_t mul(const this_t& rhs) { return *this * rhs; }
		this_t mulEq(const this_t& rhs) { return *this = *this * rhs; }
		void setIdentity() { *this = glm_t(T(1)); }
		this_t transposed() { return glm::detail::compute_transpose<l, l, T, glm::packed_highp, glm::detail::is_aligned<glm::packed_highp>::value>::call(*this); }
		this_t transpose() { return *this = glm::detail::compute_transpose<l, l, T, glm::packed_highp, glm::detail::is_aligned<glm::packed_highp>::value>::call(*this); }
		V mulVector(const V& rhs) { return *this * rhs; }
		this_t inverse();
		this_t invert();
	};
};