#include <r2/utilities/buffer.h>

namespace r2 {
    static size_t nextBufferId = 0;
    mesh_buffer::mesh_buffer() {
        m_id = nextBufferId;
        nextBufferId++;
    }
    mesh_buffer::~mesh_buffer() {
    }
}
