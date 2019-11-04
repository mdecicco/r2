#include <v8pp/context.hpp>
#include <libplatform/libplatform.h>
#include <v8.h>

#include <r2/config.h>
#include <r2/utilities/imgui/imgui.h>

namespace r2 {
	class r2engine;
	struct var {
		var(v8::Isolate* i, const v8::Local<v8::Value>& v);
		var(v8::Isolate* i, const std::string& jsonValue);
		~var();

		operator std::string();
		operator i64();
		operator u64();
		operator i32();
		operator u32();
		operator i16();
		operator u16();
		operator i8();
		operator u8();
		operator f64();
		operator f32();
		operator bool();

		var operator[](const char* prop) const;

		v8::Isolate* isolate;
		v8::Local<v8::Value> value;
	};

	struct Vector2 {
		Vector2(f32 _x, f32 _y);
		~Vector2();

		ImVec2 to_imgui() const;
		bool operator ==(const Vector2& rhs) const;
		bool operator !=(const Vector2& rhs) const;

		f32 x;
		f32 y;
	};

	struct Vector3 {
		Vector3(f32 _x, f32 _y, f32 _z);
		~Vector3();

		bool operator ==(const Vector3& rhs) const;
		bool operator !=(const Vector3& rhs) const;

		f32 x;
		f32 y;
		f32 z;
	};

	struct Vector4 {
		Vector4(f32 _x, f32 _y, f32 _z, f32 _w);
		~Vector4();

		ImVec4 to_imgui() const;
		bool operator ==(const Vector4& rhs) const;
		bool operator !=(const Vector4& rhs) const;

		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};

	v8::Local<v8::FunctionTemplate> create_class(v8pp::context* ctx, const std::string& className, v8::FunctionCallback constructor);

	struct trace {
		trace(v8::Isolate* isolate);

		std::string file;
		std::string function;
		i32 line;
	};

	typedef const v8::FunctionCallbackInfo<v8::Value>& v8Args;

	template <typename T>
	T* unwrap(v8Args args) {
		auto self = args.Holder();
		auto wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
		return static_cast<T*>(wrap->Value());
	}

	template <typename T>
	T* unwrapArg(const v8::Local<v8::Value>& val) {
		//auto self = args.Holder();
		//auto wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
		//return static_cast<T*>(wrap->Value());
		auto obj = v8::Local<v8::Object>::Cast(val);
		auto wrap = v8::Local<v8::External>::Cast(obj->GetInternalField(0));
		return static_cast<T*>(wrap->Value());
	}

	/*
	* Construct a new c++ object and wrap it in a js object
	*/
	template <typename T, typename... Args>
	v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>> make_object(v8::Handle<v8::Object> object, Args&&... args) {
		auto isolate = object->GetIsolate();
		auto x = new T(std::forward<Args>(args)...);
		auto obj = v8::Persistent<Object, v8::CopyablePersistentTraits<Object>>(isolate, object);
		obj.Get(isolate)->SetInternalField(0, v8::External::New(isolate, x));
		v8::WeakCallbackInfo<T>::Callback cb = [](const v8::WeakCallbackInfo<T>& data) {
			auto x = static_cast<T*>(data.GetInternalField(0));
			delete x;
		};

		obj.SetWeak(x, cb, v8::WeakCallbackType::kParameter);

		return obj;
	}

	void bind_engine(r2engine* eng, v8pp::context* ctx);
	void bind_event(r2engine* eng, v8pp::context* ctx);
	void bind_math(v8pp::context* ctx);
	void bind_imgui(v8pp::context* ctx);
}