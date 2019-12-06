engine.log("");
engine.log("");
engine.log("");

class MeshTest {
    constructor() {
        this.vfmt = new gfx.VertexFormat();
        this.ifmt = new gfx.InstanceFormat();
        this.mfmt = new gfx.UniformFormat();
        this.mesh = null;
        this.material = null;
        this.node = null;
    }
    
    setupVertexFormat() {
        engine.log('setupVertexFormat');
        this.vfmt.addAttr(gfx.VertexAttrType.vec2f);
        this.vfmt.addAttr(gfx.VertexAttrType.vec2f);
    }
    
    setupInstanceFormat() {
        engine.log('setupInstanceFormat');
        this.ifmt.addAttr(gfx.InstanceAttrType.vec2f);
        this.ifmt.addAttr(gfx.InstanceAttrType.float);
        this.ifmt.addAttr(gfx.InstanceAttrType.float);
        this.ifmt.addAttr(gfx.InstanceAttrType.int);
    }
    
    setupMaterialFormat() {
        engine.log('setupMaterialFormat');
        this.mfmt.addAttr("color[0]", gfx.UniformAttrType.vec3f);
        this.mfmt.addAttr("color[1]", gfx.UniformAttrType.vec3f);
        this.mfmt.addAttr("color[2]", gfx.UniformAttrType.vec3f);
        this.mfmt.addAttr("color[3]", gfx.UniformAttrType.vec3f);
    }
    
    setupMaterial() {
        engine.log('setupMaterial');
        this.material = new gfx.Material("u_material", this.mfmt);
        this.material.shader = gfx.load_shader("test", "./resource/test_shader.glsl");
    }
    
    buildMesh() {
        engine.log('buildMesh');
        this.mesh = new gfx.MeshInfo(this.vfmt, gfx.IndexType.ubyte, this.ifmt);
        this.mesh.max_vertices = 4;
        this.mesh.max_indices = 6;
        this.mesh.max_instances = 5;

        this.mesh.appendVertices([
            { pos: new vec2f(-0.5,  0.5), tex: new vec2f(0, 1) },
            { pos: new vec2f( 0.5,  0.5), tex: new vec2f(1, 1) },
            { pos: new vec2f( 0.5, -0.5), tex: new vec2f(1, 0) },
            { pos: new vec2f(-0.5, -0.5), tex: new vec2f(0, 0) }
        ]);

        this.mesh.appendInstances([
            { pos: new vec2f( 0.0, 0.0), scale: -0.5, rot: 0.0, colorIdx: 0 },
            { pos: new vec2f(-0.5, 0.0), scale:  0.5, rot: 0.0, colorIdx: 1 },
            { pos: new vec2f( 0.5, 0.0), scale: -0.5, rot: 0.0, colorIdx: 2 },
            { pos: new vec2f( 0.0,-0.5), scale:  0.5, rot: 0.0, colorIdx: 2 },
            { pos: new vec2f( 0.0, 0.5), scale: -0.5, rot: 0.0, colorIdx: 1 }
        ]);
        this.mesh.appendIndices([0, 1, 2, 0, 2, 3]);
    }
    
    makeRenderable() {
        engine.log('makeRenderable');
        this.node = this.mesh.makeRenderable();
        this.node.material_instance = this.material.instantiate();
        this.node.material_instance.uniforms.vec3f("color[0]", new vec3f(1, 1, 0));
        this.node.material_instance.uniforms.vec3f("color[1]", new vec3f(1, 0, 0));
        this.node.material_instance.uniforms.vec3f("color[2]", new vec3f(0, 1, 0));
        this.node.material_instance.uniforms.vec3f("color[3]", new vec3f(0, 0, 1));
    }
    
    run() {
        this.setupVertexFormat();
        this.setupInstanceFormat();
        this.setupMaterialFormat();
        this.setupMaterial();
        this.buildMesh();
        this.makeRenderable();
    }
};

const test = new MeshTest();
test.run();

engine.log("");
engine.log("");
engine.log("");
