#pragma once

#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

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

            void add_attr(const string& name, uniform_attribute_type type);
            bool operator==(const uniform_format& rhs) const;
			const vector<uniform_attribute_type>& attributes() const;
			const vector<string>& attributeNames() const;

            size_t size() const;
			size_t offsetOf(u16 attrIdx) const;
            string to_string() const;
			string hash_name() const;

        protected:
            vector<uniform_attribute_type> m_attrs;
			vector<string> m_attrNames;
			vector<size_t> m_attrOffsets;
            size_t m_uniformSize;
            string m_fmtString;
			string m_hashName;
    };

    class uniform_buffer;
    struct ufm_bo_segment : public gpu_buffer_segment{
		uniform_buffer* buffer;
    };

	class uniform_block {
		public:
			uniform_block(const string& name, const ufm_bo_segment& buffer_segment);
			~uniform_block();

			void uniform(const string& name, const void* value);
			string name() const;

			const ufm_bo_segment& buffer_info() const;

			void uniform_int   (const string& name, const i32&    value);
			void uniform_uint  (const string& name, const u32&    value);
			void uniform_float (const string& name, const f32&    value);
			void uniform_vec2i (const string& name, const vec2i&  value);
			void uniform_vec2ui(const string& name, const vec2ui& value);
			void uniform_vec2f (const string& name, const vec2f&  value);
			void uniform_vec3i (const string& name, const vec3i&  value);
			void uniform_vec3ui(const string& name, const vec3ui& value);
			void uniform_vec3f (const string& name, const vec3f&  value);
			void uniform_vec4i (const string& name, const vec4i&  value);
			void uniform_vec4ui(const string& name, const vec4ui& value);
			void uniform_vec4f (const string& name, const vec4f&  value);
			void uniform_mat2i (const string& name, const mat2i&  value);
			void uniform_mat2ui(const string& name, const mat2ui& value);
			void uniform_mat2f (const string& name, const mat2f&  value);
			void uniform_mat3i (const string& name, const mat3i&  value);
			void uniform_mat3ui(const string& name, const mat3ui& value);
			void uniform_mat3f (const string& name, const mat3f&  value);
			void uniform_mat4i (const string& name, const mat4i&  value);
			void uniform_mat4ui(const string& name, const mat4ui& value);
			void uniform_mat4f (const string& name, const mat4f&  value);

		protected:
			string m_name;

			ufm_bo_segment getSegmentForField(const string& name) const;

			ufm_bo_segment m_bufferSegment;
			unordered_map<string, u16> m_uniformOffsetMap;
			unordered_map<string, u16> m_uniformIndexMap;
			unordered_map<string, size_t> m_uniformSizeMap;
	};

    class uniform_buffer : public gpu_buffer {
        public:
			uniform_buffer(const uniform_format& fmt, size_t max_count);
            ~uniform_buffer();

            const uniform_format& format() const;
			virtual void* data() const;

            ufm_bo_segment append(const void* data, size_t count);
			void update(const ufm_bo_segment& segment, const void* data);

        protected:
			uniform_format m_format;
            size_t m_uniformCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}