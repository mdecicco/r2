const { OtherTestState } = require('./otherstate.js');
const { TestState } = require('./state.js');

engine.open_window({
	width: 1920 / 2,
	height: 1080,
	title: "Test",
	can_resize: true
});

gfx.set_driver(gfx.RenderDriver.OpenGL);

var state = new TestState("TestState");
var otherState = new OtherTestState("OtherTestState");
engine.register_state(state);
engine.register_state(otherState);
engine.activate_state("OtherTestState");
