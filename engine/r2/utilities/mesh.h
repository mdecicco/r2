#pragma once

#include <vector>
#include <string>
using namespace std;

#include <r2/utilities/indexbuffer.h>
#include <r2/utilities/vertexbuffer.h>
#include <r2/utilities/instancebuffer.h>

namespace r2 {
    class mesh_construction_data {
        public:
			mesh_construction_data(vertex_format* vtx_fmt, index_type idx_typ = it_unsigned_byte, const instance_format* ins_fmt = nullptr);
            ~mesh_construction_data();

			void set_max_vertex_count(u32 count);
			void set_max_index_count(u32 count);
			void set_max_instance_count(u32 count);

            const vertex_format& vertexFormat() const { return m_vertexFormat; }
            void append_vertex(void* vdata);
            size_t vertex_count() const { return m_last_vertex_idx; }
            const void* vertex_data() const { return m_vertices; }

            index_type indexType() const { return m_indexType; }
            void append_index(void* idata);
            size_t index_count() const { return m_last_index_idx; }
            const void* index_data() const { return m_indices; }

            const instance_format& instanceFormat() const { return m_instanceFormat; }
            void append_instance(void* idata);
			void update_instance(u32 idx, void* idata);
            size_t instance_count() const { return m_last_instance_idx; }
            const void* instance_data() const { return m_instances; }


			template<typename T>
			void append_vertex(const T& v) {
				if (sizeof(T) != m_vertexFormat.size()) {
					r2Error("Attempting to append vertex (%d bytes) to mesh which has a different vertex size (%d bytes) than the one being appended", sizeof(T), m_vertexFormat.size());
					return;
				}
				append_vertex((void*)&v);
			}

			template<typename T>
			void append_index(const T& i) {
				if (sizeof(T) != m_indexType) {
					r2Error("Attempting to append index (%d bytes) to mesh which has a different index size (%d bytes) than the one being appended", sizeof(T), m_indexType);
					return;
				}
				append_index((void*)&i);
			}

			template<typename T>
			void append_instance(const T& i) {
				if (sizeof(T) != m_instanceFormat.size()) {
					r2Error("Attempting to append instance (%d bytes) to mesh which has a different instance size (%d bytes) than the one being appended", sizeof(T), m_instanceFormat.size());
					return;
				}
				append_instance((void*)&i);
			}


        protected:
            //not optional
			u32 m_vertex_count;
			u32 m_last_vertex_idx;
            vertex_format m_vertexFormat;
            u8* m_vertices;

            //optional
			u32 m_index_count;
			u32 m_last_index_idx;
            index_type m_indexType;
			u8* m_indices;

			u32 m_instance_count;
			u32 m_last_instance_idx;
            instance_format m_instanceFormat;
			u8* m_instances;
    };
}
