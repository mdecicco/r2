#include <r2/engine.h>
#include <memory.h>

namespace r2 {
	static mstring attr_names[21] = {
		"int"  , "uint"  , "float",
		"vec2i", "vec2ui", "vec2f",
		"vec3i", "vec3ui", "vec3f",
		"vec4i", "vec4ui", "vec4f",
		"mat2i", "mat2ui", "mat2f",
		"mat3i", "mat3ui", "mat3f",
		"mat4i", "mat4ui", "mat4f",
	};

	static mstring attr_hash_names[21] = {
		"i"  , "u"  , "f"  ,
		"2i" , "2u" , "2f" ,
		"3i" , "3u" , "3f" ,
		"4i" , "4u" , "4f" ,
		"m2i", "m2u", "m2f",
		"m3i", "m3u", "m3f",
		"m4i", "m4u", "m4f",
	};



	// uniform format

    uniform_format::uniform_format() {
        m_uniformSize = 0;
    }

    uniform_format::uniform_format(const uniform_format& o) {
        m_attrs = o.m_attrs;
		m_attrNames = o.m_attrNames;
		m_attrOffsets = o.m_attrOffsets;
        m_uniformSize = o.m_uniformSize;
        m_fmtString = o.m_fmtString;
		m_hashName = o.m_hashName;
    }

    uniform_format::~uniform_format() {
    }

    void uniform_format::add_attr(const mstring& name, uniform_attribute_type type) {
		render_driver* driver = r2engine::get()->renderer()->driver();
		if (!driver) {
			r2Error("A render driver needs to be specified in order to add an attribute to a uniform format");
			return;
		}

		for (auto attr : m_attrNames) {
			if (attr == name) {
				r2Error("A uniform attribute named \"%s\" already exists in the format { %s }", name.c_str(), this->to_string().c_str());
				return;
			}
		}

        m_attrs.push_back(type);
		m_attrNames.push_back(name);
		m_attrOffsets.push_back(m_uniformSize);

        m_uniformSize += driver->get_uniform_attribute_size(type);
        if(m_fmtString.length() > 0) m_fmtString += ", ";
        m_fmtString += attr_names[type] + " " + name;
		m_hashName += attr_hash_names[type];
    }

    bool uniform_format::operator==(const uniform_format &rhs) const {
        if(rhs.m_attrs.size() != m_attrs.size()) return false;
        for(int i = 0;i < m_attrs.size();i++) {
            if(m_attrs[i] != rhs.m_attrs[i]) return false;
        }

        return true;
    }

	uniform_attribute_type uniform_format::attrType(u16 attrIdx) const {
		return m_attrs[attrIdx];
	}

	const mvector<uniform_attribute_type>& uniform_format::attributes() const {
		return m_attrs;
	}

	mstring uniform_format::attrName(u16 attrIdx) const {
		return m_attrNames[attrIdx];
	}

	const mvector<mstring>& uniform_format::attributeNames() const {
		return m_attrNames;
	}

    size_t uniform_format::size() const {
        return m_uniformSize;
    }

	size_t uniform_format::offsetOf(u16 attrIdx) const {
		if (attrIdx > m_attrs.size()) {
			r2Error("uniform_format::offsetOf: Attribute index %d out of range (format only has %d attributes).", attrIdx, m_attrs.size());
			return 0;
		}

		return m_attrOffsets[attrIdx];
	}

    mstring uniform_format::to_string() const {
        return m_fmtString;
    }

	mstring uniform_format::hash_name() const {
		return m_hashName;
	}



	// uniform buffer

    uniform_buffer::uniform_buffer(uniform_format* fmt, size_t max_count) : gpu_buffer(max_count * fmt->size()) {
        m_format = fmt;
        m_maxCount = max_count;
        m_uniformCount = 0;
        r2Log("Allocating %s for a maximum of %d uniform blocks of format [%s] (buf: %d)", format_size(m_format->size() * max_count), max_count, m_format->to_string().c_str(), m_id);
        m_data = new unsigned char[m_format->size() * max_count];
    }

