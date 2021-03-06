class TestState extends engine.State {
	constructor (name) {
		super(name, Memory.Megabytes(4));
		engine.log("TestState constructor");
	}

	willBecomeActive = () => {
		engine.log("TestState willBecomeActive");
	}

	becameActive = () => {
		engine.log("TestState becameActive");
		engine.activate_state("OtherTestState");
	}

	willBecomeInactive = () => {
		engine.log("TestState willBecomeInactive");
	}
	
	becameInactive = () => {
		engine.log("TestState becameInactive");
	}

	willBeDestroyed = () => {
		engine.log("TestState willBeDestroyed");
	}
};

export {
	TestState
};
