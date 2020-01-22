// vertex
#version 330
layout (location = 0) in vec3 vpos;
layout (location = 1) in vec4 vcol;

out vec4 o_color;

layout (std140) uniform u_scene {
	mat4 view;
	mat4 projection;
	mat4 view_proj;
} scene;

void main() {
    gl_Position = scene.view_proj * vec4(vpos, 1.0);
    o_color = vcol;
};

// fragment
#version 330

in vec4 o_color;

out vec4 frag_color;

void main() {
    frag_color = o_color;
}
