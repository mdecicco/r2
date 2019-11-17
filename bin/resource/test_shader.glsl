// vertex
#version 330
layout (location = 0) in vec2 vpos;
layout (location = 1) in vec2 vtex;
layout (location = 2) in vec2 position;
layout (location = 3) in float scale;
layout (location = 4) in int color_index;

layout (std140) uniform u_material { vec3 color[4]; } material;

out vec3 o_color;

void main() {
	gl_Position = vec4((vpos * scale) + position, 0.0, 1.0);
	o_color = vec3(vtex, vpos.x) * material.color[color_index];
};

// fragment
#version 330

in vec3 o_color;

out vec4 frag_color;

void main() {
    frag_color = vec4(o_color, 1);
}