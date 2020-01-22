#pragma once
#include <r2/managers/memman.h>
#include <r2/managers/engine_state.h>

namespace v8pp {
	struct raw_ptr_traits;
};

namespace r2 {
	class data_container;
	class directory_info;
	class state_container_factory : public engine_state_data_factory {
		public:
			state_container_factory() { }
			~state_container_factory() { }

			virtual engine_state_data* create();
	};

	class state_containers : public engine_state_data {
		public:
			state_containers() { }
			~state_containers();

			mlist<data_container*> containers;
			mvector<directory_info*> directories;
	};

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
            directory_entry(const directory_entry& o) : Type(o.Type), Name(o.Name), Extension(o.Extension) { }
            ~directory_entry() { }

            DIR_ENTRY_TYPE Type;
            mstring Name;
            mstring Extension;
    };

    class directory_info {
        public:
            directory_info() : m_entries(0), m_entryCount(0) { }
            ~directory_info() { if(m_entries) delete [] m_entries; }

            void populate(const mstring& d);
            int entry_count() const { return m_entryCount; }
            directory_entry* entry(i32 idx) const { return &m_entries[idx]; }

            directory_entry* m_entries;
            i32 m_entryCount;
			mstring m_path;
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
    class data_container {
        public:
            bool read_data(void* data, u32 size);
            bool write_data(const void* data, u32 size);

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
			bool read_string(mstring& data, u32 length = 0);
			bool read_line(mstring& data);

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
			bool write_string(const mstring& data);

            void seek(i32 off); /* adds to m_offset */
            void set_position(u32 pos);
			data_container* sub(size_t length);
            u32 position() const { return m_offset; }
			bool at_end(u32 end_off = 0) const { return m_offset > m_size - end_off; }
			bool at_end_v8() const { return m_offset >= m_size; }
            void* data() { return &m_data[m_offset]; }
			void clear();
            u32 size() const { return m_size; }

            mstring name() const { return m_name; }

        protected:
            friend class file_man;
			friend class state_containers;
			friend class v8pp::raw_ptr_traits;
            data_container(FILE* fp, const mstring& name, DATA_MODE mode);
            ~data_container();

            typedef list<data_container*>::iterator id;

            mstring m_name;
            DATA_MODE m_mode;
            id m_iterator;
			bool m_inState;

            /* For streamed data */
            FILE* m_handle;
            u32 m_size;

            /* For in-memory data */
            mvector<u8> m_data;
			allocator_id m_allocatorId;
            u32 m_offset;
    };

    class r2engine;
    class file_man {
        public:
            file_man() { }
            ~file_man() { }

			void initialize();

            /* container names default to filenames when not set (or pointer address when create() is used) */
            data_container* create(DATA_MODE mode, const mstring& name = "");
            data_container* open(const mstring& file, DATA_MODE mode, const mstring& name = "");
            data_container* load(const mstring& file, DATA_MODE mode, const mstring& name = "");
            bool save(data_container* data, const mstring& file);
			void destroy(data_container* container);

			// used by v8
			void destroy_nodelete(data_container* container);

            mstring working_directory() const;
            void set_working_directory(const mstring& dir);
            directory_info* parse_directory(const mstring& dir);
			void destroy_directory(directory_info* dir);
            bool exists(const mstring& entry);

        protected:
            friend class data_container;
			engine_state_data_ref<state_containers> m_stateData;
    };
};
