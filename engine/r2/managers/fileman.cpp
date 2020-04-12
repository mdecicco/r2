#include <r2/engine.h>

#ifdef _WIN32
    #include <r2/utilities/dirent.h>
	#include <direct.h>
    #define atoll _atoi64
#else
    #include <dirent.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <memory.h>
#endif

#include <stdarg.h>
#include <ctype.h>
#include <cstdlib>
#include <limits.h>
#include <float.h>
#include <stdlib.h>
using namespace std;

namespace r2 {
	engine_state_data* state_container_factory::create() {
		return (engine_state_data*)new state_containers();
	}

	state_containers::~state_containers() {
		if (containers.size() > 0) {
			r2Warn("State \"%s\" has %lu unreleased data containers at destruction time", m_state->name().c_str(), containers.size());
			for(auto container : containers) {
				r2Warn("Destroying unreleased container: %s", container->m_name.c_str());
				delete container;
			}
			containers.clear();
		}

		if (directories.size() > 0) {
			r2Warn("State \"%s\" has %lu unreleased directories at destruction time", m_state->name().c_str(), directories.size());
			for(auto directory : directories) {
				r2Warn("Destroying unreleased directory info: %s", directory->m_path.c_str());
				delete directory;
			}
			directories.clear();
		}
	}
	
	i32 _snprintf(char* Out,size_t MaxLen,const char* fmt,...) {
		//Windows friendly
		va_list Args;
		va_start(Args,fmt);
		#ifdef _WIN32
			i32 r = vsnprintf_s(Out,MaxLen,MaxLen,fmt,Args);
		#else
			i32 r = vsnprintf(Out,MaxLen,fmt,Args);
		#endif
		va_end(Args);
		return r;
	}
	bool IsWhitespace(char c) {
		return c == '\t' || c == ' ';
	}
	bool IsNumber(char c) {
		return isdigit(c);
	}