    uniform_buffer::~uniform_buffer() {
        r2Log("Deallocating uniform buffer: (%s / %s | %zu / %zu uniform blocks used) (buf: %d)", format_size(used_size()), format_size(max_size()), m_uniformCount, m_maxCount, m_id);
        if(m_data) delete [] m_data;
        m_data = 0;
    }

	uniform_format* uniform_buffer::format() const {
        return m_format;
    }

	void* uniform_buffer::data() const {
		return m_data;
	}

    ufm_bo_segment uniform_buffer::append(const void *data) {
		render_driver* driver = r2engine::get()->renderer()->driver();
		if (!driver) {
			r2Error("A render driver needs to be specified in order to append to a uniform buffer");
			ufm_bo_segment seg;
			seg.buffer = nullptr;
			memset(&seg, 0, sizeof(ufm_bo_segment));
			return seg;
		}

		size_t beginOffset = used_size();
		size_t blockAlignment = driver->get_uniform_buffer_block_offset_alignment();
		size_t alignmentDiff = beginOffset % blockAlignment;
		if (alignmentDiff != 0) {
			beginOffset += blockAlignment - alignmentDiff;
			assert(beginOffset % blockAlignment == 0);
		}

        ufm_bo_segment seg;
        seg.buffer = this;
        seg.begin = m_uniformCount;
        seg.memBegin = beginOffset;
        seg.end = m_uniformCount + 1;
        seg.memEnd = seg.memBegin + m_format->size();

		if(seg.memEnd > max_size()) {
			r2Error("Insufficient space in uniform buffer of format [%s] for uniform block (buf: %d)", m_format->to_string().c_str(), m_id);
			ufm_bo_segment seg;
			seg.buffer = nullptr;
			memset(&seg, 0, sizeof(ufm_bo_segment));
			return seg;
		}

		r2Log("Buffering data range: %zu -> %zu (buf: %d)", seg.memBegin, seg.memEnd, m_id);
        memcpy((u8*)m_data + seg.memBegin, data, seg.memsize());
        m_uniformCount += 1;

		appended(seg.memBegin, seg.memEnd);

        return seg;
    }
	
	void uniform_buffer::update(const ufm_bo_segment& seg, const void* data) {
		if (!seg.is_valid()) {
			r2Error("Attempted to update invalid segment of uniform buffer. Ignoring");
			return;
		}
		if (seg.buffer != this) {
			r2Error("Segment for another uniform buffer passed to uniform_buffer::update");
			return;
		}

		if (seg.memBegin > m_format->size() * m_uniformCount) {
			r2Error("Out of range segment.memBegin (%llu) provided to uniform_buffer::update", seg.memBegin);
			return;
		}

		if (seg.memEnd > m_format->size() * m_uniformCount) {
			r2Error("Out of range segment.memEnd (%llu) provided to uniform_buffer::update", seg.memEnd);
			return;
		}

		memcpy((u8*)m_data + seg.memBegin, data, seg.memsize());
		updated(seg.memBegin, seg.memEnd);
	}



	// uniform block
	uniform_block::uniform_block(const mstring& name, const ufm_bo_segment& buffer_segment) : m_name(name), m_bufferSegment(buffer_segment) {
		uniform_format* fmt = m_bufferSegment.buffer->format();
		auto attrNames = fmt->attributeNames();
		auto attrs = fmt->attributes();
		for (u16 i = 0;i < attrNames.size();i++) {
			uniform_attribute_type type = attrs[i];
			m_uniformOffsetMap[attrNames[i]] = fmt->offsetOf(i);
			m_uniformSizeMap[attrNames[i]] = r2engine::get()->renderer()->driver()->get_uniform_attribute_size(type);
			m_uniformIndexMap[attrNames[i]] = i;
		}
	}

	uniform_block::~uniform_block() {
		// TODO: tell the buffer this block's segment is free
	}

	void uniform_block::uniform(const mstring& name, const void* value) {
		if (m_uniformOffsetMap.count(name) == 0) {
			r2Error("No uniform with name \"%s\" exists in block \"%s\". Ignoring", name.c_str(), m_name.c_str());
			return;
		}
		uniform_attribute_type type = m_bufferSegment.buffer->format()->attributes()[m_uniformIndexMap[name]];
		static u8 castSpace[2048];
		
		// cast value to the format that the driver expects to see, if necessary
		r2engine::get()->renderer()->driver()->serialize_uniform_value(value, castSpace, type);

		// fill the buffer segment with the value in the new format
		m_bufferSegment.buffer->update(getSegmentForField(name), castSpace);
	}

