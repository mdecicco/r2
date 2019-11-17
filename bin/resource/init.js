run('./resource/otherstate.js');

var state = new OtherTestState("OtherTestState");
engine.register_state(state);
engine.activate_state("OtherTestState");