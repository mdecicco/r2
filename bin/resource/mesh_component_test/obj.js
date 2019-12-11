class ObjFile {
    constructor(objFilePath) {
        try {
            const file = IO.LoadFile(objFilePath, IO.Mode.Text);
            const lines = [];
            while (!file.at_end()) lines.push(file.read_line());
            IO.Close(file);
            
            const positions = [];
            const normals = [];
            const texCoords = [];
            
            this.vertices = [];
            this.indices = [];
            
            const fvIndices = {};
            
            const pf = parseFloat;
            const v3 = (x, y, z) => [x, y, z];
            const v2 = (x, y) => [x, y];
            
            lines.forEach(l => {
                const comps = l.split(' ').filter(c => c.length > 0);
                if (comps.length > 0) {
                    if (comps[0] === 'v') {
                        if (comps.length === 4) {
                            positions.push(v3(pf(comps[1]), pf(comps[2]), pf(comps[3])));
                        }
                    } else if (comps[0] === 'vn') {
                        if (comps.length === 4) {
                            normals.push(v3(pf(comps[1]), pf(comps[2]), pf(comps[3])));
                        }
                    } else if (comps[0] === 'vt') {
                        if (comps.length === 3) {
                            texCoords.push(v2(pf(comps[1]), pf(comps[2])));
                        }
                    } else if (comps[0] === 'f') {
                        if (comps.length === 4) {
                            comps.slice(1).forEach(c => {
                                if (fvIndices[c] !== undefined) this.indices.push(fvIndices[c]);
                                else {
                                    let idxGroup = [];
                                    let idx = '';
                                    
                                    Array.from(c).forEach(ch => {
                                        if (ch === '/') {
                                            if (idx.length > 0) idxGroup.push(parseInt(idx, 10));
                                            else idxGroup.push(-1);
                                            idx = '';
                                        } else idx += ch;
                                    });
                                    
                                    if (idx.length > 0) idxGroup.push(parseInt(idx, 10));
                                    else idxGroup.push(-1);
                                    
                                    const vertex = { };
                                    idxGroup.forEach((vidx, idx) => {
                                        if (idx === 0 && vidx !== -1) vertex.pos = positions[vidx - 1];
                                        //if (idx === 1 && vidx !== -1) vertex.tex = texCoords[vidx - 1];
                                        if (idx === 2 && vidx !== -1) vertex.normal = normals[vidx - 1];
                                    });
                                    this.vertices.push(vertex);
                                    this.indices.push(this.vertices.length - 1);
                                    fvIndices[c] = this.vertices.length - 1;
                                }
                            });
                        }
                    }
                }
            });
            
            this.center = new vec3f(0, 0, 0);
            this.vertices.forEach(v => {
                this.center.addEq(new vec3f(v.pos));
            });
            this.center.scale(1.0 / this.vertices.length);
            
        } catch (e) {
            engine.error(e);
        }
    }
    makeNode(vfmt, ifmt, instances) {
        let itype = gfx.IndexType.ubyte;
        if (this.indices.length > 256) itype = gfx.IndexType.ushort;
        if (this.indices.length > 32768) itype = gfx.IndexType.uint;
        const out = {
            vfmt,
            ifmt,
            mesh: new gfx.MeshInfo(vfmt, itype, ifmt),
            node: null
        };
        
        out.mesh.max_vertices = this.vertices.length;
        out.mesh.max_indices = this.indices.length;
        out.mesh.max_instances = instances.length;
        out.mesh.appendVertices(this.vertices);
        out.mesh.appendIndices(this.indices);
        out.mesh.appendInstances(instances);
        out.node = out.mesh.makeRenderable();
        
        return out;
    }
    print () {
        engine.log('{');
        engine.log(`    'mesh': {`);
        engine.log(`        'vertices': [`);
        this.vertices.forEach((v, idx) => {
            engine.log(`            { pos: [ ${v.pos.x}, ${v.pos.y}, ${v.pos.z} ], normal: [ ${v.normal.x}, ${v.normal.y}, ${v.normal.z} ] },`);
        });
        engine.log(`        ],`);
        engine.log(`        'indices': `, this.indices);
        engine.log('    }');
        engine.log('}');
    }
};

export {
    ObjFile
};
