class TestComponent {
	constructor() {
		this.value = 0;
	}
};

class TestSystemState {
	constructor() {
		this.increment = 1;
	}
};

class TestSystem extends engine.System {
	constructor () {
		super(TestSystemState, TestComponent, "test");
	}
	
	bind (entity) {
		
	}
	
	unbind (entity) {
		
	}
	
	handleEvent (event) {
		
	}
	
	tick (dt) {
		
	}
};

const system = new TestSystem();
engine.register_system(system);
