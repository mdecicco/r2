#include <r2/engine.h>
#include <memory.h>
#include <stdio.h>

class test_asset : public r2::asset {
    public:
        test_asset(int param1, float param2) {
            printf("test_asset(%d, %f)\n", param1, param2);
            m_int = param1;
            m_float = param2;
        }
        virtual ~test_asset() {

        }

        virtual bool deserialize(const unsigned char* data, size_t length) {
            r2::mstring s((char*)data, length);
            printf("----CONTENTS----\n%s\n----END CONTENTS----\n",s.c_str());
            return true;
        }
        virtual bool serialize(unsigned char** data,size_t* length) {
            char* out = new char[32];
            memset(out, 0, 32);
            *length = snprintf(out, 32, "Testy: %d: %f\n", m_int, m_float);
            *data = new unsigned char[*length];
            memcpy(*data, out, *length);
            delete [] out;
            return true;
        }
    protected:
        int m_int;
        float m_float;
};

int main(int argc,char** argv) {
	r2::r2engine::create(argc, argv);
	r2::r2engine* eng = r2::r2engine::get();
    r2::asset_man* assets = eng->assets();

    // no errors here, unless it can't find the file
    test_asset* ass0 = assets->create<test_asset>("test_asset0", 0, 0);
    ass0->load("asset_test/test_asset.txt");

    // should cause a warning at shutdown time
    test_asset* ass1 = assets->create<test_asset>("test_asset1", 45, 12.35f);
    ass1->save("asset_test/test.txt");

    assets->destroy(ass0);

    int ret = eng->run();
	eng->shutdown();
    return ret;
}
