class OtherTestState {
	constructor () {
		engine.log("OtherTestState constructor");
		
		engine.dispatch(new Event("before_active", { a: 5 }));
		engine.open_window({
			width: 800,
			height: 600,
			title: "Test"
		});

		this.time = 0;
		this.windowOpen = true;
		this.selection = 0;
		this.text = "";
	}

	willBecomeActive () {
		engine.log("OtherTestState willBecomeActive");
	}

	becameActive () {
		engine.log("OtherTestState becameActive");
		engine.dispatch(new Event("after_active", { a: 6 }));
	}

	handleEvent (evt) {
		engine.log(`handleEvent: ${evt.name()} ->`, evt.data());
	}

	update (dt) {
		this.time += dt;
		if (this.time > 1.0) {
			engine.log(`fps: ${1.0 / dt}`);
			this.time -= 1.0;
		}
	}

	render () {
		if(this.windowOpen) {
			ImGui.Begin("Test Window", () => { this.windowOpen = false; }, [ImGui.WindowFlags.None]);
				ImGui.Combo("Combo Label", this.selection, ["One", "Two", "Three"], 5, v => {
					this.selection = v;
				});
				ImGui.InputText("Text Label", this.text, 32, [ImGui.InputTextFlags.None], (text) => {
					this.text = text;
				});
			ImGui.End();
		}
	}

	willBecomeInactive () {
		engine.log("OtherTestState willBecomeInactive");
	}

	willBeDestroyed () {
		engine.log("OtherTestState willBeDestroyed");
	}
};

var state = new OtherTestState();