#include <r2/engine.h>

using namespace v8;
using namespace v8pp;

namespace r2 {
	data_container* create_container(const mstring& name, DATA_MODE dm) {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		data_container* c = r2engine::get()->files()->create(dm, name);
		class_<data_container, v8pp::raw_ptr_traits>::reference_external(i, c);
		return c;
	}
	data_container* open_file(const mstring& file, DATA_MODE dm) {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		mstring fn = file;
		auto bs = fn.find_first_of('\\');
		while(bs != mstring::npos) {
			fn.replace(bs, bs, "/");
			bs = fn.find_first_of('\\');
		}

		trace t(i);

		mstring current_dir = t.file.substr(0, t.file.find_last_of('/') + 1);
		mstring actual_path = current_dir + file;
		data_container* c = r2engine::get()->files()->open(actual_path, dm, actual_path);
		class_<data_container, v8pp::raw_ptr_traits>::reference_external(i, c);
		return c;
	}
	data_container* load_file(const mstring& file, DATA_MODE dm) {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		mstring fn = file;
		auto bs = fn.find_first_of('\\');
		while(bs != mstring::npos) {
			fn.replace(bs, bs, "/");
			bs = fn.find_first_of('\\');
		}

		trace t(i);

		mstring current_dir = t.file.substr(0, t.file.find_last_of('/') + 1);
		mstring actual_path = current_dir + file;
		data_container* c = r2engine::get()->files()->load(actual_path, dm, actual_path);
		class_<data_container, v8pp::raw_ptr_traits>::reference_external(i, c);
		return c;
	}
	bool save_container(const mstring& file, data_container* contents) {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		mstring fn = file;
		auto bs = fn.find_first_of('\\');
		while(bs != mstring::npos) {
			fn.replace(bs, bs, "/");
			bs = fn.find_first_of('\\');
		}

		trace t(i);

		mstring current_dir = t.file.substr(0, t.file.find_last_of('/') + 1);
		mstring actual_path = current_dir + file;
		return r2engine::get()->files()->save(contents, actual_path);
	}
	void close_container(data_container* file) {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		class_<data_container, v8pp::raw_ptr_traits>::unreference_external(i, file);
		r2engine::get()->files()->destroy(file);
	}
	void set_working_directory(const mstring& dir) {
		return r2engine::get()->files()->set_working_directory(dir);
	}
	mstring get_working_directory() {
		return r2engine::get()->files()->working_directory();
	}
	bool exists(const mstring& item) {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		mstring fn = item;
		auto bs = fn.find_first_of('\\');
		while(bs != mstring::npos) {
			fn.replace(bs, bs, "/");
			bs = fn.find_first_of('\\');
		}

		trace t(i);

		mstring current_dir = t.file.substr(0, t.file.find_last_of('/') + 1);
		mstring actual_path = current_dir + item;
		return r2engine::get()->files()->exists(actual_path);
	}
	directory_info* read_dir(const mstring& dir) {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		mstring fn = dir;
		auto bs = fn.find_first_of('\\');
		while(bs != mstring::npos) {
			fn.replace(bs, bs, "/");
			bs = fn.find_first_of('\\');
		}

		trace t(i);

		mstring current_dir = t.file.substr(0, t.file.find_last_of('/') + 1);
		mstring actual_path = current_dir + dir;

		directory_info* info = r2engine::get()->files()->parse_directory(actual_path);
		class_<directory_info, raw_ptr_traits>::reference_external(i, info);
		return info;
	}
	void dir_ent(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<directory_info, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());

