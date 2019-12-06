#include <r2/engine.h>
#include <r2/bindings/math_converters.h>

namespace v8pp {
	template <typename T>
	Local<Value> instantiate_vector_class(Isolate* isolate, const char* className, u8 pcount, const T* params) {
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		EscapableHandleScope scope(isolate);

		using value = Local<Value>;
		using object = Local<Object>;
		using func = Local<Function>;

		value args[16];
		for(u8 i = 0;i < pcount;i++) args[i] = value::New(isolate, Number::New(isolate, params[i]));

		func constructor = func::Cast(global->Get(v8str(className)));
		value ret;
		assert(constructor->CallAsConstructor(context, pcount, args).ToLocal(&ret));
		return scope.Escape(ret);
	}

	template<typename T>
	bool read_numbers(v8::Isolate* isolate, v8::Local<v8::Value>& value, u8 count, T* out) {
		using lo = Local<Object>;
		lo obj = lo::Cast(value);
		auto ctx = isolate->GetCurrentContext();

		auto arr = obj->Get(v8str("__arr"));
		if(!arr->IsUndefined()) {
			// read values from that instead, this is a js vector class
			obj = lo::Cast(arr);
		}

		auto properties = obj->GetPropertyNames(ctx);

		Local<Value> propsValue;
		if(!properties.ToLocal(&propsValue)) return false;

		Local<Array> propArr = Local<Array>::Cast(propsValue);
		i32 pcount = propArr->Length();
		if (pcount != count) {
			r2Error("Expected object or array with exactly %d properties or elements", count);
			for(u32 i = 0;i < pcount;i++){
				auto propName = propArr->Get(i);
				mstring s = convert<mstring>::from_v8(isolate, propName);
				r2Log("Prop[%d]: %s", i, s.c_str());
			}
			return false;
		}

		for(u8 i = 0;i < count;i++) {
			auto v = obj->Get(propArr->Get(i));
			if (v.IsEmpty() || !v->IsNumber()) {
				r2Error("Expected element or property at index %d to be a number", i);
				return false;
			}
			out[i] = convert<T>::from_v8(isolate, v);
		}

		return true;
	}

	template<typename T>
	void write_numbers(v8::Isolate* isolate, v8::Local<v8::Object>& out, u8 count, const T* in) {
		for(u8 i = 0;i < count;i++) {
			out->Set(i, convert<T>::to_v8(isolate, in[i]));
		}
	}

	bool _is_valid_v4(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
		if(value.IsEmpty() || !value->IsObject()) return false;
		i32 ignore[4];
		return read_numbers(isolate, value, 4, ignore);
	}

	bool _is_valid_v3(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
		if(value.IsEmpty() || !value->IsObject()) return false;
		i32 ignore[3];
		return read_numbers(isolate, value, 3, ignore);
	}

	bool _is_valid_v2(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
		if(value.IsEmpty() || !value->IsObject()) return false;
		i32 ignore[2];
		return read_numbers(isolate, value, 2, ignore);
	}

	bool _is_valid_m4(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
		if(value.IsEmpty() || !value->IsObject()) return false;
		i32 ignore[16];
		return read_numbers(isolate, value, 16, ignore);
	}

	bool _is_valid_m3(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
		if(value.IsEmpty() || !value->IsObject()) return false;
		i32 ignore[9];
		return read_numbers(isolate, value, 9, ignore);
	}

	bool _is_valid_m2(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
		if(value.IsEmpty() || !value->IsObject()) return false;
		i32 ignore[4];
		return read_numbers(isolate, value, 4, ignore);
	}


