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
		
		// make the jiggle animation differ between the two entities
		this.time = this.id * 3.0;
		
		engine.log(`mesh has ${this.vertex_count} vertices and ${this.index_count} indices`);
		
		// only do this once, since both mesh entities share the same vertices
		if (this.id === 1) {
			const vertices = this.get_vertices();
			vertices.forEach(v => {
				// position
				v[0].addEq(vec3f.random(0.1));
				
				// normal
				v[1].addEq(vec3f.random(0.1)).normalize();
			});
			this.set_vertices(vertices);
		}
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
		
		// so that the camera can track the (untranslated) center and make it look wonkier
		this.center = this.meshCenter.transformed(transform);
		
		transform.mulEq(Transform3D.translation(new vec3f((this.id - 1.5) * 5, 0, 0)));
		this.instance = { transform };
	}
};

export {
	Mesh
};
