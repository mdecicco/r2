class TestState {
	constructor () {
		engine.log("TestState constructor");
	}

	willBecomeActive () {
		engine.log("TestState willBecomeActive");
	}

	becameActive () {
		engine.log("TestState becameActive");
	}

	willBecomeInactive () {
		engine.log("TestState willBecomeInactive");
	}

	willBeDestroyed () {
		engine.log("TestState willBeDestroyed");
	}
};

var state = new TestState();