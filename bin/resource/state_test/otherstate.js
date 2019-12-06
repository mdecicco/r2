class OtherTestState extends engine.State {
	constructor(name) {
		super(name, Memory.Megabytes(8));
		engine.log("OtherTestState constructor");
	}

	willBecomeActive = () => {
		this.windowOpen = true;
		this.logWindowOpen = true;
		this.selection = 0;
		this.text = "";
		this.logs = [];
		this.log_len = 10;
		this.fps_history = [];
		this.frq_avg = new RollingAverage(200);
		this.max_freq_avg = new RollingAverage(200);
		this.aud_history = [];
		this.fps_history_len = 200;
		this.update_freq = 30.0;
		this.update_load = 1;
		this.set_update_frequency(30.0);
		engine.log("OtherTestState willBecomeActive");
	}

	becameActive = () => {
		engine.log("OtherTestState becameActive");
		engine.dispatch(new Event("after_active", { a: 6 }));
		this.logs = engine.logs();
		if (this.logs.length > this.log_len) this.logs = this.logs.slice(this.logs.length - this.log_len);
	}

	handleEvent = (evt) => {
		if (evt.name == "log") {
			this.logs.push(evt.data);
			if (this.logs.length > this.log_len) this.logs = this.logs.slice(1);
		} else engine.log(evt);
	}

	update = (dt) => {
		const avg_update_dur = this.get_average_update_duration();

		this.fps_history.push(1.0 / dt);
		this.aud_history.push(1.0 / avg_update_dur);

		if (this.fps_history.length > this.fps_history_len) this.fps_history = this.fps_history.slice(1);
		if (this.aud_history.length > this.fps_history_len) this.aud_history = this.aud_history.slice(1);

		this.frq_avg.sample(1.0 / dt);
		this.max_freq_avg.sample(1.0 / avg_update_dur);

		// artificial load
		for(let i = 0;i < this.update_load;i++) {
			const values = [];
			for(let j = 0;j < 10;j++) values.push(Math.random());
			const min = values.reduce((min, p) => p < min ? p : min, values[0]);
			const max = values.reduce((max, p) => p > max ? p : max, values[0]);
		}
	}

	render = () => {
		if (this.windowOpen) {
			ImGui.Begin("Test Window", () => { this.windowOpen = false; }, [ImGui.WindowFlags.None]);
				ImGui.Combo("Combo Label", this.selection, ["One", "Two", "Three"], 5, v => {
					this.selection = v;
				});
				ImGui.InputText("State Name", this.text, 32, [ImGui.InputTextFlags.None], (text) => {
					this.text = text;
				});
				if (ImGui.Button("Set State", [0, 0])) {
					engine.activate_state(this.text);
				}
				ImGui.DragInt("Log History Length", this.log_len, 1, 1, 200, "%d", 1, v => {
					this.log_len = v;
					this.logs = engine.logs();
					if (this.logs.length > this.log_len) this.logs = this.logs.slice(this.logs.length - this.log_len);
				});
				ImGui.DragInt("Frequency Samples", this.fps_history_len, 1, 1, 200, "%d", 1, v => {
					this.fps_history_len = v;
					if (this.fps_history.length > this.fps_history_len) this.fps_history = this.fps_history.slice(this.fps_history.length - this.fps_history_len);
					if (this.aud_history.length > this.fps_history_len) this.aud_history = this.aud_history.slice(this.aud_history.length - this.fps_history_len);
				});
				ImGui.DragInt("Artificial Load Multiplier", this.update_load, 1, 1, 100000, "%d", 1, v => { this.update_load = v; });
				ImGui.DragFloat("Desired Update Frequency (Hz)", this.update_freq, 0.5, 10, 5000, "%.2f", 1, v => {
					this.update_freq = v;
					this.set_update_frequency(v);
				});

				ImGui.Text("The actual frequency can only be as high as the frame rate.");
				ImGui.PlotLines("##actfreq", this.fps_history, 0, `Actual Frequency (~${this.frq_avg.average.toFixed(1)} Hz)`, 0, 120, [400, 200]);
				ImGui.PlotLines("##maxfreq", this.aud_history, 0, `Max Frequency (~${this.max_freq_avg.average.toFixed(1)} Hz)`, 0, 120, [400, 200]);
			ImGui.End();
		}
		if (this.logWindowOpen) {
			ImGui.Begin("Log Window", () => { this.logWindowOpen = false; }, [ImGui.WindowFlags.None]);
				const color = new vec4f(0.5, 0.5, 0.5, 1);
				this.logs.forEach(l => {
					color.x = 0.5; color.y = 0.5; color.z = 0.5;
					if (l.type == "warning") { color.x = 1; color.y = 1; color.z = 0; }
					if (l.type == "error") { color.x = 1; color.y = 0; color.z = 0; }
					ImGui.PushStyleColor(ImGui.Color.Text, color);
					ImGui.TextWrapped(`[${l.time.toFixed(2)}][${l.type}] ${l.text}`);
					ImGui.PopStyleColor(1);
				});
				ImGui.SetScrollHere(1.0);
			ImGui.End();
		}
	}

	willBecomeInactive = () => {
		engine.log("OtherTestState willBecomeInactive");
	}
	
	becameInactive = () => {
		engine.log("OtherTestState becameInactive");
	}

	willBeDestroyed = () => {
		engine.log("OtherTestState willBeDestroyed");
	}
};

export {
	OtherTestState
};
