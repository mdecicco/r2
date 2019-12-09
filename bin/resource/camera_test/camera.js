class Camera extends engine.Entity {
	constructor() {
		super(`Camera`);
	}
	
	wasInitialized = () => {
		engine.log(`Camera (${this.id}) wasInitialized`);
		this.set_update_frequency(2);
		this.add_transform_component();
	}
	
	update = (frameDt, updateDt) => {
		engine.log(`Camera (${this.id}) update (${frameDt.toFixed(2)}, ${updateDt.toFixed(2)})`);
	}
};

export {
	Camera
};
