const { ObjFile } = require('obj');
const { Camera } = require('camera');

function format_size(sz) {
	let mult = 1.0;
	let unit = 'B';
	if (sz > 1024) { mult = 1.0 / 1024.0; unit = 'KB'; }
	if (sz > 1024 * 1024) { mult = 1.0 / (1024.0 * 1024.0); unit = 'MB'; }
	if (sz > 1024 * 1024 * 1024) { mult = 1.0 / (1024.0 * 1024.0 * 1024.0); unit = 'GB'; }

	return `${(sz * mult).toFixed(2)} ${unit}`;
}

class CameraTestState extends engine.State {
	constructor () {
		super("CameraTestState", Memory.Megabytes(16));
	}
	willBecomeActive = () => { }
	becameActive = () => {
		// load mesh and make renderable
		
		const o = new ObjFile("teapot.obj");
		// o.print();
		
		const vfmt = new gfx.VertexFormat();
		const ifmt = new gfx.InstanceFormat();
		const mfmt = new gfx.UniformFormat();
		vfmt.addAttr(gfx.VertexAttrType.vec3f);
		vfmt.addAttr(gfx.VertexAttrType.vec3f);
		ifmt.addAttr(gfx.InstanceAttrType.mat4f);
		mfmt.addAttr("color", gfx.UniformAttrType.vec3f);
		
		const transform = Transform3D.translation(new vec3f(0, 0, 0));
		transform.mulEq(Transform3D.rotation(new vec3f(0, 0, 1), 90));
		transform.mulEq(Transform3D.scale(new vec3f(1.25, 1.25, 1.25)));
		
		const info = o.makeNode(vfmt, ifmt, [{ transform }]);
		
		const material = new gfx.Material("u_material", mfmt);
		material.shader = gfx.load_shader("test", "./resource/camera_test/shader.glsl");
		
		info.node.material_instance = material.instantiate();
		info.node.material_instance.uniforms.vec3f("color", new vec3f(0.75, 0.1, 0.2));
		
		const camera = new Camera();
	}
	render = () => {
		ImGui.Text(`Memory: ${format_size(this.used_memory)} / ${format_size(this.max_memory)}`);
		if (ImGui.Button('Reset State', { w: 190, h: 20 })) {
			engine.activate_state("CameraTestState");
		}
	}
	willBecomeInactive = () => { }
	becameInactive = () => { }
	willBeDestroyed = () => { engine.log("CameraTestState destroyed"); }
};

engine.open_window({
	width: 512,
	height: 512,
	title: "Test",
	can_resize: true
});
gfx.set_driver(gfx.RenderDriver.OpenGL);

var state = new CameraTestState();
engine.register_state(state);
engine.activate_state("CameraTestState");
