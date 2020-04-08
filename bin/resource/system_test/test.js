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
		super(
			/*
				The name of the system in the engine. This is the name that gets
				passed to entity.add_component() in order to add this system's
				component to the entity and make it available to the system
			 */
			'test',
			
			/*
				Each engine state has it's own corresponding system state,
				so it must be defined here as a class that can be constructed
				for each engine state.
			*/
			TestSystemState,
			
			/*
				An instance of this class is constructed for each entity that
				has add_component('test') called on it
			 */
			TestComponent,
			
			/*
				Optionally, this name of the accessor on the entity that's used
				when getting the component data. If this parameter isn't passed
				then the system name will be used.
				
				entity.test will be an instance of TestComponent
			 */
			'test'
		);
		
		/*
			Set the system's tick frequency to 1 Hz
			It's a good idea to not implement performance critical systems
			in JS.
		 */
		this.set_update_frequency(100);
	}
	
	/*
		The bind function is called whenever this system's component is added
		to an entity. The purpose of this function is to add any properties to
		the new entity that should be accessible from other scripts that relate
		to this system. For example, one could add a function as a property to
		an entity that would raise a flag in this system's component that would
		change the behavior for the component during the next tick.
		
		This effect could also be achieved by adding the function to the component
		class and using entity.test.doSomething(...)
	 */
	bind (entity) {
		engine.log(entity.id, entity.name);
		
		// this.state will refer to the instance of TestSystemState
		// that corresponds to the current engine state
		engine.log(this.state);
	}
	
	/*
		The unbind function is called whenever this system's component is removed
		from an entity. The purpose of this function is to delete any properties
		added in the bind function so they are no longer accessible from other
		scripts
	 */
	unbind (entity) {
		
	}
	
	/*
		Called whenever an event is fired that this system subscribes to with
		this.subscribe
	 */
	handleEvent (event) {
		
	}
	
	/*
		The tick function is called when it's time for the system to update the
		components. The frequency at which this function is called depends on
		the value passed to this.set_update_frequency. By default it will be
		called once per frame, though that is not recommended for scripted
		systems.
		
		frameDt - The amount of time it took for the last frame to be produced (in seconds)
		updateDt - The amount of time since the last time this function was called (in seconds)
	 */
	tick (frameDt, updateDt) {
		engine.log(`TestSystem::tick(${frameDt.toFixed(2)}, ${updateDt.toFixed(2)})`);
		
		/*
			this.query_components will return a subset of the components maintained
			by this system, or all components maintained by this system if no filters
			are passed.
			
			TODO: document filters
			TODO: add filters...
		 */
		const components = this.query_components();
		components.forEach(comp => {
			engine.log(comp.id, comp.entity.id, comp.entity.name, comp.value);
			comp.value += this.state.increment;
		});
	}
};

const system = new TestSystem();
engine.register_system(system);

engine.open_window({
	width: 1920 / 2,
	height: 1080,
	title: "Test",
	can_resize: true
});
gfx.set_driver(gfx.RenderDriver.OpenGL);

var entity = new engine.Entity('Test Entity');
entity.add_component('test');