	#define v4(t, tt, postfix) \
	t _from_v8_v4##postfix(v8::Isolate* isolate, v8::Local<v8::Value> value) { \
		tt values[4]; \
		if (read_numbers<tt>(isolate, value, 4, values)) return t(values[0], values[1], values[2], values[3]); \
		return t(); \
	} \
	v8::Handle<v8::Object> _to_v8_v4##postfix(v8::Isolate* isolate, t const& value) { \
		v8::EscapableHandleScope scope(isolate); \
		Local<Object> result = Local<Object>::Cast(instantiate_vector_class<tt>(isolate, #t, 4, &value.x)); \
		return scope.Escape(result); \
	}

	#define v3(t, tt, postfix) \
	t _from_v8_v3##postfix(v8::Isolate* isolate, v8::Local<v8::Value> value) { \
		tt values[3]; \
		if (read_numbers<tt>(isolate, value, 3, values)) return t(values[0], values[1], values[2]); \
		return t(); \
	} \
	v8::Handle<v8::Object> _to_v8_v3##postfix(v8::Isolate* isolate, t const& value) { \
		v8::EscapableHandleScope scope(isolate); \
		Local<Object> result = Local<Object>::Cast(instantiate_vector_class<tt>(isolate, #t, 3, &value.x)); \
		return scope.Escape(result); \
	}

	#define v2(t, tt, postfix) \
	t _from_v8_v2##postfix(v8::Isolate* isolate, v8::Local<v8::Value> value) { \
		tt values[2]; \
		if (read_numbers<tt>(isolate, value, 2, values)) return t(values[0], values[1]); \
		return t(); \
	} \
	v8::Handle<v8::Object> _to_v8_v2##postfix(v8::Isolate* isolate, t const& value) { \
		v8::EscapableHandleScope scope(isolate); \
		Local<Object> result = Local<Object>::Cast(instantiate_vector_class<tt>(isolate, #t, 2, &value.x)); \
		return scope.Escape(result); \
	}

	#define m4(t, tt, postfix) \
	t _from_v8_m4##postfix(v8::Isolate* isolate, v8::Local<v8::Value> value) { \
		tt v[16]; \
		if (read_numbers<tt>(isolate, value, 16, v)) return t(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]); \
		return t(); \
	} \
	v8::Handle<v8::Object> _to_v8_m4##postfix(v8::Isolate* isolate, t const& value) { \
		v8::EscapableHandleScope scope(isolate); \
		Local<Object> result = Local<Object>::Cast(instantiate_vector_class<tt>(isolate, #t, 16, &value[0][0])); \
		return scope.Escape(result); \
	}


	#define m3(t, tt, postfix) \
	t _from_v8_m3##postfix(v8::Isolate* isolate, v8::Local<v8::Value> value) { \
		tt v[9]; \
		if (read_numbers<tt>(isolate, value, 9, v)) return t(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]); \
		return t(); \
	} \
	v8::Handle<v8::Object> _to_v8_m3##postfix(v8::Isolate* isolate, t const& value) { \
		v8::EscapableHandleScope scope(isolate); \
		Local<Object> result = Local<Object>::Cast(instantiate_vector_class<tt>(isolate, #t, 9, &value[0][0])); \
		return scope.Escape(result); \
	}

	#define m2(t, tt, postfix) \
	t _from_v8_m2##postfix(v8::Isolate* isolate, v8::Local<v8::Value> value) { \
		tt v[4]; \
		if (read_numbers<tt>(isolate, value, 4, v)) return t(v[0], v[1], v[2], v[3]); \
		return t(); \
	} \
	v8::Handle<v8::Object> _to_v8_m2##postfix(v8::Isolate* isolate, t const& value) { \
		v8::EscapableHandleScope scope(isolate); \
		Local<Object> result = Local<Object>::Cast(instantiate_vector_class<tt>(isolate, #t, 4, &value[0][0])); \
		return scope.Escape(result); \
	}

	v4(vec4i, i32, i);
	v4(vec4ui, u32, ui);
	v4(vec4f, f32, f);
	v3(vec3i, i32, i);
	v3(vec3ui, u32, ui);
	v3(vec3f, f32, f);
	v2(vec2i, i32, i);
	v2(vec2ui, u32, ui);
	v2(vec2f, f32, f);
	m4(mat4i, i32, i);
	m4(mat4ui, u32, ui);
	m4(mat4f, f32, f);
	m3(mat3i, i32, i);
	m3(mat3ui, u32, ui);
	m3(mat3f, f32, f);
	m2(mat2i, i32, i);
	m2(mat2ui, u32, ui);
	m2(mat2f, f32, f);
};