	mstring uniform_block::name() const {
		return m_name;
	}

	const ufm_bo_segment& uniform_block::buffer_info() const {
		return m_bufferSegment;
	}

	void uniform_block::uniform_int   (const mstring& name, const i32&    value) { uniform(name, &value); }
	void uniform_block::uniform_uint  (const mstring& name, const u32&    value) { uniform(name, &value); }
	void uniform_block::uniform_float (const mstring& name, const float&  value) { uniform(name, &value); }
	void uniform_block::uniform_vec2i (const mstring& name, const vec2i&  value) { uniform(name, &value); }
	void uniform_block::uniform_vec2f (const mstring& name, const vec2f&  value) { uniform(name, &value); }
	void uniform_block::uniform_vec2ui(const mstring& name, const vec2ui& value) { uniform(name, &value); }
	void uniform_block::uniform_vec3i (const mstring& name, const vec3i&  value) { uniform(name, &value); }
	void uniform_block::uniform_vec3f (const mstring& name, const vec3f&  value) { uniform(name, &value); }
	void uniform_block::uniform_vec3ui(const mstring& name, const vec3ui& value) { uniform(name, &value); }
	void uniform_block::uniform_vec4i (const mstring& name, const vec4i&  value) { uniform(name, &value); }
	void uniform_block::uniform_vec4f (const mstring& name, const vec4f&  value) { uniform(name, &value); }
	void uniform_block::uniform_vec4ui(const mstring& name, const vec4ui& value) { uniform(name, &value); }
	void uniform_block::uniform_mat2i (const mstring& name, const mat2i&  value) { uniform(name, &value); }
	void uniform_block::uniform_mat2f (const mstring& name, const mat2f&  value) { uniform(name, &value); }
	void uniform_block::uniform_mat2ui(const mstring& name, const mat2ui& value) { uniform(name, &value); }
	void uniform_block::uniform_mat3i (const mstring& name, const mat3i&  value) { uniform(name, &value); }
	void uniform_block::uniform_mat3f (const mstring& name, const mat3f&  value) { uniform(name, &value); }
	void uniform_block::uniform_mat3ui(const mstring& name, const mat3ui& value) { uniform(name, &value); }
	void uniform_block::uniform_mat4i (const mstring& name, const mat4i&  value) { uniform(name, &value); }
	void uniform_block::uniform_mat4f (const mstring& name, const mat4f&  value) { uniform(name, &value); }
	void uniform_block::uniform_mat4ui(const mstring& name, const mat4ui& value) { uniform(name, &value); }

	ufm_bo_segment uniform_block::getSegmentForField(const mstring& name) const {
		u16 attrIdx = m_uniformIndexMap.at(name);

		ufm_bo_segment seg;
		seg.memBegin = m_bufferSegment.memBegin + m_uniformOffsetMap.at(name);
		seg.memEnd = seg.memBegin + m_uniformSizeMap.at(name);
		seg.buffer = m_bufferSegment.buffer;
		return seg;
	}



	namespace static_uniform_formats {
		static uniform_format sceneFmt;
		static uniform_format nodeFmt;

		uniform_format* scene() {
			if (sceneFmt.size() > 0) return &sceneFmt;
			if (!r2engine::get()->renderer()->driver()) { r2Error("The static scene uniform format can't be created until a render driver is specified"); }
			else {
				sceneFmt.add_attr("transform", uat_mat4f);
				sceneFmt.add_attr("projection", uat_mat4f);
				sceneFmt.add_attr("view_proj", uat_mat4f);
			}
			return &sceneFmt;
		}
		uniform_format* node() {
			if (nodeFmt.size() > 0) return &nodeFmt;
			if (!r2engine::get()->renderer()->driver()) { r2Error("The static node uniform format can't be created until a render driver is specified"); }
			else {
				nodeFmt.add_attr("transform", uat_mat4f);
			}
			return &nodeFmt;
		}
	};
};
