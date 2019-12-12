const { ObjFile } = require('obj');
const { Camera } = require('camera');
const { Mesh } = require('mesh');

function format_size(sz) {
	let mult = 1.0;
	let unit = 'B';
	if (sz > 1024) { mult = 1.0 / 1024.0; unit = 'KB'; }
	if (sz > 1024 * 1024) { mult = 1.0 / (1024.0 * 1024.0); unit = 'MB'; }
	if (sz > 1024 * 1024 * 1024) { mult = 1.0 / (1024.0 * 1024.0 * 1024.0); unit = 'GB'; }

	return `${(sz * mult).toFixed(2)} ${unit}`;
}

class MeshTestState extends engine.State {
	constructor () {
		super("MeshTestState", Memory.Megabytes(24));
	}
	willBecomeActive = () => { }
	becameActive = () => {
		// to do:
		// integrate assimp and expose mesh loading functions c++ side
		// maybe make require()ing an object return a function to produce a render
		// node with a specified vertex format, and optional instance format, index
		// type, max instance count.
		
		// load mesh and make renderable
		
		const o = new ObjFile("cube.obj");
		// o.print();
		
		const vfmt = new gfx.VertexFormat();
		const ifmt = new gfx.InstanceFormat();
		const mfmt = new gfx.UniformFormat();
		vfmt.addAttr(gfx.VertexAttrType.vec3f);
		vfmt.addAttr(gfx.VertexAttrType.vec3f);
		ifmt.addAttr(gfx.InstanceAttrType.mat4f);
		mfmt.addAttr("color", gfx.UniformAttrType.vec3f);
		
		const transform = Transform3D.scale(new vec3f(1.25, 1.25, 1.25));
		
		const entity_count = 80;
		
		const transforms = [];
		for(var i = 0;i < entity_count;i++) transforms.push({ transform });
		
		const info = o.makeNode(vfmt, ifmt, transforms);
		
		const material = new gfx.Material("u_material", mfmt);
		material.shader = gfx.load_shader("test", "./resource/mesh_component_test/shader.glsl");
		
		info.node.material_instance = material.instantiate();
		info.node.material_instance.uniforms.vec3f("color", new vec3f(0.75, 0.1, 0.2));
		
		this.meshes = [];
		for(var i = 0;i < entity_count;i++) {
			this.meshes.push(new Mesh(info.node));
		}
		this.camera = new Camera();
	}
	render = () => {
		ImGui.Text(`Memory: ${format_size(this.used_memory)} / ${format_size(this.max_memory)}`);
		ImGui.Text(`FPS: ${engine.frame_rate().toFixed(2)}`);
		if (ImGui.Button('Reset State', { w: 190, h: 20 })) {
			engine.activate_state("MeshTestState");
		}
	}
	willBecomeInactive = () => { }
	becameInactive = () => { }
	willBeDestroyed = () => { engine.log("MeshTestState destroyed"); }
};

engine.open_window({
	width: 512,
	height: 512,
	title: "Test",
	can_resize: true
});
gfx.set_driver(gfx.RenderDriver.OpenGL);

var state = new MeshTestState();
engine.register_state(state);
engine.activate_state("MeshTestState");