	#define ParseInteger(Type, Min, Max, MinFmt, MaxFmt) \
	{ \
		char c = 0; \
		if(!read(c)) { \
			r2Error("Failed to parse integer from data container (%s)", m_name.c_str()); \
			return false; \
		} \
		while(IsWhitespace(c)) { \
			if(!read(c)) { \
				r2Error("Failed to parse integer from data container (%s)", m_name.c_str()); \
				return false; \
			} \
		} \
		bool IsNeg = c == '-'; \
		if(IsNeg) { \
			if(!read(c)) { r2Error("Failed to parse integer from data container (%s)",m_name.c_str()); return false; } \
		} \
		mstring s; \
		while(IsNumber(c)) { \
			s += c; \
			if(!read(c)) { \
				r2Error("Failed to parse integer from data container (%s)", m_name.c_str()); \
				return false; \
			} \
		} \
		long long i = atoll(s.c_str()); \
		if(i > Max) { \
			r2Warn("Parsing integer from data container (%s) that will not fit in data type ("#Type")", m_name.c_str()); \
			r2Log("Info: Parsed value: %lli | Min/Max value for data type: "#MinFmt"/"#MaxFmt, i, Min, Max); \
		} \
		if(IsNeg) data = -i; \
		else data = i; \
	}

	#define writeInteger(Type,MaxStrLen,fmt) \
	{ \
		char Out[MaxStrLen]; \
		i32 r = _snprintf(Out,MaxStrLen,#fmt,data); \
		if(r < 0) { \
			r2Error("Failed to convert integer (" #Type ") to mstring for data container (%s): call to snprintf returned %d", m_name.c_str(),r); \
			return false; \
		} \
		else return write_data(Out,r); \
	}



    void directory_info::populate(const mstring& dir) {
        if(m_entries) { delete [] m_entries; m_entries = 0; }
        m_entryCount = 0;

        DIR *dp = opendir(dir.c_str());
        if(dp != NULL) {
            struct dirent *ep = readdir(dp);
            while(ep) {
                m_entryCount++;
                ep = readdir(dp);
            }
            rewinddir(dp);
            m_entries = new directory_entry[m_entryCount];
            int i = 0;
            ep = readdir(dp);
            while(ep) {
                mstring n = mstring(ep->d_name, strlen(ep->d_name));
                if(n.size() != 0 && n != ".DS_Store") {
                    m_entries[i].Name = n;
                    switch(ep->d_type) {
                        case DT_REG: { m_entries[i].Type = DET_FILE;    break; }
                        case DT_DIR: { m_entries[i].Type = DET_FOLDER;  break; }
                        case DT_LNK: { m_entries[i].Type = DET_LINK;    break; }
                        default:     { m_entries[i].Type = DET_INVALID; break; }
                    }
                    if(ep->d_type == DT_REG) {
                        size_t extbegin = m_entries[i].Name.rfind('.');
                        if(extbegin != m_entries[i].Name.npos) m_entries[i].Extension = m_entries[i].Name.substr(extbegin + 1);
                        else m_entries[i].Extension = "";
                    }
                    i++;
                }
                ep = readdir(dp);
            }
            closedir(dp);

			m_path = dir;
        }
    }


	data_container::data_container(FILE* fp, const mstring& name, DATA_MODE mode)
		: m_name(name), m_mode(mode), m_handle(fp), m_size(0), m_offset(0), m_inState(true) {
		m_allocatorId = memory_man::current()->id();
        if (fp) {
            fseek(fp, 0, SEEK_END);
            m_size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
        }
	}
    data_container::~data_container() {
    }

    bool data_container::read_data(void* data,u32 size) {
        if(m_handle) {
            if(fread(data,size,1,m_handle) != 1) {
                r2Error("Failed to read %u bytes from data container (%s): Call to fread failed",size,m_name.c_str());
                return false;
            } else {
                m_offset += size;
                return true;
            }
        }

        if(at_end(size)) {
            r2Error("Failed to read %u bytes from data container (%s): End of data encountered",size,m_name.c_str());
            return false;
        }
        for(u32 i = 0;i < size;i++) {
            ((u8*)data)[i] = m_data[i + m_offset];
        }
        m_offset += size;
        return true;
    }

    bool data_container::write_data(const void* data,u32 size) {
        if(m_handle) {
            if(fwrite(data,size,1,m_handle) != 1) {
                r2Error("Failed to write %u bytes to data container (%s): Call to fwrite failed",size,m_name.c_str());
                return false;
            } else {
                m_offset += size;
                m_size += size;
                return true;
            }
        }

        for(u64 i = 0;i < size;i++) {
			memory_man::push_current(m_allocatorId);
            m_data.insert(m_data.begin() + m_offset,((u8*)data)[i]);
			memory_man::pop_current();
            m_offset++;
            m_size++;
        }
        return true;
    }

    bool data_container::read_ubyte(u8& data) { return read(data); }

    bool data_container::read_byte (s8& data) { return read(data); }

    bool data_container::read_uint16(u16& data) {
        if(m_mode == DM_BINARY) return read(data);
        else {
            ParseInteger(u16,0,UINT16_MAX,%d,%d);
        }
        return true;
    }

    bool data_container::read_uint32(u32& data) {
        if(m_mode == DM_BINARY) return read(data);
        else {
            ParseInteger(u32,0,UINT32_MAX,%d,%d);
        }
        return true;
    }

    bool data_container::read_uint64(u64& data) {
        if(m_mode == DM_BINARY) return read(data);
        else {
            ParseInteger(u64,0,UINT64_MAX,%d,%llu);
        }
        return true;
    }

    bool data_container::read_int16(i16& data) {
        if(m_mode == DM_BINARY) return read(data);
        else {
            ParseInteger(i16,INT16_MIN,INT16_MAX,%d,%d);
        }
        return true;
    }

    bool data_container::read_int32(i32& data) {
        if(m_mode == DM_BINARY) return read(data);
        else {
            ParseInteger(i32,INT32_MIN,INT32_MAX,%d,%d);
        }
        return true;
    }

    bool data_container::read_int64(i64& data) {
        if(m_mode == DM_BINARY) return read(data);
        else {
            ParseInteger(i64,INT64_MIN,INT64_MAX,%lli,%lli);
        }
        return true;
    }

    bool data_container::read_float32(f32& data) {
        if(m_mode == DM_BINARY) return read(data);
        else {
            char c = 0;
            if(!read(c)) {
				r2Error("Failed to parse float32 from data container (%s)", m_name.c_str());
				return false;
			}

            //Skip whitespace
            while(IsWhitespace(c)) {
				if(!read(c)) {
					r2Error("Failed to parse float32 from data container (%s)", m_name.c_str());
					return false;
				}
			}

            //Skip '-' if negative
            bool IsNeg = c == '-';
            if(IsNeg) {
                if(!read(c)) {
					r2Error("Failed to parse float32 from data container (%s)", m_name.c_str());
					return false;
				}
            }

            //Parse value
            bool DecimalEncountered = false;
            mstring s;
            while(IsNumber(c) || (c == '.' && !DecimalEncountered)) {
                if(c == '.') DecimalEncountered = true;
                s += c;
                if(!read(c)) {
					r2Error("Failed to parse float32 from data container (%s)", m_name.c_str());
					return false;
				}
            }

            f64 f = atof(s.c_str());
            if(f > FLT_MAX) {
                r2Warn("Parsing real number from data container (%s) that will not fit in data type (f32)", m_name.c_str());
                r2Log("Info: Parsed value: %f | Min/Max value for data type: %f,%f", f, FLT_MIN, FLT_MAX);
            }
            u32 Digits = s.length();
            if(DecimalEncountered) Digits--;
            if(Digits > 7) {
                r2Warn("Parsing real number from data container (%s) into data type (f32) that may lose precision (more than 7 digits)", m_name.c_str());
                r2Log("Info: Actual value: %s | float value: %f", s.c_str(), (f32)f);
            }

            if(IsNeg) data = -f;
            else data = f;
        }
        return true;
    }

    bool data_container::read_float64(f64& data) {
        if(m_mode == DM_BINARY) return read(data);
        else {
            char c = 0;
            if(!read(c)) {
			    r2Error("Failed to parse float64 from data container (%s)", m_name.c_str());
				return false;
			}

            //Skip whitespace
            while(IsWhitespace(c)) {
				if(!read(c)) {
					r2Error("Failed to parse float64 from data container (%s)", m_name.c_str());
					return false;
				}
			}

            //Skip '-' if negative
            bool IsNeg = c == '-';
            if(IsNeg) {
                if(!read(c)) {
					r2Error("Failed to parse float64 from data container (%s)", m_name.c_str());
					return false;
				}
            }

            //Parse value
            bool DecimalEncountered = false;
            mstring s;
            while(IsNumber(c) || (c == '.' && !DecimalEncountered)) {
                if(c == '.') DecimalEncountered = true;
                s += c;
                if(!read(c)) {
					r2Error("Failed to parse float64 from data container (%s)", m_name.c_str());
					return false;
				}
            }

            f64 f = atof(s.c_str());
            if(f > DBL_MAX) {
                r2Warn("Parsing real number from data container (%s) that will not fit in data type (f64)", m_name.c_str());
                r2Log("Info: Parsed value: %f | Min/Max value for data type: %f,%f", f, DBL_MIN, DBL_MAX);
            }
            u32 Digits = s.length();
            if(DecimalEncountered) Digits--;
            if(Digits > 7) {
                r2Warn("Parsing real number from data container (%s) into data type (f64) that may lose precision (more than 16 digits)", m_name.c_str());
                r2Log("Info: Actual value: %s | float64 value: %f", s.c_str(), (f64)f);
            }

            if(IsNeg) data = -f;
            else data = f;
        }
        return true;
    }

	bool data_container::read_string(mstring& data, u32 length) {
		if (length != 0) {
			data.resize(length);
			return read_data(&data[0], length);
		} else {
			char c = 0;
			if (!read_data(&c, 1)) return false;
			while (c != 0) {
				data += c;
				if (!read_data(&c, 1)) return false;
			}
			return true;
		}
		return false;
	}

	bool data_container::read_line(mstring& data) {
		if (m_mode != DM_TEXT) {
			r2Error("read_line can only be called in text mode");
			return false;
		}
		
		char c = 0;
		if(!read(c)) {
			r2Error("Failed to read line from container \"%s\"", m_name.c_str());
			return false;
		}
		while(c != '\n' && c != '\r' && !at_end(1)) {
			data += c;
			if(!read(c)) {
				r2Error("Failed to read line from container \"%s\"", m_name.c_str());
				return false;
			}
		}
		return true;
	}

    bool data_container::write_ubyte(const u8& data) { return write(data); }

    bool data_container::write_byte (const s8& data) { return write(data); }

    bool data_container::write_uint16(const u16& data) {
        if(m_mode == DM_BINARY) return write(data);
        else {
            writeInteger(u16, 32, %d);
        }
        return true;
    }

    bool data_container::write_uint32(const u32& data) {
        if(m_mode == DM_BINARY) return write(data);
        else {
            writeInteger(u32, 32, %d);
        }
        return true;
    }

    bool data_container::write_uint64(const u64& data) {
        if(m_mode == DM_BINARY) return write(data);
        else {
            writeInteger(u64, 32, %llu);
        }
        return true;
    }

    bool data_container::write_int16(const i16& data) {
        if(m_mode == DM_BINARY) return write(data);
        else {
            writeInteger(i16, 32, %d);
        }
        return true;
    }

    bool data_container::write_int32(const i32& data) {
        if(m_mode == DM_BINARY) return write(data);
        else {
            writeInteger(i32, 32, %d)
        }
        return true;
    }

    bool data_container::write_int64(const i64& data) {
        if(m_mode == DM_BINARY) return write(data);
        else {
            writeInteger(i64, 32, %lli)
        }
        return true;
    }

    bool data_container::write_float32(const f32& data) {
        if(m_mode == DM_BINARY) return write(data);
        else {
            char Out[32];
            i32 r = _snprintf(Out,32,"%f",data);
            if(r < 0) {
                r2Error("Failed to convert real number (f32) to mstring for data container (%s): call to snprintf returned %d",m_name.c_str(),r);
                return false;
            }
            else return write_data(Out,r);
        }
        return true;
    }

    bool data_container::write_float64(const f64& data) {
        if(m_mode == DM_BINARY) return write(data);
        else {
            char Out[32];
            i32 r = _snprintf(Out,32,"%f",data);
            if(r < 0) {
                r2Error("Failed to convert real number (f64) to mstring for data container (%s): call to snprintf returned %d",m_name.c_str(),r);
                return false;
            }
            else return write_data(Out,r);
        }
        return true;
    }

	bool data_container::write_string(const mstring& data) {
		return write_data(&data[0], data.length());
	}

    void data_container::seek(i32 Offset) {
        if(m_offset + Offset > m_size) {
            r2Error("Call to seek(%u) with data container (%s) failed: End of data would be exceeded", Offset, m_name.c_str());
            return;
        }
        if(((i32)m_offset + (i32)Offset) < 0) {
            r2Error("Call to seek(%u) with data container (%s) failed: Resulting data position less than 0", Offset, m_name.c_str());
            return;
        }

        if(m_handle) {
            i32 r = fseek(m_handle, Offset, SEEK_CUR);
            if(r != 0) {
                r2Error("Call to seek(%d) with data container (%s) failed: Call to fseek returned (%d)", Offset, m_name.c_str(),r);
            }
            else m_offset += Offset;
        } else m_offset += Offset;
    }

    void data_container::set_position(u32 Offset) {
        if(Offset > m_size) {
            r2Error("Call to set_position(%u) with data container (%s) failed: New position exceeds end of data", Offset, m_name.c_str());
            return;
        }

        if(m_handle) {
            i32 r = fseek(m_handle, Offset, SEEK_SET);
            if(r != 0) {
                r2Error("Call to seek(%d) with data container (%s) failed: Call to fseek returned (%d)", Offset, m_name.c_str(), r);
            }
            else m_offset = Offset;
        }
        else m_offset = Offset;
    }

	data_container* data_container::sub(size_t length) {
		if (m_offset + length > m_size) {
			r2Error("Can't open container of length %llu from '%s', there are only %llu bytes left in this container relative to its current offset", length, m_name.c_str(), m_size - m_offset);
			return nullptr;
		}

		data_container* result = r2engine::files()->create(m_mode, m_name + "_sub");
		if (!result) return nullptr;
		if (m_handle) {
			u8* data = new u8[length];
			if (!read_data(data, length)) {
				delete[] data;
				r2engine::files()->destroy(result);
				return nullptr;
			}
			result->write_data(data, length);
		} else {
			result->write_data(data(), length);
			m_offset += length;
		}
		result->set_position(0);
		return result;
	}

	void data_container::clear() {
		if (m_data.size() > 0) m_data.clear();
		m_offset = 0;
		m_size = 0;
	}


	void file_man::initialize() {
		m_stateData = r2engine::get()->states()->register_state_data_factory<state_containers>(new state_container_factory());
	}

	data_container* file_man::create(DATA_MODE Mode, const mstring& Name) {
		m_stateData.enable();
        data_container* c = new data_container(0, Name, Mode);
        if(Name.length() == 0) {
            char addr[32];
            memset(addr, 0, 32);
            _snprintf(addr, 32, "0x%lX", (intptr_t)c);
            c->m_name = addr;
        }
        c->m_iterator = m_stateData->containers.insert(m_stateData->containers.end(), c);
		m_stateData.disable();
        return c;
    }
    
	data_container* file_man::open(const mstring& File, DATA_MODE Mode, const mstring& Name)  {
        FILE* fp = fopen(File.c_str(),"rb+");
        if(!fp) {
            r2Error("Failed to open file (%s)",File.c_str());
            return 0;
        }

		m_stateData.enable();

        data_container* c = new data_container(fp, Name.length() == 0 ? File : Name, Mode);
        fseek(fp, 0, SEEK_END);
        c->m_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        c->m_iterator = m_stateData->containers.insert(m_stateData->containers.end(), c);
		m_stateData.disable();
        return c;
    }
    
	data_container* file_man::load(const mstring& File, DATA_MODE Mode, const mstring& Name) {
        FILE* fp = fopen(File.c_str(),"rb+");
        if(!fp) {
            r2Error("Failed to open file (%s)", File.c_str());
            return 0;
        }

        fseek(fp,0,SEEK_END);
        u32 Sz = ftell(fp);
        fseek(fp,0,SEEK_SET);

		data_container* c = nullptr;

		m_stateData.enable();
        if(Sz == 0) {
            fclose(fp);
            r2Warn("Loading empty file (%s) into data container (%s)", File.c_str(), Name.length() == 0 ? File.c_str() : Name.c_str());

            c = new data_container(0, Name.length() == 0 ? File : Name, Mode);
            c->m_iterator = m_stateData->containers.insert(m_stateData->containers.end(), c);
        } else {
            u8* data = new u8[Sz];
            if(fread(data, Sz, 1, fp) != 1)
            {
                r2Error("Failed to load %u bytes from file (%s) into memory", Sz, File.c_str());
                fclose(fp);
                return 0;
            }

            c = new data_container(0, Name.length() == 0 ? File : Name, Mode);
            c->m_size = Sz;
            for(u32 i = 0;i < c->m_size;i++) c->m_data.push_back(data[i]);
            delete [] data;
            fclose(fp);

            c->m_iterator = m_stateData->containers.insert(m_stateData->containers.end(), c);
        }

		m_stateData.disable();
		return c;
    }
   
    bool file_man::save(data_container* data, const mstring& File) {
        FILE* fp = fopen(File.c_str(), "wb");
        if(!fp) {
            r2Error("Failed to open file (%s)",File.c_str());
            return false;
        }

        if(fwrite(&data->m_data[0], data->m_size, 1, fp) != 1) {
            r2Error("Failed to write %u bytes from data container (%s) to file (%s)", data->m_size, data->m_name.c_str(), File.c_str());
            fclose(fp);
            return false;
        }

        fclose(fp);
        return true;
    }

    mstring file_man::working_directory() const {
        char cwd[256];
        getcwd(cwd, 256);
        return cwd;
    }

    void file_man::set_working_directory(const mstring& dir) {
        if(!exists(dir)) {
            r2Error("Unable to set current working directory (%s): Directory does not exist",dir.c_str());
            return;
        }

		chdir(dir.c_str());
    }

    directory_info* file_man::parse_directory(const mstring& dir) {
        if(!exists(dir)) {
            r2Error("Unable to parse directory (%s): Directory does not exist",dir.c_str());
            return 0;
        }

		m_stateData.enable();
        directory_info* dinfo = new directory_info();
		dinfo->populate(dir);
		m_stateData->directories.push_back(dinfo);
		m_stateData.disable();
        return dinfo;
    }

	void file_man::destroy_directory(directory_info* dir) {
		m_stateData.enable();
		bool found = false;
		for(auto i = m_stateData->directories.begin();i != m_stateData->directories.end();i++) {
			if (*i == dir) {
				delete dir;
				m_stateData->directories.erase(i);
				found = true;
				break;
			}
		}
		if (!found) {
			r2Error("Could not release directory info, it doesn't exist in this state");
		}
		m_stateData.disable();
	}

    bool file_man::exists(const mstring& Item) {
        struct stat buf;
        return (stat(Item.c_str(),&buf) == 0);
    }

	void file_man::destroy(data_container *container) {
		if (!container->m_inState) return;
		container->m_inState = false;
		m_stateData.enable();
		m_stateData->containers.erase(container->m_iterator);
		if(container->m_handle) fclose(container->m_handle);
		container->m_handle = nullptr;
		delete container;
		m_stateData.disable();
	}

	void file_man::destroy_nodelete(data_container *container) {
		if (!container->m_inState) return;
		container->m_inState = false;
		m_stateData.enable();
		m_stateData->containers.erase(container->m_iterator);
		if(container->m_handle) fclose(container->m_handle);
		container->m_handle = nullptr;
		m_stateData.disable();
	}
}
