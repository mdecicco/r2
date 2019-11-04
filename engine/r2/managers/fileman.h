#pragma once
#include <r2/config.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <list>
using namespace std;

namespace r2
{
    enum DIR_ENTRY_TYPE
    {
        DET_FILE,
        DET_FOLDER,
        DET_LINK,
        DET_INVALID,
    };

    class directory_entry
    {
        public:
            directory_entry() : Type(DET_INVALID) { }
            directory_entry(const directory_entry& o);
            ~directory_entry() { }

            DIR_ENTRY_TYPE Type;
            string Name;
            string Extension;
    };

    class directory_info
    {
        public:
            directory_info() : m_entries(0), m_entryCount(0) { }
            ~directory_info() { if(m_entries) delete [] m_entries; }

            void populate(const string& d);
            int entry_count() const { return m_entryCount; }
            directory_entry entry(i32 idx) const { return m_entries[idx]; }

            /* std::vector refuses to work in this case... */
            directory_entry* m_entries;
            i32 m_entryCount;
    };

    enum DATA_MODE
    {
        DM_BINARY,
        DM_TEXT,
    };

    /*
     * NOTE:
     * data written with this class in non-streaming mode will not be
     * written to a file until Save is called.
     */
    class file_man;
    class data_container
    {
        public:
            bool read_data(void* data,u32 size);
            bool write_data(const void* data,u32 size);
            bool write_string(const string& data) { return write_data(&data[0],data.length()); }

            template <typename T> bool read(T& data) { return read_data((void*)&data,sizeof(T)); }
            template <typename T> bool write(const T& data) { return write_data((const void*)&data,sizeof(T)); }

            /*
             * The following functions functions automatically parse
             * text if the file is not opened in binary mode
             */
            bool read_ubyte(u8& data);
            bool read_byte(s8& data);
            bool read_uint16(u16& data);
            bool read_uint32(u32& data);
            bool read_uint64(u64& data);
            bool read_int16(i16& data);
            bool read_int32(i32& data);
            bool read_int64(i64& data);
            bool read_float32(f32& data);
            bool read_float64(f64& data);

            /*
             * The following functions functions automatically
             * convert values to text if the file is not opened
             * in binary mode.
             */
            bool write_ubyte(const u8& data);
            bool write_byte(const s8& data);
            bool write_uint16(const u16& data);
            bool write_uint32(const u32& data);
            bool write_uint64(const u64& data);
            bool write_int16(const i16& data);
            bool write_int32(const i32& data);
            bool write_int64(const i64& data);
            bool write_float32(const f32& data);
            bool write_float64(const f64& data);

            void seek(i32 off); /* adds to m_offset */
            void set_position(u32 pos);
            u32 position() const { return m_offset; }
            bool at_end(u32 end_off = 0) const { return m_offset > m_size - end_off; }
            void* data() { return &m_data[m_offset]; }
			void clear();
            u32 size() const { return m_size; }

            string name() const { return m_name; }

        protected:
            friend class file_man;
            data_container(file_man* fs,FILE* fp,const string& name,DATA_MODE mode) :
                m_mgr(fs), m_name(name), m_mode(mode), m_handle(fp), m_size(0), m_offset(0) { }
            ~data_container();

            typedef list<data_container*>::iterator id;

            file_man* m_mgr;
            string m_name;
            DATA_MODE m_mode;
            id m_iterator;

            /* For streamed data */
            FILE* m_handle;
            u32 m_size;

            /* For in-memory data */
            vector<u8> m_data;
            u32 m_offset;
    };

    class r2engine;
    class file_man
    {
        public:
            file_man(r2engine* eng) { m_eng = eng; }
            ~file_man();

            r2engine* engine() const { return m_eng; }

            /* container names default to filenames when not set (or pointer address when create() is used) */
            data_container* create(DATA_MODE mode,const string& name = "");
            data_container* open(const string& file,DATA_MODE mode,const string& name = "");
            data_container* load(const string& file,DATA_MODE mode,const string& name = "");
            bool save(data_container* data,const string& file);
            void destroy(data_container* container);

            string working_directory() const;
            void set_working_directory(const string& dir);
            directory_info* parse_directory(const string& dir);
            bool exists(const string& entry);

            void shutdown();

        protected:
            friend class data_container;
            r2engine* m_eng;
            list<data_container*> m_containers;

    };
};
