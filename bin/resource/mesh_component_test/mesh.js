class Mesh extends engine.Entity {
	constructor(node) {
		super('Mesh');
		this.instanceOf = node;
		this.position = vec3f.random(10.0);
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
		/*
		if (this.id === 1) {
			const vertices = this.get_vertices();
			vertices.forEach(v => {
				// position
				v[0].scale(vec3f.random(0.1));
				
				// normal
				v[1].addEq(vec3f.random(0.1)).normalize();
			});
			this.set_vertices(vertices);
		}
		*/
		
		this.instance = { transform: this.getTransform() };
	}
	
	getTransform = () => {
		const jiggleAccel = Math.sin(this.time * 6);
		const fac = this.time % 6.28318530718;
		const jiggleFactor = Math.sin(fac) * jiggleAccel;
		const scale = new vec3f(
			1 + (jiggleFactor * 0.2),
			1 + (jiggleFactor * 0.8),
			1 + (jiggleFactor * 0.2)
		);
		return Transform3D.rotationTranslationScale(quat.fromEuler(0, 1, 0), this.position, scale);
	}
	
	update = (frameDt, updateDt) => {
		this.time += updateDt;
		
		this.instance = { transform: this.getTransform() };
	}
};

export {
	Mesh
};
