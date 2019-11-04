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
    void directory_info::populate(const string& dir)
    {
        if(m_entries) { delete [] m_entries; m_entries = 0; }
        m_entryCount = 0;

        DIR *dp = opendir(dir.c_str());
        if(dp != NULL)
        {
            struct dirent *ep = readdir(dp);
            while(ep)
            {
                m_entryCount++;
                ep = readdir(dp);
            }
            rewinddir(dp);
            m_entries = new directory_entry[m_entryCount];
            int i = 0;
            ep = readdir(dp);
            while(ep)
            {
                string n = string(ep->d_name,strlen(ep->d_name));
                if(n.size() != 0 && n != ".DS_Store")
                {
                    m_entries[i].Name = n;
                    switch(ep->d_type)
                    {
                        case DT_REG: { m_entries[i].Type = DET_FILE;    break; }
                        case DT_DIR: { m_entries[i].Type = DET_FOLDER;  break; }
                        case DT_LNK: { m_entries[i].Type = DET_LINK;    break; }
                        default:     { m_entries[i].Type = DET_INVALID; break; }
                    }
                    if(ep->d_type == DT_REG)
                    {
                        size_t extbegin = m_entries[i].Name.rfind('.');
                        if(extbegin != m_entries[i].Name.npos) m_entries[i].Extension = m_entries[i].Name.substr(extbegin + 1);
                        else m_entries[i].Extension = "";
                    }
                    i++;
                }
                ep = readdir(dp);
            }
            closedir(dp);
        }
    }

    data_container::~data_container() {
    }

    bool data_container::read_data(void* data,u32 size)
    {
        if(m_handle)
        {
            if(fread(data,size,1,m_handle) != 1)
            {
                r2Error("Failed to read %u bytes from data container (%s): Call to fread failed",size,m_name.c_str());
                return false;
            }
            else
            {
                m_offset += size;
                return true;
            }
        }

        if(at_end(size))
        {
            r2Error("Failed to read %u bytes from data container (%s): End of data encountered",size,m_name.c_str());
            return false;
        }
        for(u32 i = m_offset;i < size;i++) {
            ((u8*)data)[i] = m_data[i];
        }
        m_offset += size;
        return true;
    }
    bool data_container::write_data(const void* data,u32 size)
    {
        if(m_handle)
        {
            if(fwrite(data,size,1,m_handle) != 1)
            {
                r2Error("Failed to write %u bytes to data container (%s): Call to fwrite failed",size,m_name.c_str());
                return false;
            }
            else
            {
                m_offset += size;
                m_size += size;
                return true;
            }
        }

        for(u64 i = 0;i < size;i++)
        {
            m_data.insert(m_data.begin() + m_offset,((u8*)data)[i]);
            m_offset++;
            m_size++;
        }
        return true;
    }

    bool data_container::read_ubyte(u8& data) { return read(data); }
    bool data_container::read_byte (s8& data) { return read(data); }

    bool IsWhitespace(char c)
    {
        return c == '\t' || c == ' ';
    }
    bool IsNumber(char c)
    {
        return isdigit(c);
    }

    #define ParseInteger(Type,Min,Max,MinFmt,MaxFmt) \
    char c = 0; \
    if(!read(c)) { r2Error("Failed to parse integer from data container (%s)",m_name.c_str()); return false; } \
    while(IsWhitespace(c)) { if(!read(c)) { r2Error("Failed to parse integer from data container (%s)",m_name.c_str()); return false; } } \
    bool IsNeg = c == '-'; \
    if(IsNeg) \
    { \
        if(!read(c)) { r2Error("Failed to parse integer from data container (%s)",m_name.c_str()); return false; } \
    } \
    string s; \
    while(IsNumber(c)) \
    { \
        s += c; \
        if(!read(c)) { r2Error("Failed to parse integer from data container (%s)",m_name.c_str()); return false; } \
    } \
    long long i = atoll(s.c_str()); \
    if(i > Max) \
    { \
        r2Warn("Parsing integer from data container (%s) that will not fit in data type ("#Type")",m_name.c_str()); \
        r2Log("Info: Parsed value: %lli | Min/Max value for data type: "#MinFmt"/"#MaxFmt,i,Min,Max); \
    } \
    if(IsNeg) data = -i; \
    else data = i;

    bool data_container::read_uint16(u16& data)
    {
        if(m_mode == DM_BINARY) return read(data);
        else
        {
            ParseInteger(u16,0,UINT16_MAX,%d,%d);
        }
        return true;
    }
    bool data_container::read_uint32(u32& data)
    {
        if(m_mode == DM_BINARY) return read(data);
        else
        {
            ParseInteger(u32,0,UINT32_MAX,%d,%d);
        }
        return true;
    }
    bool data_container::read_uint64(u64& data)
    {
        if(m_mode == DM_BINARY) return read(data);
        else
        {
            ParseInteger(u64,0,UINT64_MAX,%d,%llu);
        }
        return true;
    }
    bool data_container::read_int16(i16& data)
    {
        if(m_mode == DM_BINARY) return read(data);
        else
        {
            ParseInteger(i16,INT16_MIN,INT16_MAX,%d,%d);
        }
        return true;
    }
    bool data_container::read_int32(i32& data)
    {
        if(m_mode == DM_BINARY) return read(data);
        else
        {
            ParseInteger(i32,INT32_MIN,INT32_MAX,%d,%d);
        }
        return true;
    }
    bool data_container::read_int64(i64& data)
    {
        if(m_mode == DM_BINARY) return read(data);
        else
        {
            ParseInteger(i64,INT64_MIN,INT64_MAX,%lli,%lli);
        }
        return true;
    }
    bool data_container::read_float32(f32& data)
    {
        if(m_mode == DM_BINARY) return read(data);
        else
        {
            char c = 0;
            if(!read(c)) { r2Error("Failed to parse float32 from data container (%s)",m_name.c_str()); return false; }

            //Skip whitespace
            while(IsWhitespace(c)) { if(!read(c)) { r2Error("Failed to parse float32 from data container (%s)",m_name.c_str()); return false; } }

            //Skip '-' if negative
            bool IsNeg = c == '-';
            if(IsNeg)
            {
                if(!read(c)) { r2Error("Failed to parse float32 from data container (%s)",m_name.c_str()); return false; }
            }

            //Parse value
            bool DecimalEncountered = false;
            string s;
            while(IsNumber(c) || (c == '.' && !DecimalEncountered))
            {
                if(c == '.') DecimalEncountered = true;
                s += c;
                if(!read(c)) { r2Error("Failed to parse float32 from data container (%s)",m_name.c_str()); return false; }
            }

            f64 f = atof(s.c_str());
            if(f > FLT_MAX)
            {
                r2Warn("Parsing real number from data container (%s) that will not fit in data type (f32)",m_name.c_str());
                r2Log("Info: Parsed value: %f | Min/Max value for data type: %f,%f",f,FLT_MIN,FLT_MAX);
            }
            u32 Digits = s.length();
            if(DecimalEncountered) Digits--;
            if(Digits > 7)
            {
                r2Warn("Parsing real number from data container (%s) into data type (f32) that may lose precision (more than 7 digits)",m_name.c_str());
                r2Log("Info: Actual value: %s | float value: %f",s.c_str(),(f32)f);
            }

            if(IsNeg) data = -f;
            else data = f;
        }
        return true;
    }
    bool data_container::read_float64(f64& data)
    {
        if(m_mode == DM_BINARY) return read(data);
        else
        {
            char c = 0;
            if(!read(c)) { r2Error("Failed to parse float64 from data container (%s)",m_name.c_str()); return false; }

            //Skip whitespace
            while(IsWhitespace(c)) { if(!read(c)) { r2Error("Failed to parse float64 from data container (%s)",m_name.c_str()); return false; } }

            //Skip '-' if negative
            bool IsNeg = c == '-';
            if(IsNeg)
            {
                if(!read(c)) { r2Error("Failed to parse float64 from data container (%s)",m_name.c_str()); return false; }
            }

            //Parse value
            bool DecimalEncountered = false;
            string s;
            while(IsNumber(c) || (c == '.' && !DecimalEncountered))
            {
                if(c == '.') DecimalEncountered = true;
                s += c;
                if(!read(c)) { r2Error("Failed to parse float64 from data container (%s)",m_name.c_str()); return false; }
            }

            f64 f = atof(s.c_str());
            if(f > DBL_MAX)
            {
                r2Warn("Parsing real number from data container (%s) that will not fit in data type (f64)",m_name.c_str());
                r2Log("Info: Parsed value: %f | Min/Max value for data type: %f,%f",f,DBL_MIN,DBL_MAX);
            }
            u32 Digits = s.length();
            if(DecimalEncountered) Digits--;
            if(Digits > 7)
            {
                r2Warn("Parsing real number from data container (%s) into data type (f64) that may lose precision (more than 16 digits)",m_name.c_str());
                r2Log("Info: Actual value: %s | float64 value: %f",s.c_str(),(f64)f);
            }

            if(IsNeg) data = -f;
            else data = f;
        }
        return true;
    }

    bool data_container::write_ubyte(const u8& data) { return write(data); }
    bool data_container::write_byte (const s8& data) { return write(data); }

	i32 _snprintf(char* Out,size_t MaxLen,const char* fmt,...)
	{
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

    #define writeInteger(Type,MaxStrLen,fmt) \
    char Out[MaxStrLen]; \
    i32 r = _snprintf(Out,MaxStrLen,#fmt,data); \
    if(r < 0) \
    { \
        r2Error("Failed to convert integer (" #Type ") to string for data container (%s): call to snprintf returned %d",m_name.c_str(),r); \
        return false; \
    } \
    else return write_data(Out,r);

    bool data_container::write_uint16(const u16& data)
    {
        if(m_mode == DM_BINARY) return write(data);
        else
        {
            writeInteger(u16,32,%d);
        }
        return true;
    }
    bool data_container::write_uint32(const u32& data)
    {
        if(m_mode == DM_BINARY) return write(data);
        else
        {
            writeInteger(u32,32,%d);
        }
        return true;
    }
    bool data_container::write_uint64(const u64& data)
    {
        if(m_mode == DM_BINARY) return write(data);
        else
        {
            writeInteger(u64,32,%llu);
        }
        return true;
    }
    bool data_container::write_int16(const i16& data)
    {
        if(m_mode == DM_BINARY) return write(data);
        else
        {
            writeInteger(i16,32,%d);
        }
        return true;
    }
    bool data_container::write_int32(const i32& data)
    {
        if(m_mode == DM_BINARY) return write(data);
        else
        {
            writeInteger(i32,32,%d)
        }
        return true;
    }
    bool data_container::write_int64(const i64& data)
    {
        if(m_mode == DM_BINARY) return write(data);
        else
        {
            writeInteger(i64,32,%lli)
        }
        return true;
    }
    bool data_container::write_float32(const f32& data)
    {
        if(m_mode == DM_BINARY) return write(data);
        else
        {
            char Out[32];
            i32 r = _snprintf(Out,32,"%f",data);
            if(r < 0)
            {
                r2Error("Failed to convert real number (f32) to string for data container (%s): call to snprintf returned %d",m_name.c_str(),r);
                return false;
            }
            else return write_data(Out,r);
        }
        return true;
    }
    bool data_container::write_float64(const f64& data)
    {
        if(m_mode == DM_BINARY) return write(data);
        else
        {
            char Out[32];
            i32 r = _snprintf(Out,32,"%f",data);
            if(r < 0)
            {
                r2Error("Failed to convert real number (f64) to string for data container (%s): call to snprintf returned %d",m_name.c_str(),r);
                return false;
            }
            else return write_data(Out,r);
        }
        return true;
    }

    void data_container::seek(i32 Offset)
    {
        if(m_offset + Offset > m_size)
        {
            r2Error("Call to seek(%u) with data container (%s) failed: End of data would be exceeded",Offset,m_name.c_str());
            return;
        }
        if(((i32)m_offset + (i32)Offset) < 0)
        {
            r2Error("Call to seek(%u) with data container (%s) failed: Resulting data position less than 0",Offset,m_name.c_str());
            return;
        }

        if(m_handle)
        {
            i32 r = fseek(m_handle,Offset,SEEK_CUR);
            if(r != 0)
            {
                r2Error("Call to seek(%d) with data container (%s) failed: Call to fseek returned (%d)",Offset,m_name.c_str(),r);
            }
            else m_offset += Offset;
        }
    }
    void data_container::set_position(u32 Offset)
    {
        if(Offset > m_size)
        {
            r2Error("Call to set_position(%u) with data container (%s) failed: New position exceeds end of data",Offset,m_name.c_str());
            return;
        }

        if(m_handle)
        {
            i32 r = fseek(m_handle,Offset,SEEK_SET);
            if(r != 0)
            {
                r2Error("Call to seek(%d) with data container (%s) failed: Call to fseek returned (%d)",Offset,m_name.c_str(),r);
            }
            else m_offset = Offset;
        }
        else m_offset = Offset;
    }
	void data_container::clear() {
		if (m_data.size() > 0) m_data.clear();
		m_offset = 0;
		m_size = 0;
	}
    file_man::~file_man() {
        if(m_containers.size() != 0) {
            r2Warn("File manager has %lu unreleased data containers at shutdown time",m_containers.size());

            for(auto c = m_containers.begin();c != m_containers.end();c++) {
				r2Warn("Destroying unreleased container: %s",(*c)->m_name.c_str());
                delete *c;
            }
        }
    }
    data_container* file_man::create(DATA_MODE Mode,const string& Name)
    {
        data_container* c = new data_container(this,0,Name,Mode);
        if(Name.length() == 0)
        {
            char addr[32];
            memset(addr,0,32);
            _snprintf(addr,32,"0x%lX",(Ptr)c);
            c->m_name = addr;
        }
        c->m_iterator = m_containers.insert(m_containers.end(),c);
        return c;
    }
    data_container* file_man::open(const string& File,DATA_MODE Mode,const string& Name)
    {
        #ifndef __WIN32
        FILE* fp = fopen(File.c_str(),"rb+");
        #else
        #endif
        if(!fp)
        {
            r2Error("Failed to open file (%s)",File.c_str());
            return 0;
        }

        data_container* c = new data_container(this,fp,Name.length() == 0 ? File : Name,Mode);
        fseek(fp,0,SEEK_END);
        c->m_size = ftell(fp);
        fseek(fp,0,SEEK_SET);

        c->m_iterator = m_containers.insert(m_containers.end(),c);
        return c;
    }
    data_container* file_man::load(const string& File,DATA_MODE Mode,const string& Name)
    {
        #ifndef __WIN32
        FILE* fp = fopen(File.c_str(),"rb+");
        #else
        #endif
        if(!fp)
        {
            r2Error("Failed to open file (%s)",File.c_str());
            return 0;
        }

        fseek(fp,0,SEEK_END);
        u32 Sz = ftell(fp);
        fseek(fp,0,SEEK_SET);

        if(Sz == 0)
        {
            fclose(fp);
            r2Warn("Loading empty file (%s) into data container (%s)",File.c_str(),Name.length() == 0 ? File.c_str() : Name.c_str());

            data_container* c = new data_container(this,0,Name.length() == 0 ? File : Name,Mode);
            c->m_iterator = m_containers.insert(m_containers.end(),c);
            return c;
        }
        else
        {
            u8* data = new u8[Sz];
            if(fread(data,Sz,1,fp) != 1)
            {
                r2Error("Failed to load %u bytes from file (%s) into memory",Sz,File.c_str());
                fclose(fp);
                return 0;
            }

            data_container* c = new data_container(this,0,Name.length() == 0 ? File : Name,Mode);
            c->m_size = Sz;
            for(u32 i = 0;i < c->m_size;i++) c->m_data.push_back(data[i]);
            delete [] data;
            fclose(fp);

            c->m_iterator = m_containers.insert(m_containers.end(),c);
            return c;
        }
    }
    bool file_man::save(data_container* data,const string& File)
    {
        #ifndef __WIN32
        FILE* fp = fopen(File.c_str(),"wb");
        #else
        #endif
        if(!fp)
        {
            r2Error("Failed to open file (%s)",File.c_str());
            return false;
        }

        if(fwrite(&data->m_data[0],data->m_size,1,fp) != 1)
        {
            r2Error("Failed to write %u bytes from data container (%s) to file (%s)",data->m_size,data->m_name.c_str(),File.c_str());
            fclose(fp);
            return false;
        }

        fclose(fp);
        return true;
    }

#ifdef _WIN32
#define getcwd _getcwd
#define chdir _chdir
#endif

    string file_man::working_directory() const {
        char cwd[256];
        getcwd(cwd,256);
        return cwd;
    }
    void file_man::set_working_directory(const string& dir)
    {
        if(!exists(dir)) {
            r2Error("Unable to set current working directory (%s): Directory does not exist",dir.c_str());
            return;
        }

		chdir(dir.c_str());
    }
    directory_info* file_man::parse_directory(const string& dir) {
        if(!exists(dir)) {
            r2Error("Unable to parse directory (%s): Directory does not exist",dir.c_str());
            return 0;
        }

        directory_info* Info = new directory_info();
        Info->populate(dir);
        return Info;
    }
    bool file_man::exists(const string& Item)
    {
        struct stat buf;
        return (stat(Item.c_str(),&buf) == 0);
    }


    void file_man::destroy(data_container *container) {
        if(container->m_mgr == 0) return;
        container->m_mgr = 0;
        m_containers.erase(container->m_iterator);
        if(container->m_handle) fclose(container->m_handle);
        container->m_handle = 0;
        delete container;
    }
}
