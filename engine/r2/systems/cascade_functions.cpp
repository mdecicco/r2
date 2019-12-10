#include <r2/systems/cascade_functions.h>

namespace r2 {
	mat4f cascade_mat4f(const mat4f& parent, const mat4f& child) { return parent * child; }
};