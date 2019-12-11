class Mesh extends engine.Entity {
	constructor(node, meshCenter) {
		super('Mesh');
		this.instanceOf = node;
		this.meshCenter = meshCenter;
		this.center = new vec3f(0, 0, 0);
	}
	
	wasInitialized = () => {
		engine.log(`Mesh (${this.id}) wasInitialized`);
		this.set_update_frequency(60);
		this.add_transform_component();
		this.add_mesh_component();
		this.node = this.instanceOf;
		this.time = 0.0;
	}
	
	update = (frameDt, updateDt) => {
		this.time += updateDt;
		const jiggleAccel = Math.sin(this.time * 10);
		const fac = this.time % 6.28318530718;
		const jiggleFactor = Math.sin(fac) * jiggleAccel;
		const transform = Transform3D.scale(new vec3f(
			1 + (jiggleFactor * 0.2),
			1 + (jiggleFactor * 0.8),
			1 + (jiggleFactor * 0.2)
		));
		this.center = this.meshCenter.transformed(transform);
		transform.mulEq(Transform3D.translation(new vec3f((this.id - 1.5) * 5, 0, 0)));
		this.instance_data = { transform };
	}
};

export {
	Mesh
};
