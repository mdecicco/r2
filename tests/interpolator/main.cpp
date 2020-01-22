#include <r2/utilities/interpolator.hpp>
#include <glm/glm.hpp>
using namespace r2;

int main(int argc, char** argv) {
	bool done = false;

	interpolator<f32> i(0, 5, interpolate::easeInOutQuad);
	(i = 1.0f).then([&done]() { done = true; });

	while(!done) {
		printf("%0.2f\n", f32(i));
	}

	done = false;

	interpolator<glm::vec2> v(glm::vec2(0.0f, 0.0f), 5.0f, interpolate::easeOutCubic);
	(v = glm::vec2(1.0f, 1.0f)).then([&done]() { done = true; });

	while(!done) {
		glm::vec2 d = v;
		printf("%0.2f, %0.2f\n", d.x, d.y);
	}

	return 0;
}
