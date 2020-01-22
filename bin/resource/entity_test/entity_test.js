class TestEntity extends engine.Entity {
	constructor() {
		super(`TestEntity`);
		this.destroy_timer = 5;
		engine.log(`\n\n\nnew TestEntity created (id: ${this.id})`);
	}
	
	wasInitialized = () => {
		engine.log(`TestEntity(${this.id}) wasInitialized`);
		this.set_update_frequency(2);
		this.subscribe("test_event");
	}
	
	test_callback = (a, b) => {
		engine.log(`TestEntity(${this.id}) test_callback(${a}, ${b})`);
		
		// properties don't exist
		engine.log(`TestEntity(${this.id}) test_component -> ${this.x}, ${this.y}, ${this.z}`);
		
		engine.log(`TestEntity(${this.id}) add_test_component();`);
		this.add_test_component();
		
		// now they do
		engine.log(`TestEntity(${this.id}) test_component -> ${this.x}, ${this.y}, ${this.z}`);
		
		this.x = 1;
		this.y = 2;
		this.z = 3;
		
		// now they're different
		engine.log(`TestEntity(${this.id}) test_component -> ${this.x}, ${this.y}, ${this.z}`);
		
		engine.log(`TestEntity(${this.id}) remove_test_component();`);
		this.remove_test_component();
		
		// now they're gone
		engine.log(`TestEntity(${this.id}) test_component -> ${this.x}, ${this.y}, ${this.z}`);
		
		this.add_test_component();
		this.x = 0;
		this.y = 0;
		this.z = 0;
	}
	
	handleEvent = (evt) => {
		engine.log(`TestEntity(${this.id}) handleEvent`, evt.data);
	}
	
	update = (frameDt, updateDt) => {
		engine.log(`TestEntity(${this.id}) update (fps: ${(1.0 / frameDt).toFixed(2)}  ups: ${(1.0 / updateDt).toFixed(2)}), (comp(${this.test_component_id()}): ${this.x}, ${this.y}, ${this.z}), destroying in ${this.destroy_timer}...`);
		
		if (this.destroy_timer === 0) {
			engine.dispatch(new Event("test_event", { a: 6 }));
			new TestEntity();
			this.destroy();
		}
		this.destroy_timer--;
	}
	
	willBeDestroyed = () => {
		engine.log(`TestEntity(${this.id}) willBeDestroyed`);
	}
};

// make sure engine runs
engine.open_window({
	width: 1920 / 2,
	height: 1080,
	title: "Test",
	can_resize: true
});
gfx.set_driver(gfx.RenderDriver.OpenGL);

var entity = new TestEntity();
