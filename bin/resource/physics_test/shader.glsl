// vertex
#version 330
layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vnorm;
layout (location = 2) in mat4 transform;

out vec3 o_norm;
out vec3 o_color;

layout (std140) uniform u_material { vec3 color; } material;
layout (std140) uniform u_scene { mat4 transform; mat4 projection; mat4 view_proj; } scene;
layout (std140) uniform u_model { mat4 transform; } model;

void main() {
    gl_Position = scene.view_proj * transform * vec4(vpos, 1.0);
    o_norm = (inverse(transpose(transform)) * vec4(vnorm, 1.0)).xyz;
    o_color = normalize(vpos);
};

// fragment
#version 330

in vec3 o_color;
in vec3 o_norm;

out vec4 frag_color;

void main() {
    frag_color = vec4(max(dot(o_norm, normalize(vec3(1, 1, 1))), 0.1) * o_color, 1);
}
