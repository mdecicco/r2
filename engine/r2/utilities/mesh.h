#pragma once
#include <r2/utilities/indexbuffer.h>
#include <r2/utilities/vertexbuffer.h>
#include <r2/utilities/instancebuffer.h>

#include <r2/bindings/v8helpers.h>

namespace r2 {
    class mesh_construction_data {
        public:
			mesh_construction_data(vertex_format* vtx_fmt, index_type idx_typ = it_unsigned_byte, instance_format* ins_fmt = nullptr);
            ~mesh_construction_data();

			void set_max_vertex_count(u32 count);
			u32 max_vertex_count() const { return m_vertex_count; }
			u32 vertex_count() const { return m_last_vertex_idx; }

			void set_max_index_count(u32 count);
			u32 max_index_count() const { return m_index_count; }
			u32 index_count() const { return m_last_index_idx; }

			void set_max_instance_count(u32 count);
			u32 max_instance_count() const { return m_instance_count; }
			u32 instance_count() const { return m_last_instance_idx; }

            vertex_format* vertexFormat() const { return m_vertexFormat; }
            void append_vertex_data(void* vdata);
            const void* vertex_data() const { return m_vertices; }

            index_type indexType() const { return m_indexType; }
            void append_index_data(void* idata);
            const void* index_data() const { return m_indices; }

            instance_format* instanceFormat() const { return m_instanceFormat; }
            void append_instance_data(void* idata);
			void update_instance(u32 idx, void* idata);
            const void* instance_data() const { return m_instances; }

			template<typename T>
			void append_vertex(const T& v) {
				if (sizeof(T) != m_vertexFormat->size()) {
					r2Error("Attempting to append vertex (%d bytes) to mesh which has a different vertex size (%d bytes) than the one being appended", sizeof(T), m_vertexFormat->size());
					return;
				}
				append_vertex_data((void*)&v);
			}

			template<typename T>
			void append_index(const T& i) {
				if (sizeof(T) != m_indexType) {
					r2Error("Attempting to append index (%d bytes) to mesh which has a different index size (%d bytes) than the one being appended", sizeof(T), m_indexType);
					return;
				}
				append_index_data((void*)&i);
			}

			template<typename T>
			void append_instance(const T& i) {
				if (sizeof(T) != m_instanceFormat->size()) {
					r2Error("Attempting to append instance (%d bytes) to mesh which has a different instance size (%d bytes) than the one being appended", sizeof(T), m_instanceFormat->size());
					return;
				}
				append_instance_data((void*)&i);
			}

			// js functions
			void append_vertices_v8(v8Args args);
			void append_indices_v8(v8Args args);
			void append_instances_v8(v8Args args);

        protected:
			friend class scene;
			bool m_wasSentToGpu;

            //not optional
			u32 m_vertex_count;
			u32 m_last_vertex_idx;
            vertex_format* m_vertexFormat;
            u8* m_vertices;

            //optional
			u32 m_index_count;
			u32 m_last_index_idx;
            index_type m_indexType;
			u8* m_indices;

			u32 m_instance_count;
			u32 m_last_instance_idx;
            instance_format* m_instanceFormat;
			u8* m_instances;
    };
}
