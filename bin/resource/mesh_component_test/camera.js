class Camera extends engine.Entity {
	constructor() {
		super(`Camera`);
	}
	
	wasInitialized = () => {
		engine.log(`Camera (${this.id}) wasInitialized`);
		this.set_update_frequency(60);
		this.add_transform_component();
		this.add_camera_component();
		this.activate();
		this.time = 0.0;
		
		this.target = new vec3f(0, 0, 0);
		this.position = new vec3f(0, 0, 0);
		this.up = new vec3f(0, 1, 0);
	}
	
	update = (frameDt, updateDt) => {
		this.time += updateDt;
		this.position = vec3.rotateY(new vec3f(0, 4, -14), this.target, this.time * 30.0);
		var windowSize = engine.window_size();
		this.projection = Projection.perspective(60.0, windowSize.x / windowSize.y, 0.001, 100.0);
		this.transform = View.lookAt(this.position, this.target, this.up);
	}
};

export {
	Camera
};
