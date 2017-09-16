#include <r2/engine.h>
using namespace r2;

class test_vertex {
    public:
        test_vertex(float _x = 0,float _y = 0,float _u = 0,float _v = 0) {
            x = _x; y = _y;
            u = _u; v = _v;
        }
        ~test_vertex() { }

        float x, y;
        float u, v;
};

class test_instance_data {
    public:
        test_instance_data(float x = 0,float y = 0,float _scale = 1) {
            pos_x = x; pos_y = y;
            scale = _scale;
        }
        ~test_instance_data() { }

        float pos_x, pos_y;
        float scale;
};

typedef render_mesh<test_vertex,unsigned char,test_instance_data> instanced_mesh;

int main(int argc,char** argv) {
    r2engine* eng = new r2engine(argc,argv);
    scene* scene1 = eng->scenes()->create("scene1");

    // define mesh characteristics
    vertex_format vfmt;
    vfmt.add_attr(vat_float); // x
    vfmt.add_attr(vat_float); // y
    vfmt.add_attr(vat_float); // u
    vfmt.add_attr(vat_float); // v

    instance_format ifmt;
    ifmt.add_attr(iat_float); // pos_x
    ifmt.add_attr(iat_float); // pos_y
    ifmt.add_attr(iat_float); // scale

    // create the mesh
    instanced_mesh* mesh = new instanced_mesh(&vfmt,it_byte,&ifmt);

    // single quad vertices (clockwise)
    mesh->append_vertex(test_vertex(-0.5, 0.5));
    mesh->append_vertex(test_vertex( 0.5, 0.5));
    mesh->append_vertex(test_vertex( 0.5,-0.5));
    mesh->append_vertex(test_vertex(-0.5,-0.5));

    // top right triangle indices
    mesh->append_index(0); // top left
    mesh->append_index(1); // top right
    mesh->append_index(2); // bottom right

    // bottom left triangle indices
    mesh->append_index(0); // top left
    mesh->append_index(2); // bottom right
    mesh->append_index(3); // bottom left

    // a single instance at 0, 0 (I know... This is just a test)
    mesh->append_instance(test_instance_data(0,0,1.0));

    render_node* node = scene1->add_mesh(mesh);

    // start the engine
    int r = eng->run();

    eng->scenes()->destroy(scene1);
    delete eng;

    return r;
}
