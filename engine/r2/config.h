#pragma once
#pragma warning(disable: 26812)
#define r2_debug

#include <limits.h>
#include <float.h>
#include <stdint.h>
#include <chrono>

#define GLM_FORCE_XYZW_ONLY
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef r2_debug
#define r2Log(...) r2::r2engine::get()->log("debug", __VA_ARGS__);
#define r2Warn(...) r2::r2engine::get()->log("warning", __VA_ARGS__);
#define r2Error(...) r2::r2engine::get()->log("error", __VA_ARGS__);
#endif

#ifdef _WIN32
	#define getcwd _getcwd
	#define chdir _chdir
#endif


#define _CRT_NO_VA_START_VALIDATION

namespace r2 {
	typedef intptr_t					word;

	typedef int8_t                      i8;
	typedef int16_t                     i16;
	typedef int32_t                     i32;
	typedef int64_t                     i64;

	typedef uint8_t                     u8;
	typedef uint16_t                    u16;
	typedef uint32_t                    u32;
	typedef uint64_t                    u64;

	typedef signed char                 s8;
	typedef signed short                s16;
	typedef signed int                  s32;
	typedef signed long long            s64;

	typedef float                       f32;
	typedef double                      f64;

	typedef glm::vec<2, i32, glm::packed_highp>		vec2i;
	typedef glm::vec<2, u32, glm::packed_highp>		vec2ui;
	typedef glm::vec<2, f32, glm::packed_highp>     vec2f;
	typedef glm::vec<3, i32, glm::packed_highp>     vec3i;
	typedef glm::vec<3, u32, glm::packed_highp>     vec3ui;
	typedef glm::vec<3, f32, glm::packed_highp>     vec3f;
	typedef glm::vec<4, i32, glm::packed_highp>     vec4i;
	typedef glm::vec<4, u32, glm::packed_highp>     vec4ui;
	typedef glm::vec<4, f32, glm::packed_highp>     vec4f;
	typedef glm::mat<2, 2, i32, glm::packed_highp>  mat2i;
	typedef glm::mat<2, 2, u32, glm::packed_highp>  mat2ui;
	typedef glm::mat<2, 2, f32, glm::packed_highp>  mat2f;
	typedef glm::mat<3, 3, i32, glm::packed_highp>  mat3i;
	typedef glm::mat<3, 3, u32, glm::packed_highp>  mat3ui;
	typedef glm::mat<3, 3, f32, glm::packed_highp>  mat3f;
	typedef glm::mat<4, 4, i32, glm::packed_highp>  mat4i;
	typedef glm::mat<4, 4, u32, glm::packed_highp>  mat4ui;
	typedef glm::mat<4, 4, f32, glm::packed_highp>  mat4f;

	typedef std::chrono::high_resolution_clock tmr;
	typedef std::chrono::duration<f32> dur;
}
