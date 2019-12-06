#pragma once
#include <r2/managers/memman.h>
#include <r2/utilities/buffer.h>

namespace r2 {
    class r2engine;

    enum uniform_attribute_type {
        uat_int = 0, uat_uint  , uat_float,
        uat_vec2i  , uat_vec2ui, uat_vec2f,
        uat_vec3i  , uat_vec3ui, uat_vec3f,
        uat_vec4i  , uat_vec4ui, uat_vec4f,
        uat_mat2i  , uat_mat2ui, uat_mat2f,
        uat_mat3i  , uat_mat3ui, uat_mat3f,
        uat_mat4i  , uat_mat4ui, uat_mat4f
    };

    class uniform_format {
        public:
			uniform_format();
			uniform_format(const uniform_format& o);
            ~uniform_format();

            void add_attr(const mstring& name, uniform_attribute_type type);
            bool operator==(const uniform_format& rhs) const;
			const mvector<uniform_attribute_type>& attributes() const;
			uniform_attribute_type attrType(u16 attrIdx) const;
			const mvector<mstring>& attributeNames() const;
			mstring attrName(u16 attrIdx) const;

            size_t size() const;
			size_t offsetOf(u16 attrIdx) const;
            mstring to_string() const;
			mstring hash_name() const;

        protected:
            mvector<uniform_attribute_type> m_attrs;
			mvector<mstring> m_attrNames;
			mvector<size_t> m_attrOffsets;
            size_t m_uniformSize;
            mstring m_fmtString;
			mstring m_hashName;
    };

    class uniform_buffer;
    struct ufm_bo_segment : public gpu_buffer_segment {
		ufm_bo_segment() : gpu_buffer_segment(), buffer(nullptr) { }
		uniform_buffer* buffer;
    };

	class uniform_block {
		public:
			uniform_block(const mstring& name, const ufm_bo_segment& buffer_segment);
			~uniform_block();

			void uniform(const mstring& name, const void* value);
			mstring name() const;

			const ufm_bo_segment& buffer_info() const;

			void uniform_int   (const mstring& name, const i32&    value);
			void uniform_uint  (const mstring& name, const u32&    value);
			void uniform_float (const mstring& name, const f32&    value);
			void uniform_vec2i (const mstring& name, const vec2i&  value);
			void uniform_vec2ui(const mstring& name, const vec2ui& value);
			void uniform_vec2f (const mstring& name, const vec2f&  value);
			void uniform_vec3i (const mstring& name, const vec3i&  value);
			void uniform_vec3ui(const mstring& name, const vec3ui& value);
			void uniform_vec3f (const mstring& name, const vec3f&  value);
			void uniform_vec4i (const mstring& name, const vec4i&  value);
			void uniform_vec4ui(const mstring& name, const vec4ui& value);
			void uniform_vec4f (const mstring& name, const vec4f&  value);
			void uniform_mat2i (const mstring& name, const mat2i&  value);
			void uniform_mat2ui(const mstring& name, const mat2ui& value);
			void uniform_mat2f (const mstring& name, const mat2f&  value);
			void uniform_mat3i (const mstring& name, const mat3i&  value);
			void uniform_mat3ui(const mstring& name, const mat3ui& value);
			void uniform_mat3f (const mstring& name, const mat3f&  value);
			void uniform_mat4i (const mstring& name, const mat4i&  value);
			void uniform_mat4ui(const mstring& name, const mat4ui& value);
			void uniform_mat4f (const mstring& name, const mat4f&  value);

		protected:
			mstring m_name;

			ufm_bo_segment getSegmentForField(const mstring& name) const;

			ufm_bo_segment m_bufferSegment;
			munordered_map<mstring, u16> m_uniformOffsetMap;
			munordered_map<mstring, u16> m_uniformIndexMap;
			munordered_map<mstring, size_t> m_uniformSizeMap;
	};

    class uniform_buffer : public gpu_buffer {
        public:
			uniform_buffer(uniform_format* fmt, size_t max_count);
            ~uniform_buffer();

            uniform_format* format() const;
			virtual void* data() const;

            ufm_bo_segment append(const void* data);
			void update(const ufm_bo_segment& segment, const void* data);

        protected:
			uniform_format* m_format;
            size_t m_uniformCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };

	namespace static_uniform_formats {
		uniform_format* scene();
		uniform_format* node();
	};
}