		u32 idx = convert<u32>::from_v8(i, args[0]);
		directory_entry* entry = self->entry(idx);
		args.GetReturnValue().Set(class_<directory_entry, raw_ptr_traits>::reference_external(i, entry));
	}


	void dc_read_ubyte(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		u8 val = 0;
		bool ret = self->read_ubyte(val);
		if (ret) args.GetReturnValue().Set(convert<u8>::to_v8(i, val));
	}
	void dc_read_byte(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		s8 val = 0;
		bool ret = self->read_byte(val);
		if (ret) args.GetReturnValue().Set(convert<s8>::to_v8(i, val));
	}
	void dc_read_uint16(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		u16 val = 0;
		bool ret = self->read_uint16(val);
		if (ret) args.GetReturnValue().Set(convert<u16>::to_v8(i, val));
	}
	void dc_read_uint32(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		u32 val = 0;
		bool ret = self->read_uint32(val);
		if (ret) args.GetReturnValue().Set(convert<u32>::to_v8(i, val));
	}
	void dc_read_uint64(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		u64 val = 0;
		bool ret = self->read_uint64(val);
		if (ret) args.GetReturnValue().Set(convert<u64>::to_v8(i, val));
	}
	void dc_read_int16(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		i16 val = 0;
		bool ret = self->read_int16(val);
		if (ret) args.GetReturnValue().Set(convert<i16>::to_v8(i, val));
	}
	void dc_read_int32(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		i32 val = 0;
		bool ret = self->read_int32(val);
		if (ret) args.GetReturnValue().Set(convert<i32>::to_v8(i, val));
	}
	void dc_read_int64(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		i64 val = 0;
		bool ret = self->read_int64(val);
		if (ret) args.GetReturnValue().Set(convert<i64>::to_v8(i, val));
	}
	void dc_read_float32(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		f32 val = 0;
		bool ret = self->read_float32(val);
		if (ret) args.GetReturnValue().Set(convert<f32>::to_v8(i, val));
	}
	void dc_read_float64(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		f64 val = 0;
		bool ret = self->read_float64(val);
		if (ret) args.GetReturnValue().Set(convert<f64>::to_v8(i, val));
	}
	void dc_read_string(v8Args args) {
		if (args.Length() != 1 || !args[0]->IsNumber()) {
			r2Error("Container.read_string requires one parameter, the length of the mstring to read");
			return;
		}
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		mstring val;
		u32 length = convert<u32>::from_v8(i, args[0]);
		bool ret = self->read_string(val, length);
		if (ret) args.GetReturnValue().Set(convert<mstring>::to_v8(i, val));
	}
	void dc_read_line(v8Args args) {
		Isolate* i = args.GetIsolate();
		auto self = class_<data_container, v8pp::raw_ptr_traits>::unwrap_object(i, args.This());
		mstring val;
		bool ret = self->read_line(val);
		

		if (ret) args.GetReturnValue().Set(convert<mstring>::to_v8(i, val));
	}


	void bind_io(context* ctx) {
		auto isolate = ctx->isolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		module m(isolate);

		{
			module dm(isolate);
			dm.set_const("Binary", DM_BINARY);
			dm.set_const("Text", DM_TEXT);
			m.set("Mode", dm);
		}

		{
			module et(isolate);
			et.set_const("File", DET_FILE);
			et.set_const("Folder", DET_FOLDER);
			et.set_const("Link", DET_LINK);
			m.set("EntryType", et);
		}

		{
			using c = directory_entry;
			class_<c, v8pp::raw_ptr_traits> s(isolate);
			register_class_state(s);
			s.set("type", &c::Type);
			s.set("name", &c::Name);
			s.set("extension", &c::Extension);
			m.set("DirectoryEntry", s);
		}

		{
			using c = directory_info;
			class_<c, v8pp::raw_ptr_traits> s(isolate);
			register_class_state(s);
			s.set("length", property(&c::entry_count));
			s.set("entry", &dir_ent);
			m.set("DirectoryInfo", s);
		}

		{
			using c = data_container;
			class_<c, v8pp::raw_ptr_traits> s(isolate);
			register_class_state(s);
			s.set("rw_position", property(&c::position, &c::set_position));
			s.set("size", property(&c::size));
			s.set("name", property(&c::name));
			s.set("at_end", &c::at_end_v8);
			s.set("clear", &c::clear);
			s.set("read_ubyte", &dc_read_ubyte);
			s.set("read_byte", &dc_read_byte);
			s.set("read_uint16", &dc_read_uint16);
			s.set("read_uint32", &dc_read_uint32);
			s.set("read_uint64", &dc_read_uint64);
			s.set("read_int16", &dc_read_int16);
			s.set("read_int32", &dc_read_int32);
			s.set("read_int64", &dc_read_int64);
			s.set("read_float32", &dc_read_float32);
			s.set("read_float64", &dc_read_float64);
			s.set("read_string", &dc_read_string);
			s.set("read_line", &dc_read_line);
			s.set("write_byte", &c::write_byte);
			s.set("write_uint16", &c::write_uint16);
			s.set("write_uint32", &c::write_uint32);
			s.set("write_uint64", &c::write_uint64);
			s.set("write_int16", &c::write_int16);
			s.set("write_int32", &c::write_int32);
			s.set("write_int64", &c::write_int64);
			s.set("write_float32", &c::write_float32);
			s.set("write_float64", &c::write_float64);
			s.set("write_string", &c::write_string);

			m.set("DataContainer", s);
		}

		m.set("CreateContainer", &create_container);
		m.set("OpenFile", &open_file);
		m.set("LoadFile", &load_file);
		m.set("Save", &save_container);
		m.set("Close", &close_container);
		m.set("Exists", &exists);
		m.set("GetWorkingDirectory", &get_working_directory);
		m.set("SetWorkingDirectory", &set_working_directory);

		global->Set(v8str("IO"), m.new_instance());
	}

	void release_io_objects() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<data_container>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<directory_info>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<directory_entry>()).remove_objects();
	}

	void reset_io_object_storage() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<data_container>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<directory_info>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<directory_entry>()).reset_objects_map();
	}
};