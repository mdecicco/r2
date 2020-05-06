#include <r2/engine.h>
#include <r2/utilities/imgui/imgui.h>

#include <v8pp/context.hpp>
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>
#include <v8pp/factory.hpp>
#include <v8pp/config.hpp>
#include <r2/bindings/math_converters.h>

#include <functional>
#include <algorithm>

using namespace v8;
using namespace v8pp;
using namespace std;

#ifndef v8str
	#define v8str(str) v8::String::NewFromUtf8(isolate, str, v8::String::kNormalString, strlen(str))
#endif

using namespace r2;
namespace v8pp {
	bool _is_valid_imv4(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
		return convert<vec4f>::is_valid(isolate, value);
	}

	template <typename T>
	T _from_v8_imv4(v8::Isolate* isolate, v8::Local<v8::Value> value) {
		vec4f v = convert<vec4f>::from_v8(isolate, value);
		return ImVec4(v.x, v.y, v.z, v.w);
	}

	template <typename T>
	v8::Handle<v8::Object> _to_v8_imv4(v8::Isolate* isolate, T const& value) {
		return convert<vec4f>::to_v8(isolate, vec4f(value.x, value.y, value.z, value.w));
	}

	template <typename T>
	v8::Handle<v8::Object> _to_v8_imc(v8::Isolate* isolate, T const& value) {
		v8::EscapableHandleScope scope(isolate);
		v8::Local<v8::Object> obj = v8::Object::New(isolate);

		obj->Set(v8str("r"), convert<f32>::to_v8(isolate, value.Value.x));
		obj->Set(v8str("g"), convert<f32>::to_v8(isolate, value.Value.y));
		obj->Set(v8str("b"), convert<f32>::to_v8(isolate, value.Value.z));
		obj->Set(v8str("a"), convert<f32>::to_v8(isolate, value.Value.w));

		return scope.Escape(obj);
	}

	bool _is_valid_imv2(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
		return convert<vec2f>::is_valid(isolate, value);
	}

	template <typename T>
	T _from_v8_imv2(v8::Isolate* isolate, v8::Local<v8::Value> value) {
		vec2f v = convert<vec2f>::from_v8(isolate, value);
		return ImVec2(v.x, v.y);
	}

	template <typename T>
	v8::Handle<v8::Object> _to_v8_imv2(v8::Isolate* isolate, T const& value) {
		return convert<vec2f>::to_v8(isolate, vec2f(value.x, value.y));
	}

	template<>
	struct convert<ImVec4> {
		using from_type = ImVec4;
		using to_type = v8::Handle<v8::Object>;

		static bool is_valid(v8::Isolate* isolate, v8::Handle<v8::Value> value) { return _is_valid_imv4(isolate, value); }
		static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value) { return _from_v8_imv4<from_type>(isolate, value); }
		static v8::Handle<v8::Object> to_v8(v8::Isolate* isolate, from_type const& value) { return _to_v8_imv4<from_type>(isolate, value); }
	};

	template<>
	struct convert<ImVec2> {
		using from_type = ImVec2;
		using to_type = v8::Handle<v8::Object>;

		static bool is_valid(v8::Isolate* isolate, v8::Handle<v8::Value> value) { return _is_valid_imv2(isolate, value); }
		static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value) { return _from_v8_imv2<from_type>(isolate, value); }
		static v8::Handle<v8::Object> to_v8(v8::Isolate* isolate, from_type const& value) { return _to_v8_imv2<from_type>(isolate, value); }
	};

	template<>
	struct convert<ImColor> {
		using from_type = ImColor;
		using to_type = v8::Handle<v8::Object>;

		static bool is_valid(v8::Isolate* isolate, v8::Handle<v8::Value> value) { return _is_valid_imv4(isolate, value); }
		static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value) { return _from_v8_imv4<from_type>(isolate, value); }
		static v8::Handle<v8::Object> to_v8(v8::Isolate* isolate, from_type const& value) { return _to_v8_imc<from_type>(isolate, value); }
	};
}

namespace r2 {
	struct ImGuiInputTextCallbackData_custom : public ImGuiInputTextCallbackData {
		ImGuiInputTextCallbackData_custom(const ImGuiInputTextCallbackData_custom& o) {
			memcpy(this, &o, sizeof(ImGuiInputTextCallbackData_custom));
		}
		mstring get_buf() { return Buf; }
		void set_buf(const mstring& str) { snprintf(Buf, BufSize, str.c_str()); BufTextLen = str.length(); }
	};

	struct ImGuiPayload_custom : public ImGuiPayload {
		mvector<u8> get_data() {
			mvector<u8> d;
			d.resize(DataSize);
			memcpy(&d[0], Data, DataSize);
			return d;
		}
	};

	int get_flags(const mvector<int>& flags) {
		int flag_input = 0;
		
		if (flags.size() > 0) {
			flag_input = flags[0];
			for (u8 i = 1;i < flags.size();i++) {
				flag_input |= flags[i];
			}
		}

		return flag_input;
	}

	mstring replace_in_str(mstring subject, const mstring& search, const mstring& replace) {
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != mstring::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
		return subject;
	}
	mstring sanitizeText(const mstring& text) {
		mstring t = text;
		t = replace_in_str(t, "%", "%%");
		return t;
	}

	bool Begin(const mstring& name, Local<Function> closed, const mvector<i32>& flags) {
		bool open = true;
		bool ret = ImGui::Begin(name.c_str(), &open, get_flags(flags));
		if (!open) {
			auto ctx = closed->CreationContext();
			auto isolate = closed->GetIsolate();
			closed->Call(ctx, ctx->Global(), 0, NULL);
		}

		return ret;
	}
	bool BeginWithSize(const mstring& name, const ImVec2& size, f32 opacity, Local<Function> closed, const mvector<i32>& flags) {
		bool open = true;
		bool ret = ImGui::Begin(name.c_str(), &open, size, opacity, get_flags(flags));
		if (!open) {
			auto ctx = closed->CreationContext();
			auto isolate = closed->GetIsolate();
			closed->Call(ctx, ctx->Global(), 0, NULL);
		}

		return ret;
	}
	void End() {
		ImGui::End();
	}
	bool BeginChild1(const mstring& id, const ImVec2& size, bool border, const mvector<int>& flags) {
		return ImGui::BeginChild(id.c_str(), size, border, get_flags(flags));;
	}
	bool BeginChild2(ImGuiID id, const ImVec2& size, bool border, const mvector<int>& flags) {
		return ImGui::BeginChild(id, size, border, get_flags(flags));
	}
	void EndChild() {
		ImGui::EndChild();
	}
	void ShowDemoWindow(Local<Function> closed) {
		bool open = true;
		ImGui::ShowDemoWindow(&open);
		if (!open) {
			auto ctx = closed->CreationContext();
			auto isolate = closed->GetIsolate();
			closed->Call(ctx, ctx->Global(), 0, NULL);
		}
	}
	void ShowAboutWindow(Local<Function> closed) {
		bool open = true;
		ImGui::ShowAboutWindow(&open);
		if (!open) {
			auto ctx = closed->CreationContext();
			auto isolate = closed->GetIsolate();
			closed->Call(ctx, ctx->Global(), 0, NULL);
		}
	}
	void ShowMetricsWindow(Local<Function> closed) {
		bool open = true;
		ImGui::ShowMetricsWindow(&open);
		if (!open) {
			auto ctx = closed->CreationContext();
			auto isolate = closed->GetIsolate();
			closed->Call(ctx, ctx->Global(), 0, NULL);
		}
	}
	void ShowStyleEditor() {
		ImGui::ShowStyleEditor();
	}
	void StyleColorsDark() {
		ImGui::StyleColorsDark();
	}
	void StyleColorsClassic() {
		ImGui::StyleColorsClassic();
	}
	void StyleColorsLight() {
		ImGui::StyleColorsLight();
	}
	void SetWindowPos1(const ImVec2& pos, ImGuiCond cond = 0) {
		ImGui::SetWindowPos(pos, cond);
	}
	void SetWindowSize1(const ImVec2& size, ImGuiCond cond = 0) {
		ImGui::SetWindowSize(size, cond);
	}
	void SetWindowCollapsed1(bool collapsed, ImGuiCond cond = 0) {
		ImGui::SetWindowCollapsed(collapsed, cond);
	}
	void SetWindowFocus1() {
		ImGui::SetWindowFocus();
	}
	void SetWindowPos2(const char* name, const ImVec2& pos, ImGuiCond cond = 0) {
		ImGui::SetWindowPos(name, pos, cond);
	}
	void SetWindowSize2(const char* name, const ImVec2& size, ImGuiCond cond = 0) {
		ImGui::SetWindowSize(name, size, cond);
	}
	void SetWindowCollapsed2(const char* name, bool collapsed, ImGuiCond cond = 0) {
		ImGui::SetWindowCollapsed(name, collapsed, cond);
	}
	void SetWindowFocus2(const char* name) {
		ImGui::SetWindowFocus(name);
	}
	void TextUnformatted(const mstring& text) {
		ImGui::TextUnformatted(text.c_str());
	}
	void TextColored(const ImVec4& color, const mstring& text) {
		ImGui::TextColored(color, sanitizeText(text).c_str());
	}
	void TextDisabled(const mstring& text) {
		ImGui::TextDisabled(sanitizeText(text).c_str());
	}
	void TextWrapped(const mstring& text) {
		ImGui::TextWrapped(sanitizeText(text).c_str());
	}
	void LabelText(const mstring& label, const mstring& text) {
		ImGui::LabelText(label.c_str(), sanitizeText(text).c_str());
	}
	void BulletText(const mstring& text) {
		ImGui::BulletText(sanitizeText(text).c_str());
	}
	bool Checkbox(const mstring& label, bool checked, Local<Function> changed) {
		bool cur = checked;
		bool ret = ImGui::Checkbox(label.c_str(), &cur);

		if (cur != checked) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool BeginCombo(const mstring& label, const mstring& preview, const mvector<int>& flags) {
		return ImGui::BeginCombo(label.c_str(), preview.c_str(), get_flags(flags));
	}
	bool Combo(const mstring& label, i32 current, const mvector<mstring>items, i32 max_items, Local<Function> changed) {
		i32 cur = current;
		const char* citems[512] = { 0 };
		for(u16 i = 0; i < items.size() && i < 512;i++) citems[i] = items[i].c_str();
		bool ret = ImGui::Combo(label.c_str(), &cur, citems, items.size(), max_items);

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool DragFloat(const mstring& label, f32 current, f32 speed, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		f32 cur = current;
		bool ret = ImGui::DragFloat(label.c_str(), &cur, speed, min, max, fmt.c_str(), power);

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool DragFloat2(const mstring& label, const vec2f& current, f32 speed, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		vec2f cur = current;
		bool ret = ImGui::DragFloat2(label.c_str(), &cur.x, speed, min, max, fmt.c_str(), power);

		if (cur.x != current.x || cur.y != cur.y) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool DragFloat3(const mstring& label, const vec3f& current, f32 speed, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		vec3f cur = current;
		bool ret = ImGui::DragFloat3(label.c_str(), &cur.x, speed, min, max, fmt.c_str(), power);

		if (cur.x != current.x || cur.y != current.y || cur.z != current.z) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool DragFloat4(const mstring& label, const vec4f& current, f32 speed, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		vec4f cur = current;
		bool ret = ImGui::DragFloat3(label.c_str(), &cur.x, speed, min, max, fmt.c_str(), power);

		if (cur.x != current.x || cur.y != current.y || cur.z != current.z || cur.w != current.w) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool DragInt(const mstring& label, i32 current, f32 speed, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		i32 cur = current;
		bool ret = ImGui::DragInt(label.c_str(), &cur, speed, min, max, fmt.c_str());

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool DragFloatRange(const mstring& label, f32 cur_min, f32 cur_max, f32 speed, f32 min, f32 max, const mstring& fmt_min, const mstring& fmt_max, f32 power, Local<Function> changed) {
		f32 cmin = cur_min;
		f32 cmax = cur_max;
		bool ret = ImGui::DragFloatRange2(label.c_str(), &cmin, &cmax, speed, min, max, fmt_min.c_str(), fmt_max.c_str(), power);

		if (cmin != cur_min || cmax != cur_max) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param[] = { to_v8(isolate, cmin), to_v8(isolate, cmax) };
			changed->Call(ctx, ctx->Global(), 2, param);
		}

		return ret;
	}
	bool DragIntRange(const mstring& label, i32 cur_min, i32 cur_max, f32 speed, i32 min, i32 max, const mstring& fmt_min, const mstring& fmt_max, Local<Function> changed) {
		i32 cmin = cur_min;
		i32 cmax = cur_max;
		bool ret = ImGui::DragIntRange2(label.c_str(), &cmin, &cmax, speed, min, max, fmt_min.c_str(), fmt_max.c_str());

		if (cmin != cur_min || cmax != cur_max) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param[] = { to_v8(isolate, cmin), to_v8(isolate, cmax) };
			changed->Call(ctx, ctx->Global(), 2, param);
		}

		return ret;
	}
	bool SliderFloat(const mstring& label, f32 current, f32 speed, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		f32 cur = current;
		bool ret = ImGui::SliderFloat(label.c_str(), &cur, min, max, fmt.c_str(), power);

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool SliderFloat2(const mstring& label, const vec2f& current, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		vec2f cur = current;
		bool ret = ImGui::SliderFloat2(label.c_str(), &cur.x, min, max, fmt.c_str(), power);

		if (cur.x != current.x || cur.y != cur.y) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool SliderFloat3(const mstring& label, const vec3f& current, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		vec3f cur = current;
		bool ret = ImGui::SliderFloat3(label.c_str(), &cur.x, min, max, fmt.c_str(), power);

		if (cur.x != current.x || cur.y != current.y || cur.z != current.z) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool SliderFloat4(const mstring& label, const vec4f& current, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		vec4f cur = current;
		bool ret = ImGui::SliderFloat3(label.c_str(), &cur.x, min, max, fmt.c_str(), power);

		if (cur.x != current.x || cur.y != current.y || cur.z != current.z || cur.w != current.w) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool SliderAngle(const mstring& label, f32 current, f32 min_deg, f32 max_deg, const mstring& fmt, Local<Function> changed) {
		f32 cur = current;
		bool ret = ImGui::SliderAngle(label.c_str(), &cur, min_deg, max_deg, fmt.c_str());

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool SliderInt(const mstring& label, i32 current, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		i32 cur = current;
		bool ret = ImGui::SliderInt(label.c_str(), &cur, min, max, fmt.c_str());

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool VSliderFloat(const mstring& label, const ImVec2& size, f32 current, f32 speed, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		f32 cur = current;
		bool ret = ImGui::VSliderFloat(label.c_str(), size, &cur, min, max, fmt.c_str(), power);

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool VSliderInt(const mstring& label, const ImVec2& size, i32 current, f32 min, f32 max, const mstring& fmt, f32 power, Local<Function> changed) {
		i32 cur = current;
		bool ret = ImGui::VSliderInt(label.c_str(), size, &cur, min, max, fmt.c_str());

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool InputText(const mstring& label, const mstring& text, i32 max_len, const mvector<int>& flags, Local<Function> changed) {
		static char buf[1024] = { 0 };
		memset(buf, 0, 1024);
		if (max_len > 1023) max_len = 1023;
		memcpy(buf, text.c_str(), text.length());
		
		bool ret = ImGui::InputText(label.c_str(), buf, max_len, get_flags(flags));

		mstring newStr = buf;
		if (newStr != text) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, newStr);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool InputTextMultiline(const mstring& label, const mstring& text, i32 max_len, const ImVec2& size, const mvector<int>& flags, Local<Function> changed) {
		static char buf[16384] = { 0 };
		memset(buf, 0, 16384);
		if (max_len > 16383) max_len = 16383;
		memcpy(buf, text.c_str(), text.length());

		bool ret = ImGui::InputTextMultiline(label.c_str(), buf, max_len, size, get_flags(flags));

		mstring newStr = buf;
		if (newStr != text) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, newStr);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool InputTextWithHint(const mstring& label, const mstring& hint, const mstring& text, i32 max_len, const mvector<int>& flags, Local<Function> changed) {
		static char buf[1024] = { 0 };
		memset(buf, 0, 1024);
		if (max_len > 1023) max_len = 1023;
		memcpy(buf, text.c_str(), text.length());

		bool ret = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buf, max_len, get_flags(flags));

		mstring newStr = buf;
		if (newStr != text) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, newStr);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool InputFloat(const mstring& label, f32 current, f32 step, f32 step_fast, const mstring& fmt, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::InputFloat(label.c_str(), &cur, step, step_fast, fmt.c_str());

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool InputFloat2(const mstring& label, const vec2f& current, const mstring& fmt, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::InputFloat2(label.c_str(), &cur.x, fmt.c_str(), get_flags(flags));

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool InputFloat3(const mstring& label, const vec3f& current, const mstring& fmt, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::InputFloat3(label.c_str(), &cur.x, fmt.c_str(), get_flags(flags));

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool InputFloat4(const mstring& label, const vec4f& current, const mstring& fmt, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::InputFloat4(label.c_str(), &cur.x, fmt.c_str(), get_flags(flags));

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool InputInt(const mstring& label, i32 current, i32 step, i32 step_fast, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::InputInt(label.c_str(), &cur, step, step_fast);

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool ColorEdit3(const mstring& label, ImColor& current, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::ColorEdit3(label.c_str(), &current.Value.x, get_flags(flags));

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool ColorEdit4(const mstring& label, ImColor& current, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::ColorEdit4(label.c_str(), &current.Value.x, get_flags(flags));

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool ColorPicker3(const mstring& label, ImColor& current, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::ColorEdit3(label.c_str(), &current.Value.x, get_flags(flags));

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool ColorPicker4(const mstring& label, ImColor& current, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::ColorPicker4(label.c_str(), &current.Value.x, get_flags(flags));

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool ColorPicker4Ref(const mstring& label, ImColor& current, ImColor& ref, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::ColorPicker4(label.c_str(), &current.Value.x, get_flags(flags), &ref.Value.x);

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool ColorButton(const mstring& desc_id, const ImColor& color, const ImVec2& size, const mvector<i32>& flags) {
		return ImGui::ColorButton(desc_id.c_str(), color.Value, get_flags(flags), size);
	}
	void SetColorEditOptions(const mvector<i32>& flags) {
		ImGui::SetColorEditOptions(get_flags(flags));
	}
	bool TreeNode(const mstring& label, const mvector<i32>& flags) {
		return ImGui::TreeNodeEx(label.c_str(), get_flags(flags));
	}
	void TreePush(const mstring& id) {
		ImGui::TreePush(id.c_str());
	}
	bool CollapsingHeader(const mstring& label, const mvector<i32>& flags) {
		return ImGui::CollapsingHeader(label.c_str(), get_flags(flags));
	}
	bool Selectable(const mstring& label, const ImVec2& size, bool current, const mvector<i32>& flags, Local<Function> changed) {
		auto cur = current;
		bool ret = ImGui::Selectable(label.c_str(), &cur, get_flags(flags), size);

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool ListBox(const mstring& label, i32 current, const mvector<mstring>items, i32 max_items, Local<Function> changed) {
		i32 cur = current;
		const char* citems[512] = { 0 };
		for(u16 i = 0; i < items.size() && i < 512;i++) citems[i] = items[i].c_str();
		bool ret = ImGui::ListBox(label.c_str(), &cur, citems, items.size(), max_items);

		if (cur != current) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	bool ListBoxHeader(const mstring& label, const ImVec2& size) {
		return ImGui::ListBoxHeader(label.c_str(), size);
	}
	bool ListBoxHeader1(const mstring& label, i32 item_count, i32 height_in_items) {
		return ImGui::ListBoxHeader(label.c_str(), item_count, height_in_items);
	}
	void PlotLines(const mstring& label, const mvector<f32>& values, i32 offset, const mstring& overlay, f32 min, f32 max, const ImVec2& size) {
		const f32* ptr = 0;
		if (values.size() > 0) ptr = &values[0];
		ImGui::PlotLines(label.c_str(), ptr, values.size(), offset, overlay.c_str(), min, max, size);
	}
	void PlotHistogram(const mstring& label, const mvector<f32>& values, i32 offset, const mstring& overlay, f32 min, f32 max, const ImVec2& size) {
		const f32* ptr = 0;
		if (values.size() > 0) ptr = &values[0];
		ImGui::PlotHistogram(label.c_str(), ptr, values.size(), offset, overlay.c_str(), min, max, size);
	}
	void BoolValue(const mstring& prefix, bool val) {
		ImGui::Value(prefix.c_str(), val);
	}
	void IntValue(const mstring& prefix, i32 val) {
		ImGui::Value(prefix.c_str(), val);
	}
	void UIntValue(const mstring& prefix, u32 val) {
		ImGui::Value(prefix.c_str(), val);
	}
	void FloatValue(const mstring& prefix, f32 val, const mstring& fmt) {
		ImGui::Value(prefix.c_str(), val, fmt.c_str());
	}
	bool MenuItem(const mstring& label, const mstring& shortcut, bool selected, bool enabled, Local<Function> changed) {
		bool cur = selected;
		bool ret = ImGui::MenuItem(label.c_str(), shortcut.c_str(), &cur, enabled);

		if (cur != selected) {
			auto ctx = changed->CreationContext();
			auto isolate = changed->GetIsolate();
			Local<Value> param = to_v8(isolate, cur);
			changed->Call(ctx, ctx->Global(), 1, &param);
		}

		return ret;
	}
	void SetToolTip(const mstring& label) {
		ImGui::SetTooltip(label.c_str());
	}
	bool BeginPopup(const mstring& str_id, const mvector<i32>& flags) {
		return ImGui::BeginPopup(str_id.c_str(), get_flags(flags));
	}
	bool BeginPopupModal(const mstring& name, const mvector<i32>& flags, Local<Function> closed) {
		bool open = true;
		bool ret = ImGui::BeginPopupModal(name.c_str(), &open, get_flags(flags));
		if (!open) {
			auto ctx = closed->CreationContext();
			auto isolate = closed->GetIsolate();
			closed->Call(ctx, ctx->Global(), 0, NULL);
		}

		return ret;
	}
	bool BeginTabBar(const mstring& str_id, const mvector<i32>& flags) {
		return ImGui::BeginTabBar(str_id.c_str(), get_flags(flags));
	}
	bool BeginTabItem(const mstring& label, const mvector<i32>& flags, Local<Function> closed) {
		bool open = true;
		bool ret = ImGui::BeginTabItem(label.c_str(), &open, get_flags(flags));
		if (!open) {
			auto ctx = closed->CreationContext();
			auto isolate = closed->GetIsolate();
			closed->Call(ctx, ctx->Global(), 0, NULL);
		}

		return ret;
	}
	void LogText(const mstring& text) {
		ImGui::LogText(sanitizeText(text).c_str());
	}
	bool IsItemHovered(const mvector<i32>& flags) {
		return ImGui::IsItemHovered(get_flags(flags));
	}
	bool IsRectVisible(const ImVec2& pos, const ImVec2& size) {
		return ImGui::IsRectVisible(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	}
	bool BeginChildFrame(ImGuiID id, const ImVec2& size, const mvector<i32>& flags) {
		return ImGui::BeginChildFrame(id, size, get_flags(flags));
	}
	bool IsMousePosValid() {
		return ImGui::IsMousePosValid();
	}
	mstring SaveIniSettingsToMemory() {
		return ImGui::SaveIniSettingsToMemory();
	}
	struct ListClippingResult { i32 start, end; };
	ListClippingResult CalcListClipping(i32 count, i32 items_height) {
		ListClippingResult r;
		r.start = -1;
		r.end = -1;
		ImGui::CalcListClipping(count, items_height, &r.start, &r.end);
		return r;
	}
	void PushStyleColor(ImGuiCol idx, const ImVec4& color) {
		ImGui::PushStyleColor(idx, color);
	}
	void PushStyleVarFloat(ImGuiStyleVar idx, float value) {
		ImGui::PushStyleVar(idx, value);
	}
	void PushStyleVarVec2(ImGuiStyleVar idx, const ImVec2& value) {
		ImGui::PushStyleVar(idx, value);
	}
	
	void register_enums(module& imgui) {
		//ImGuiWindowFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiWindowFlags_None);
			m.set_const("NoTitleBar", ImGuiWindowFlags_NoTitleBar);
			m.set_const("NoResize", ImGuiWindowFlags_NoResize);
			m.set_const("NoMove", ImGuiWindowFlags_NoMove);
			m.set_const("NoScrollbar", ImGuiWindowFlags_NoScrollbar);
			m.set_const("NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse);
			m.set_const("NoCollapse", ImGuiWindowFlags_NoCollapse);
			m.set_const("AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize);
			m.set_const("NoBackground", ImGuiWindowFlags_NoBackground);
			m.set_const("NoSavedSettings", ImGuiWindowFlags_NoSavedSettings);
			m.set_const("NoMouseInputs", ImGuiWindowFlags_NoMouseInputs);
			m.set_const("MenuBar", ImGuiWindowFlags_MenuBar);
			m.set_const("HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar);
			m.set_const("NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing);
			m.set_const("NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus);
			m.set_const("AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar);
			m.set_const("AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar);
			m.set_const("AlwaysUseWindowPadding", ImGuiWindowFlags_AlwaysUseWindowPadding);
			m.set_const("NoNavInputs", ImGuiWindowFlags_NoNavInputs);
			m.set_const("NoNavFocus", ImGuiWindowFlags_NoNavFocus);
			m.set_const("UnsavedDocument", ImGuiWindowFlags_UnsavedDocument);
			m.set_const("NoNav", ImGuiWindowFlags_NoNav);
			m.set_const("NoDecoration", ImGuiWindowFlags_NoDecoration);
			m.set_const("NoInputs", ImGuiWindowFlags_NoInputs);
			m.set_const("NavFlattened", ImGuiWindowFlags_NavFlattened);
			m.set_const("ChildWindow", ImGuiWindowFlags_ChildWindow);
			m.set_const("Tooltip", ImGuiWindowFlags_Tooltip);
			m.set_const("Popup", ImGuiWindowFlags_Popup);
			m.set_const("Modal", ImGuiWindowFlags_Modal);
			m.set_const("ChildMenu", ImGuiWindowFlags_ChildMenu);
			imgui.set("WindowFlags", m);
		}

		//ImGuiInputTextFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiInputTextFlags_None);
			m.set_const("CharsDecimal", ImGuiInputTextFlags_CharsDecimal);
			m.set_const("CharsHexadecimal", ImGuiInputTextFlags_CharsHexadecimal);
			m.set_const("CharsUppercase", ImGuiInputTextFlags_CharsUppercase);
			m.set_const("CharsNoBlank", ImGuiInputTextFlags_CharsNoBlank);
			m.set_const("AutoSelectAll", ImGuiInputTextFlags_AutoSelectAll);
			m.set_const("EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue);
			m.set_const("CallbackCompletion", ImGuiInputTextFlags_CallbackCompletion);
			m.set_const("CallbackHistory", ImGuiInputTextFlags_CallbackHistory);
			m.set_const("CallbackAlways", ImGuiInputTextFlags_CallbackAlways);
			m.set_const("CallbackCharFilter", ImGuiInputTextFlags_CallbackCharFilter);
			m.set_const("AllowTabInput", ImGuiInputTextFlags_AllowTabInput);
			m.set_const("CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine);
			m.set_const("NoHorizontalScroll", ImGuiInputTextFlags_NoHorizontalScroll);
			m.set_const("AlwaysInsertMode", ImGuiInputTextFlags_AlwaysInsertMode);
			m.set_const("ReadOnly", ImGuiInputTextFlags_ReadOnly);
			m.set_const("Password", ImGuiInputTextFlags_Password);
			m.set_const("NoUndoRedo", ImGuiInputTextFlags_NoUndoRedo);
			m.set_const("CharsScientific", ImGuiInputTextFlags_CharsScientific);
			m.set_const("CallbackResize", ImGuiInputTextFlags_CallbackResize);
			m.set_const("Multiline", ImGuiInputTextFlags_Multiline);
			m.set_const("NoMarkEdited", ImGuiInputTextFlags_NoMarkEdited);
			imgui.set("InputTextFlags", m);
		}

		//ImGuiTreeNodeFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiTreeNodeFlags_None);
			m.set_const("Selected", ImGuiTreeNodeFlags_Selected);
			m.set_const("Framed", ImGuiTreeNodeFlags_Framed);
			m.set_const("AllowItemOverlap", ImGuiTreeNodeFlags_AllowItemOverlap);
			m.set_const("NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen);
			m.set_const("NoAutoOpenOnLog", ImGuiTreeNodeFlags_NoAutoOpenOnLog);
			m.set_const("DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen);
			m.set_const("OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick);
			m.set_const("OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow);
			m.set_const("Leaf", ImGuiTreeNodeFlags_Leaf);
			m.set_const("Bullet", ImGuiTreeNodeFlags_Bullet);
			m.set_const("FramePadding", ImGuiTreeNodeFlags_FramePadding);
			m.set_const("SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth);
			m.set_const("SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth);
			m.set_const("NavLeftJumpsBackHere", ImGuiTreeNodeFlags_NavLeftJumpsBackHere);
			m.set_const("CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader);
			m.set_const("AllowOverlapMode", ImGuiTreeNodeFlags_AllowOverlapMode);
			imgui.set("TreeNodeFlags", m);
		}

		//ImGuiSelectableFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiSelectableFlags_None);
			m.set_const("DontClosePopups", ImGuiSelectableFlags_DontClosePopups);
			m.set_const("SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns);
			m.set_const("AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick);
			m.set_const("Disabled", ImGuiSelectableFlags_Disabled);
			m.set_const("AllowItemOverlap", ImGuiSelectableFlags_AllowItemOverlap);
			imgui.set("SelectableFlags", m);
		}

		//ImGuiComboFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiComboFlags_None);
			m.set_const("PopupAlignLeft", ImGuiComboFlags_PopupAlignLeft);
			m.set_const("HeightSmall", ImGuiComboFlags_HeightSmall);
			m.set_const("HeightRegular", ImGuiComboFlags_HeightRegular);
			m.set_const("HeightLarge", ImGuiComboFlags_HeightLarge);
			m.set_const("HeightLargest", ImGuiComboFlags_HeightLargest);
			m.set_const("NoArrowButton", ImGuiComboFlags_NoArrowButton);
			m.set_const("NoPreview", ImGuiComboFlags_NoPreview);
			m.set_const("HeightMask", ImGuiComboFlags_HeightMask_);
			imgui.set("ComboFlags", m);
		}

		//ImGuiTabBarFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiTabBarFlags_None);
			m.set_const("Reorderable", ImGuiTabBarFlags_Reorderable);
			m.set_const("AutoSelectNewTabs", ImGuiTabBarFlags_AutoSelectNewTabs);
			m.set_const("TabListPopupButton", ImGuiTabBarFlags_TabListPopupButton);
			m.set_const("NoCloseWithMiddleMouseButton", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
			m.set_const("NoTabListScrollingButtons", ImGuiTabBarFlags_NoTabListScrollingButtons);
			m.set_const("NoTooltip", ImGuiTabBarFlags_NoTooltip);
			m.set_const("FittingPolicyResizeDown", ImGuiTabBarFlags_FittingPolicyResizeDown);
			m.set_const("FittingPolicyScroll", ImGuiTabBarFlags_FittingPolicyScroll);
			m.set_const("FittingPolicyMask", ImGuiTabBarFlags_FittingPolicyMask_);
			m.set_const("FittingPolicyDefault", ImGuiTabBarFlags_FittingPolicyDefault_);
			imgui.set("TabBarFlags", m);
		}

		//ImGuiTabItemFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiTabItemFlags_None);
			m.set_const("UnsavedDocument", ImGuiTabItemFlags_UnsavedDocument);
			m.set_const("SetSelected", ImGuiTabItemFlags_SetSelected);
			m.set_const("NoCloseWithMiddleMouseButton", ImGuiTabItemFlags_NoCloseWithMiddleMouseButton);
			m.set_const("NoPushId", ImGuiTabItemFlags_NoPushId);
			imgui.set("TabItemFlags", m);
		}

		//ImGuiFocusedFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiFocusedFlags_None);
			m.set_const("ChildWindows", ImGuiFocusedFlags_ChildWindows);
			m.set_const("RootWindow", ImGuiFocusedFlags_RootWindow);
			m.set_const("AnyWindow", ImGuiFocusedFlags_AnyWindow);
			m.set_const("RootAndChildWindows", ImGuiFocusedFlags_RootAndChildWindows);
			imgui.set("FocusedFlags", m);
		}

		//ImGuiHoveredFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiHoveredFlags_None);
			m.set_const("ChildWindows", ImGuiHoveredFlags_ChildWindows);
			m.set_const("RootWindow", ImGuiHoveredFlags_RootWindow);
			m.set_const("AnyWindow", ImGuiHoveredFlags_AnyWindow);
			m.set_const("AllowWhenBlockedByPopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup);
			m.set_const("AllowWhenBlockedByActiveItem", ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
			m.set_const("AllowWhenOverlapped", ImGuiHoveredFlags_AllowWhenOverlapped);
			m.set_const("AllowWhenDisabled", ImGuiHoveredFlags_AllowWhenDisabled);
			m.set_const("RectOnly", ImGuiHoveredFlags_RectOnly);
			m.set_const("RootAndChildWindows", ImGuiHoveredFlags_RootAndChildWindows);
			imgui.set("HoveredFlags", m);
		}

		//ImGuiDragDropFlags
		/*
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiDragDropFlags_None);
			m.set_const("SourceNoPreviewTooltip", ImGuiDragDropFlags_SourceNoPreviewTooltip);
			m.set_const("SourceNoDisableHover", ImGuiDragDropFlags_SourceNoDisableHover);
			m.set_const("SourceNoHoldToOpenOthers", ImGuiDragDropFlags_SourceNoHoldToOpenOthers);
			m.set_const("SourceAllowNullID", ImGuiDragDropFlags_SourceAllowNullID);
			m.set_const("SourceExtern", ImGuiDragDropFlags_SourceExtern);
			m.set_const("SourceAutoExpirePayload", ImGuiDragDropFlags_SourceAutoExpirePayload);
			m.set_const("AcceptBeforeDelivery", ImGuiDragDropFlags_AcceptBeforeDelivery);
			m.set_const("AcceptNoDrawDefaultRect", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
			m.set_const("AcceptNoPreviewTooltip", ImGuiDragDropFlags_AcceptNoPreviewTooltip);
			m.set_const("AcceptPeekOnly", ImGuiDragDropFlags_AcceptPeekOnly);
			imgui.set("DragDropFlags", m);
		}
		*/

		//ImGuiDataType
		{
			module m(imgui.isolate());
			m.set_const("S8", ImGuiDataType_S8);
			m.set_const("U8", ImGuiDataType_U8);
			m.set_const("S16", ImGuiDataType_S16);
			m.set_const("U16", ImGuiDataType_U16);
			m.set_const("S32", ImGuiDataType_S32);
			m.set_const("U32", ImGuiDataType_U32);
			m.set_const("S64", ImGuiDataType_S64);
			m.set_const("U64", ImGuiDataType_U64);
			m.set_const("Float", ImGuiDataType_Float);
			m.set_const("Double", ImGuiDataType_Double);
			imgui.set("DataType", m);
		}

		//ImGuiDir
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiDir_None);
			m.set_const("Left", ImGuiDir_Left);
			m.set_const("Right", ImGuiDir_Right);
			m.set_const("Up", ImGuiDir_Up);
			m.set_const("Down", ImGuiDir_Down);
			imgui.set("Dir", m);
		}

		//ImGuiKey
		{
			module m(imgui.isolate());
			m.set_const("Tab", ImGuiKey_Tab);
			m.set_const("LeftArrow", ImGuiKey_LeftArrow);
			m.set_const("RightArrow", ImGuiKey_RightArrow);
			m.set_const("UpArrow", ImGuiKey_UpArrow);
			m.set_const("DownArrow", ImGuiKey_DownArrow);
			m.set_const("PageUp", ImGuiKey_PageUp);
			m.set_const("PageDown", ImGuiKey_PageDown);
			m.set_const("Home", ImGuiKey_Home);
			m.set_const("End", ImGuiKey_End);
			m.set_const("Insert", ImGuiKey_Insert);
			m.set_const("Delete", ImGuiKey_Delete);
			m.set_const("Backspace", ImGuiKey_Backspace);
			m.set_const("Space", ImGuiKey_Space);
			m.set_const("Enter", ImGuiKey_Enter);
			m.set_const("Escape", ImGuiKey_Escape);
			m.set_const("KeyPadEnter", ImGuiKey_KeyPadEnter);
			m.set_const("A", ImGuiKey_A);
			m.set_const("C", ImGuiKey_C);
			m.set_const("V", ImGuiKey_V);
			m.set_const("X", ImGuiKey_X);
			m.set_const("Y", ImGuiKey_Y);
			m.set_const("Z", ImGuiKey_Z);
			imgui.set("Key", m);
		}

		//ImGuiNavInput
		{
			module m(imgui.isolate());
			m.set_const("Activate", ImGuiNavInput_Activate);
			m.set_const("Cancel", ImGuiNavInput_Cancel);
			m.set_const("Input", ImGuiNavInput_Input);
			m.set_const("Menu", ImGuiNavInput_Menu);
			m.set_const("DpadLeft", ImGuiNavInput_DpadLeft);
			m.set_const("DpadRight", ImGuiNavInput_DpadRight);
			m.set_const("DpadUp", ImGuiNavInput_DpadUp);
			m.set_const("DpadDown", ImGuiNavInput_DpadDown);
			m.set_const("LStickLeft", ImGuiNavInput_LStickLeft);
			m.set_const("LStickRight", ImGuiNavInput_LStickRight);
			m.set_const("LStickUp", ImGuiNavInput_LStickUp);
			m.set_const("LStickDown", ImGuiNavInput_LStickDown);
			m.set_const("FocusPrev", ImGuiNavInput_FocusPrev);
			m.set_const("FocusNext", ImGuiNavInput_FocusNext);
			m.set_const("TweakSlow", ImGuiNavInput_TweakSlow);
			m.set_const("TweakFast", ImGuiNavInput_TweakFast);
			m.set_const("KeyMenu", ImGuiNavInput_KeyMenu_);
			m.set_const("KeyLeft", ImGuiNavInput_KeyLeft_);
			m.set_const("KeyRight", ImGuiNavInput_KeyRight_);
			m.set_const("KeyUp", ImGuiNavInput_KeyUp_);
			m.set_const("KeyDown", ImGuiNavInput_KeyDown_);
			m.set_const("InternalStart", ImGuiNavInput_InternalStart_);
			imgui.set("NavInput", m);
		}

		//ImGuiConfigFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiConfigFlags_None);
			m.set_const("NavEnableKeyboard", ImGuiConfigFlags_NavEnableKeyboard);
			m.set_const("NavEnableGamepad", ImGuiConfigFlags_NavEnableGamepad);
			m.set_const("NavEnableSetMousePos", ImGuiConfigFlags_NavEnableSetMousePos);
			m.set_const("NavNoCaptureKeyboard", ImGuiConfigFlags_NavNoCaptureKeyboard);
			m.set_const("NoMouse", ImGuiConfigFlags_NoMouse);
			m.set_const("NoMouseCursorChange", ImGuiConfigFlags_NoMouseCursorChange);
			m.set_const("IsSRGB", ImGuiConfigFlags_IsSRGB);
			m.set_const("IsTouchScreen", ImGuiConfigFlags_IsTouchScreen);
			imgui.set("ConfigFlags", m);
		}

		//ImGuiBackendFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiBackendFlags_None);
			m.set_const("HasGamepad", ImGuiBackendFlags_HasGamepad);
			m.set_const("HasMouseCursors", ImGuiBackendFlags_HasMouseCursors);
			m.set_const("HasSetMousePos", ImGuiBackendFlags_HasSetMousePos);
			m.set_const("RendererHasVtxOffset", ImGuiBackendFlags_RendererHasVtxOffset);
			imgui.set("BackendFlags", m);
		}

		//ImGuiCol
		{
			module m(imgui.isolate());
			m.set_const("Text", ImGuiCol_Text);
			m.set_const("TextDisabled", ImGuiCol_TextDisabled);
			m.set_const("WindowBg", ImGuiCol_WindowBg);
			m.set_const("ChildBg", ImGuiCol_ChildBg);
			m.set_const("PopupBg", ImGuiCol_PopupBg);
			m.set_const("Border", ImGuiCol_Border);
			m.set_const("BorderShadow", ImGuiCol_BorderShadow);
			m.set_const("FrameBg", ImGuiCol_FrameBg);
			m.set_const("FrameBgHovered", ImGuiCol_FrameBgHovered);
			m.set_const("FrameBgActive", ImGuiCol_FrameBgActive);
			m.set_const("TitleBg", ImGuiCol_TitleBg);
			m.set_const("TitleBgActive", ImGuiCol_TitleBgActive);
			m.set_const("TitleBgCollapsed", ImGuiCol_TitleBgCollapsed);
			m.set_const("MenuBarBg", ImGuiCol_MenuBarBg);
			m.set_const("ScrollbarBg", ImGuiCol_ScrollbarBg);
			m.set_const("ScrollbarGrab", ImGuiCol_ScrollbarGrab);
			m.set_const("ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered);
			m.set_const("ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive);
			m.set_const("CheckMark", ImGuiCol_CheckMark);
			m.set_const("SliderGrab", ImGuiCol_SliderGrab);
			m.set_const("SliderGrabActive", ImGuiCol_SliderGrabActive);
			m.set_const("Button", ImGuiCol_Button);
			m.set_const("ButtonHovered", ImGuiCol_ButtonHovered);
			m.set_const("ButtonActive", ImGuiCol_ButtonActive);
			m.set_const("Header", ImGuiCol_Header);
			m.set_const("HeaderHovered", ImGuiCol_HeaderHovered);
			m.set_const("HeaderActive", ImGuiCol_HeaderActive);
			m.set_const("Separator", ImGuiCol_Separator);
			m.set_const("SeparatorHovered", ImGuiCol_SeparatorHovered);
			m.set_const("SeparatorActive", ImGuiCol_SeparatorActive);
			m.set_const("ResizeGrip", ImGuiCol_ResizeGrip);
			m.set_const("ResizeGripHovered", ImGuiCol_ResizeGripHovered);
			m.set_const("ResizeGripActive", ImGuiCol_ResizeGripActive);
			m.set_const("Tab", ImGuiCol_Tab);
			m.set_const("TabHovered", ImGuiCol_TabHovered);
			m.set_const("TabActive", ImGuiCol_TabActive);
			m.set_const("TabUnfocused", ImGuiCol_TabUnfocused);
			m.set_const("TabUnfocusedActive", ImGuiCol_TabUnfocusedActive);
			m.set_const("PlotLines", ImGuiCol_PlotLines);
			m.set_const("PlotLinesHovered", ImGuiCol_PlotLinesHovered);
			m.set_const("PlotHistogram", ImGuiCol_PlotHistogram);
			m.set_const("PlotHistogramHovered", ImGuiCol_PlotHistogramHovered);
			m.set_const("TextSelectedBg", ImGuiCol_TextSelectedBg);
			m.set_const("DragDropTarget", ImGuiCol_DragDropTarget);
			m.set_const("NavHighlight", ImGuiCol_NavHighlight);
			m.set_const("NavWindowingHighlight", ImGuiCol_NavWindowingHighlight);
			m.set_const("NavWindowingDimBg", ImGuiCol_NavWindowingDimBg);
			m.set_const("ModalWindowDimBg", ImGuiCol_ModalWindowDimBg);
			m.set_const("ModalWindowDarkening", ImGuiCol_ModalWindowDarkening);
			m.set_const("ChildWindowBg", ImGuiCol_ChildWindowBg);
			imgui.set("Color", m);
		}

		//ImGuiStyleVar
		{
			module m(imgui.isolate());
			m.set_const("Alpha", ImGuiStyleVar_Alpha);
			m.set_const("WindowPadding", ImGuiStyleVar_WindowPadding);
			m.set_const("WindowRounding", ImGuiStyleVar_WindowRounding);
			m.set_const("WindowBorderSize", ImGuiStyleVar_WindowBorderSize);
			m.set_const("WindowMinSize", ImGuiStyleVar_WindowMinSize);
			m.set_const("WindowTitleAlign", ImGuiStyleVar_WindowTitleAlign);
			m.set_const("ChildRounding", ImGuiStyleVar_ChildRounding);
			m.set_const("ChildBorderSize", ImGuiStyleVar_ChildBorderSize);
			m.set_const("PopupRounding", ImGuiStyleVar_PopupRounding);
			m.set_const("PopupBorderSize", ImGuiStyleVar_PopupBorderSize);
			m.set_const("FramePadding", ImGuiStyleVar_FramePadding);
			m.set_const("FrameRounding", ImGuiStyleVar_FrameRounding);
			m.set_const("FrameBorderSize", ImGuiStyleVar_FrameBorderSize);
			m.set_const("ItemSpacing", ImGuiStyleVar_ItemSpacing);
			m.set_const("ItemInnerSpacing", ImGuiStyleVar_ItemInnerSpacing);
			m.set_const("IndentSpacing", ImGuiStyleVar_IndentSpacing);
			m.set_const("ScrollbarSize", ImGuiStyleVar_ScrollbarSize);
			m.set_const("ScrollbarRounding", ImGuiStyleVar_ScrollbarRounding);
			m.set_const("GrabMinSize", ImGuiStyleVar_GrabMinSize);
			m.set_const("GrabRounding", ImGuiStyleVar_GrabRounding);
			m.set_const("TabRounding", ImGuiStyleVar_TabRounding);
			m.set_const("ButtonTextAlign", ImGuiStyleVar_ButtonTextAlign);
			m.set_const("SelectableTextAlign", ImGuiStyleVar_SelectableTextAlign);
			m.set_const("ChildWindowRounding", ImGuiStyleVar_ChildWindowRounding);
			imgui.set("StyleVar", m);
		}

		//ImGuiColorEditFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiColorEditFlags_None);
			m.set_const("NoAlpha", ImGuiColorEditFlags_NoAlpha);
			m.set_const("NoPicker", ImGuiColorEditFlags_NoPicker);
			m.set_const("NoOptions", ImGuiColorEditFlags_NoOptions);
			m.set_const("NoSmallPreview", ImGuiColorEditFlags_NoSmallPreview);
			m.set_const("NoInputs", ImGuiColorEditFlags_NoInputs);
			m.set_const("NoTooltip", ImGuiColorEditFlags_NoTooltip);
			m.set_const("NoLabel", ImGuiColorEditFlags_NoLabel);
			m.set_const("NoSidePreview", ImGuiColorEditFlags_NoSidePreview);
			m.set_const("NoDragDrop", ImGuiColorEditFlags_NoDragDrop);
			m.set_const("AlphaBar", ImGuiColorEditFlags_AlphaBar);
			m.set_const("AlphaPreview", ImGuiColorEditFlags_AlphaPreview);
			m.set_const("AlphaPreviewHalf", ImGuiColorEditFlags_AlphaPreviewHalf);
			m.set_const("HDR", ImGuiColorEditFlags_HDR);
			m.set_const("DisplayRGB", ImGuiColorEditFlags_DisplayRGB);
			m.set_const("DisplayHSV", ImGuiColorEditFlags_DisplayHSV);
			m.set_const("DisplayHex", ImGuiColorEditFlags_DisplayHex);
			m.set_const("Uint8", ImGuiColorEditFlags_Uint8);
			m.set_const("Float", ImGuiColorEditFlags_Float);
			m.set_const("PickerHueBar", ImGuiColorEditFlags_PickerHueBar);
			m.set_const("PickerHueWheel", ImGuiColorEditFlags_PickerHueWheel);
			m.set_const("InputRGB", ImGuiColorEditFlags_InputRGB);
			m.set_const("InputHSV", ImGuiColorEditFlags_InputHSV);
			m.set_const("OptionsDefault", ImGuiColorEditFlags__OptionsDefault);
			m.set_const("DisplayMask", ImGuiColorEditFlags__DisplayMask);
			m.set_const("DataTypeMask", ImGuiColorEditFlags__DataTypeMask);
			m.set_const("PickerMask", ImGuiColorEditFlags__PickerMask);
			m.set_const("InputMask", ImGuiColorEditFlags__InputMask);
			m.set_const("RGB", ImGuiColorEditFlags_RGB);
			m.set_const("HSV", ImGuiColorEditFlags_HSV);
			m.set_const("HEX", ImGuiColorEditFlags_HEX);
			imgui.set("ColorEditFlags", m);
		}

		//ImGuiMouseCursor
		{
			module m(imgui.isolate());
			m.set_const("None", ImGuiMouseCursor_None);
			m.set_const("Arrow", ImGuiMouseCursor_Arrow);
			m.set_const("TextInput", ImGuiMouseCursor_TextInput);
			m.set_const("ResizeAll", ImGuiMouseCursor_ResizeAll);
			m.set_const("ResizeNS", ImGuiMouseCursor_ResizeNS);
			m.set_const("ResizeEW", ImGuiMouseCursor_ResizeEW);
			m.set_const("ResizeNESW", ImGuiMouseCursor_ResizeNESW);
			m.set_const("ResizeNWSE", ImGuiMouseCursor_ResizeNWSE);
			m.set_const("Hand", ImGuiMouseCursor_Hand);
			imgui.set("MouseCursor", m);
		}

		//ImGuiCond
		{
			module m(imgui.isolate());
			m.set_const("Always", ImGuiCond_Always);
			m.set_const("Once", ImGuiCond_Once);
			m.set_const("FirstUseEver", ImGuiCond_FirstUseEver);
			m.set_const("Appearing", ImGuiCond_Appearing);
			imgui.set("Cond", m);
		}

		//ImGuiDrawCornerFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImDrawCornerFlags_None);
			m.set_const("TopLeft", ImDrawCornerFlags_TopLeft);
			m.set_const("TopRight", ImDrawCornerFlags_TopRight);
			m.set_const("BotLeft", ImDrawCornerFlags_BotLeft);
			m.set_const("BotRight", ImDrawCornerFlags_BotRight);
			m.set_const("Top", ImDrawCornerFlags_Top);
			m.set_const("Bot", ImDrawCornerFlags_Bot);
			m.set_const("Left", ImDrawCornerFlags_Left);
			m.set_const("Right", ImDrawCornerFlags_Right);
			m.set_const("All", ImDrawCornerFlags_All);
			imgui.set("DrawCornerFlags", m);
		}

		//ImFontAtlasFlags
		{
			module m(imgui.isolate());
			m.set_const("None", ImFontAtlasFlags_None);
			m.set_const("NoPowerOfTwoHeight", ImFontAtlasFlags_NoPowerOfTwoHeight);
			m.set_const("NoMouseCursors", ImFontAtlasFlags_NoMouseCursors);
			imgui.set("", m);
		}
	}
	void register_structs(module& m) {
		/*
		//ImVec2
		{
			class_<ImVec2, v8pp::raw_ptr_traits> s(m.isolate());
			register_class_state(s);
			s.ctor<f32, f32>();
			s.set("x", &ImVec2::x);
			s.set("y", &ImVec2::y);
			m.set("ImVec2", s);
		}

		//ImVec4
		{
			class_<ImVec4, v8pp::raw_ptr_traits>s(m.isolate());
			register_class_state(s);
			s.ctor<f32, f32, f32, f32>();
			s.set("x", &ImVec4::x);
			s.set("y", &ImVec4::y);
			s.set("z", &ImVec4::z);
			s.set("w", &ImVec4::w);
			m.set("ImVec4", s);
		}
		*/

		//ImGuiInputTextCallbackData
		/*
		{
			class_<ImGuiInputTextCallbackData_custom, v8pp::raw_ptr_traits>s(m.isolate());
		register_class_state(s);
			s.set("EventFlag", &ImGuiInputTextCallbackData_custom::EventFlag);
			s.set("Flags", &ImGuiInputTextCallbackData_custom::Flags);
			s.set("EventKey", &ImGuiInputTextCallbackData_custom::EventKey);
			s.set("Buf", property(&ImGuiInputTextCallbackData_custom::get_buf, &ImGuiInputTextCallbackData_custom::set_buf));
			s.set("BufTextLen", &ImGuiInputTextCallbackData_custom::BufTextLen);
			s.set("BufSize", &ImGuiInputTextCallbackData_custom::BufSize);
			s.set("BufDirty", &ImGuiInputTextCallbackData_custom::BufDirty);
			s.set("CursorPos", &ImGuiInputTextCallbackData_custom::CursorPos);
			s.set("SelectionStart", &ImGuiInputTextCallbackData_custom::SelectionStart);
			s.set("SelectionEnd", &ImGuiInputTextCallbackData_custom::SelectionEnd);
			s.set("DeleteChars", &ImGuiInputTextCallbackData_custom::DeleteChars);
			s.set("InsertChars", &ImGuiInputTextCallbackData_custom::InsertChars);
			s.set("HasSelection", &ImGuiInputTextCallbackData_custom::HasSelection);
			m.set("ImGuiInputTextCallbackData", s);
		}
		*/

		//ImGuiSizeCallbackData
		{
			class_<ImGuiSizeCallbackData, v8pp::raw_ptr_traits>s(m.isolate());
			register_class_state(s);
			s.set("Pos", &ImGuiSizeCallbackData::Pos);
			s.set("CurrentSize", &ImGuiSizeCallbackData::CurrentSize);
			s.set("DesiredSize", &ImGuiSizeCallbackData::DesiredSize);
			m.set("ImGuiSizeCallbackData", s);
		}

		//ImGuiPayload
		/*
		{
			class_<ImGuiPayload_custom, v8pp::raw_ptr_traits>s(m.isolate());
			register_class_state(s);
			s.ctor();
			s.set("Data", property(&ImGuiPayload_custom::get_data));
			s.set("DataSize", &ImGuiPayload_custom::DataSize);
			s.set("SourceId", &ImGuiPayload_custom::SourceId);
			s.set("SourceParentId", &ImGuiPayload_custom::SourceParentId);
			s.set("Clear", &ImGuiPayload_custom::Clear);
			s.set("IsDataType", &ImGuiPayload_custom::IsDataType);
			s.set("IsPreview", &ImGuiPayload_custom::IsPreview);
			s.set("IsDelivery", &ImGuiPayload_custom::IsDelivery);
			m.set("ImGuiPayload", s);
		}

		//ImColor
		{
			class_<ImColor, v8pp::raw_ptr_traits>s(m.isolate());
			register_class_state(s);
			s.ctor<const ImVec4&>();
			s.ctor<f32,f32,f32,f32>();
			s.set("Value", &ImColor::Value);
			s.set("SetHSV", &ImColor::SetHSV);
			//s.set_static("HSV", &ImColor::HSV, true);
			m.set("ImColor", s);
		}
		*/

		//ListClippingResult
		{
			class_<ListClippingResult, v8pp::raw_ptr_traits>s(m.isolate());
			register_class_state(s);
			s.set("start", &ListClippingResult::start);
			s.set("end", &ListClippingResult::end);
			m.set("ListClippingResult", s);
		}
	}

	void bind_imgui(context* ctx) {
		auto isolate = ctx->isolate();
		module m(isolate);
		register_enums(m);
		register_structs(m);

		//m.set("SetNextWindowSizeConstraints", &ImGui::SetNextWindowSizeConstraints);
		//m.set("Image", &ImGui::Image);
		//m.set("ImageButton", &ImGui::ImageButton);
		//m.set("ScaleAllSizes", &ImGui::ScaleAllSizes);

		m.set("Begin", &Begin);
		m.set("BeginWithSize", &BeginWithSize);
		m.set("End", &End);
		m.set("BeginChild", &BeginChild2);
		m.set("EndChild", &EndChild);
		m.set("ShowUserGuide", &ImGui::ShowUserGuide);
		m.set("GetVersion", &ImGui::GetVersion);
		m.set("ShowStyleSelector", &ImGui::ShowStyleSelector);
		m.set("ShowFontSelector", &ImGui::ShowFontSelector);
		m.set("ShowDemoWindow", &ShowDemoWindow);
		m.set("ShowAboutWindow", &ShowAboutWindow);
		m.set("ShowMetricsWindow", &ShowMetricsWindow);
		m.set("ShowStyleEditor", &ShowStyleEditor);
		m.set("IsWindowAppearing", &ImGui::IsWindowAppearing);
		m.set("IsWindowCollapsed", &ImGui::IsWindowCollapsed);
		m.set("IsWindowFocused", &ImGui::IsWindowFocused);
		m.set("IsWindowHovered", &ImGui::IsWindowHovered);
		m.set("GetWindowPos", &ImGui::GetWindowPos);
		m.set("GetWindowSize", &ImGui::GetWindowSize);
		m.set("GetWindowWidth", &ImGui::GetWindowWidth);
		m.set("GetWindowHeight", &ImGui::GetWindowHeight);
		m.set("SetNextWindowPos", &ImGui::SetNextWindowPos);
		m.set("SetNextWindowSize", &ImGui::SetNextWindowSize);
		m.set("SetNextWindowContentSize", &ImGui::SetNextWindowContentSize);
		m.set("SetNextWindowCollapsed", &ImGui::SetNextWindowCollapsed);
		m.set("SetNextWindowFocus", &ImGui::SetNextWindowFocus);
		m.set("SetNextWindowBgAlpha", &ImGui::SetNextWindowBgAlpha);
		m.set("SetCurrentWindowPos", &SetWindowPos1);
		m.set("SetCurrentWindowSize", &SetWindowSize1);
		m.set("SetCurrentWindowCollapsed", &SetWindowCollapsed1);
		m.set("SetCurrentWindowFocus", &SetWindowFocus1);
		m.set("SetWindowFontScale", &ImGui::SetWindowFontScale);
		m.set("SetWindowPos", &SetWindowPos2);
		m.set("SetWindowSize", &SetWindowSize2);
		m.set("SetWindowCollapsed", &SetWindowCollapsed2);
		m.set("SetWindowFocus", &SetWindowFocus2);
		m.set("GetContentRegionMax", &ImGui::GetContentRegionMax);
		m.set("GetContentRegionAvail", &ImGui::GetContentRegionAvail);
		m.set("GetWindowContentRegionMin", &ImGui::GetWindowContentRegionMin);
		m.set("GetWindowContentRegionMax", &ImGui::GetWindowContentRegionMax);
		m.set("GetWindowContentRegionWidth", &ImGui::GetWindowContentRegionWidth);
		m.set("GetScrollX", &ImGui::GetScrollX);
		m.set("GetScrollY", &ImGui::GetScrollY);
		m.set("GetScrollMaxX", &ImGui::GetScrollMaxX);
		m.set("GetScrollMaxY", &ImGui::GetScrollMaxY);
		m.set("SetScrollX", &ImGui::SetScrollX);
		m.set("SetScrollY", &ImGui::SetScrollY);
		m.set("SetScrollHereX", &ImGui::SetScrollHereX);
		m.set("SetScrollHereY", &ImGui::SetScrollHereY);
		m.set("SetScrollFromPosX", &ImGui::SetScrollFromPosX);
		m.set("SetScrollFromPosY", &ImGui::SetScrollFromPosY);
		m.set("PushStyleColor", &PushStyleColor);
		m.set("PopStyleColor", &ImGui::PopStyleColor);
		m.set("PushStyleVarFloat", &PushStyleVarFloat);
		m.set("PushStyleVarVec2", &PushStyleVarVec2);
		m.set("GetStyleColorVec4", &ImGui::GetStyleColorVec4);
		m.set("GetStyleColorName", &ImGui::GetStyleColorName);
		m.set("GetFontSize", &ImGui::GetFontSize);
		m.set("PopStyleVar", &ImGui::PopStyleVar);
		m.set("PushItemWidth", &ImGui::PushItemWidth);
		m.set("PopItemWidth", &ImGui::PopItemWidth);
		m.set("SetNextItemWidth", &ImGui::SetNextItemWidth);
		m.set("CalcItemWidth", &ImGui::CalcItemWidth);
		m.set("PushTextWrapPos", &ImGui::PushTextWrapPos);
		m.set("PopTextWrapPos", &ImGui::PopTextWrapPos);
		m.set("PushAllowKeyboardFocus", &ImGui::PushAllowKeyboardFocus);
		m.set("PopAllowKeyboardFocus", &ImGui::PopAllowKeyboardFocus);
		m.set("PushButtonRepeat", &ImGui::PushButtonRepeat);
		m.set("PopButtonRepeat", &ImGui::PopButtonRepeat);
		m.set("Separator", ImGui::Separator);
		m.set("SameLine", ImGui::SameLine);
		m.set("NewLine", ImGui::NewLine);
		m.set("Spacing", ImGui::Spacing);
		m.set("Dummy", ImGui::Dummy);
		m.set("Indent", ImGui::Indent);
		m.set("Unindent", ImGui::Unindent);
		m.set("BeginGroup", ImGui::BeginGroup);
		m.set("EndGroup", ImGui::EndGroup);
		m.set("GetCursorPos", ImGui::GetCursorPos);
		m.set("GetCursorPosX", ImGui::GetCursorPosX);
		m.set("GetCursorPosY", ImGui::GetCursorPosY);
		m.set("SetCursorPos", ImGui::SetCursorPos);
		m.set("SetCursorPosX", ImGui::SetCursorPosX);
		m.set("SetCursorPosY", ImGui::SetCursorPosY);
		m.set("GetCursorStartPos", ImGui::GetCursorStartPos);
		m.set("GetCursorScreenPos", ImGui::GetCursorScreenPos);
		m.set("SetCursorScreenPos", ImGui::SetCursorScreenPos);
		m.set("AlignTextToFramePadding", ImGui::AlignTextToFramePadding);
		m.set("GetTextLineHeight", ImGui::GetTextLineHeight);
		m.set("GetTextLineHeightWithSpacing", ImGui::GetTextLineHeightWithSpacing);
		m.set("GetFrameHeight", ImGui::GetFrameHeight);
		m.set("GetFrameHeightWithSpacing", ImGui::GetFrameHeightWithSpacing);
		m.set("Text", &TextUnformatted);
		m.set("TextColored", &TextColored);
		m.set("TextDisabled", &TextDisabled);
		m.set("TextWrapped", &TextWrapped);
		m.set("LabelText", &LabelText);
		m.set("BulletText", &BulletText);
		m.set("Button", &ImGui::Button);
		m.set("SmallButton", &ImGui::SmallButton);
		m.set("InvisibleButton", &ImGui::InvisibleButton);
		m.set("ArrowButton", &ImGui::ArrowButton);
		m.set("Checkbox", &Checkbox);
		m.set("ProgressBar", &ImGui::ProgressBar);
		m.set("Bullet", &ImGui::Bullet);
		m.set("BeginCombo", &BeginCombo);
		m.set("EndCombo", &ImGui::EndCombo);
		m.set("Combo", &Combo);
		m.set("DragFloat", &DragFloat);
		m.set("DragFloat2", &DragFloat2);
		m.set("DragFloat3", &DragFloat3);
		m.set("DragFloat4", &DragFloat4);
		m.set("DragInt", &DragInt);
		m.set("DragFloatRange", &DragFloatRange);
		m.set("DragIntRange", &DragIntRange);
		m.set("SliderFloat", &SliderFloat);
		m.set("SliderFloat2", &SliderFloat2);
		m.set("SliderFloat3", &SliderFloat3);
		m.set("SliderFloat4", &SliderFloat4);
		m.set("SliderAngle", &SliderAngle);
		m.set("SliderInt", &SliderInt);
		m.set("VSliderFloat", &VSliderFloat);
		m.set("VSliderInt", &VSliderInt);
		m.set("InputText", &InputText);
		m.set("InputTextMultiline", &InputTextMultiline);
		m.set("InputTextWithHint", &InputTextWithHint);
		m.set("InputFloat", &InputFloat);
		m.set("InputFloat2", &InputFloat2);
		m.set("InputFloat3", &InputFloat3);
		m.set("InputFloat4", &InputFloat4);
		m.set("InputInt", &InputInt);
		m.set("ColorEdit3", &ColorEdit3);
		m.set("ColorEdit4", &ColorEdit4);
		m.set("ColorPicker3", &ColorPicker3);
		m.set("ColorPicker4", &ColorPicker4);
		m.set("ColorButton", &ColorButton);
		m.set("SetColorEditOptions", &SetColorEditOptions);
		m.set("TreeNode", &TreeNode);
		m.set("TreePush", &TreePush);
		m.set("TreePop", ImGui::TreePop);
		m.set("GetTreeNodeToLabelSpacing", &ImGui::GetTreeNodeToLabelSpacing);
		m.set("CollapsingHeader", &CollapsingHeader);
		m.set("SetNextItemOpen", &ImGui::SetNextItemOpen);
		m.set("Selectable", &Selectable);
		m.set("ListBox", &ListBox);
		m.set("ListBoxHeader", &ListBoxHeader);
		m.set("ListBoxHeader1", &ListBoxHeader1);
		m.set("ListBoxFooter", &ImGui::ListBoxFooter);
		m.set("PlotLines", &PlotLines);
		m.set("PlotHistogram", &PlotHistogram);
		m.set("BoolValue", &BoolValue);
		m.set("IntValue", &IntValue);
		m.set("UIntValue", &UIntValue);
		m.set("FloatValue", &FloatValue);
		m.set("BeginMainMenuBar", &ImGui::BeginMainMenuBar);
		m.set("EndMainMenuBar", &ImGui::EndMainMenuBar);
		m.set("BeginMenuBar", &ImGui::BeginMenuBar);
		m.set("EndMenuBar", &ImGui::EndMenuBar);
		m.set("BeginMenu", &ImGui::BeginMenu);
		m.set("EndMenu", &ImGui::EndMenu);
		m.set("MenuItem", &MenuItem);
		m.set("SetTooltip", &SetToolTip);
		m.set("BeginTooltip", &ImGui::BeginTooltip);
		m.set("EndTooltip", &ImGui::EndTooltip);
		m.set("OpenPopup", &ImGui::OpenPopup);
		m.set("BeginPopup", &BeginPopup);
		m.set("BeginPopupContextItem", &ImGui::BeginPopupContextItem);
		m.set("BeginPopupContextWindow", &ImGui::BeginPopupContextWindow);
		m.set("BeginPopupContextVoid", &ImGui::BeginPopupContextVoid);
		m.set("BeginPopupModal", &BeginPopupModal);
		m.set("EndPopup", &ImGui::EndPopup);
		m.set("OpenPopupOnItemClick", &ImGui::OpenPopupOnItemClick);
		m.set("IsPopupOpen", &ImGui::IsPopupOpen);
		m.set("CloseCurrentPopup", &ImGui::CloseCurrentPopup);
		m.set("Columns", &ImGui::Columns);
		m.set("NextColumn", &ImGui::NextColumn);
		m.set("GetColumnIndex", &ImGui::GetColumnIndex);
		m.set("GetColumnWidth", &ImGui::GetColumnWidth);
		m.set("SetColumnWidth", &ImGui::SetColumnWidth);
		m.set("GetColumnOffset", &ImGui::GetColumnOffset);
		m.set("SetColumnOffset", &ImGui::SetColumnOffset);
		m.set("GetColumnsCount", &ImGui::GetColumnsCount);
		m.set("BeginTabBar", &BeginTabBar);
		m.set("EndTabBar", &ImGui::EndTabBar);
		m.set("BeginTabItem", &BeginTabItem);
		m.set("EndTabItem", &ImGui::EndTabItem);
		m.set("SetTabItemClosed", &ImGui::SetTabItemClosed);
		m.set("LogToTTY", &ImGui::LogToTTY);
		m.set("LogToFile", &ImGui::LogToFile);
		m.set("LogToClipboard", &ImGui::LogToClipboard);
		m.set("LogFinish", &ImGui::LogFinish);
		m.set("LogButtons", &ImGui::LogButtons);
		m.set("LogText", &LogText);
		m.set("PushClipRect", &ImGui::PushClipRect);
		m.set("PopClipRect", &ImGui::PopClipRect);
		m.set("SetItemDefaultFocus", &ImGui::SetItemDefaultFocus);
		m.set("SetKeyboardFocusHere", &ImGui::SetKeyboardFocusHere);
		m.set("IsItemHovered", &IsItemHovered);
		m.set("IsItemActive", &ImGui::IsItemActive);
		m.set("IsItemFocused", &ImGui::IsItemFocused);
		m.set("IsItemClicked", &ImGui::IsItemClicked);
		m.set("IsItemVisible", &ImGui::IsItemVisible);
		m.set("IsItemEdited", &ImGui::IsItemEdited);
		m.set("IsItemActivated", &ImGui::IsItemActivated);
		m.set("IsItemDeactivated", &ImGui::IsItemDeactivated);
		m.set("IsItemDeactivatedAfterEdit", &ImGui::IsItemDeactivatedAfterEdit);
		m.set("IsAnyItemHovered", &ImGui::IsAnyItemHovered);
		m.set("IsAnyItemActive", &ImGui::IsAnyItemActive);
		m.set("IsAnyItemFocused", &ImGui::IsAnyItemFocused);
		m.set("GetItemRectMin", &ImGui::GetItemRectMin);
		m.set("GetItemRectMax", &ImGui::GetItemRectMax);
		m.set("GetItemRectSize", &ImGui::GetItemRectSize);
		m.set("SetItemAllowOverlap", &ImGui::SetItemAllowOverlap);
		m.set("IsRectVisible", &IsRectVisible);
		m.set("GetTime", &ImGui::GetTime);
		m.set("GetFrameCount", &ImGui::GetFrameCount);
		m.set("CalcTextSize", &ImGui::CalcTextSize);
		m.set("CalcListClipping", &CalcListClipping);
		m.set("BeginChildFrame", &ImGui::BeginChildFrame);
		m.set("EndChildFrame", &ImGui::EndChildFrame);
		m.set("ColorConvertU32ToFloat4", &ImGui::ColorConvertU32ToFloat4);
		m.set("ColorConvertFloat4ToU32", &ImGui::ColorConvertFloat4ToU32);
		m.set("GetKeyIndex", &ImGui::GetKeyIndex);
		m.set("IsKeyDown", &ImGui::IsKeyDown);
		m.set("IsKeyPressed", &ImGui::IsKeyPressed);
		m.set("IsKeyReleased", &ImGui::IsKeyReleased);
		m.set("GetKeyPressedAmount", &ImGui::GetKeyPressedAmount);
		m.set("IsMouseDown", &ImGui::IsMouseDown);
		m.set("IsAnyMouseDown", &ImGui::IsAnyMouseDown);
		m.set("IsMouseClicked", &ImGui::IsMouseClicked);
		m.set("IsMouseDoubleClicked", &ImGui::IsMouseDoubleClicked);
		m.set("IsMouseReleased", &ImGui::IsMouseReleased);
		m.set("IsMouseDragging", &ImGui::IsMouseDragging);
		m.set("IsMouseHoveringRect", &ImGui::IsMouseHoveringRect);
		m.set("IsMousePosValid", &IsMousePosValid);
		m.set("GetMousePos", &ImGui::GetMousePos);
		m.set("GetMousePosOnOpeningCurrentPopup", &ImGui::GetMousePosOnOpeningCurrentPopup);
		m.set("GetMouseDragDelta", &ImGui::GetMouseDragDelta);
		m.set("ResetMouseDragDelta", &ImGui::ResetMouseDragDelta);
		m.set("GetMouseCursor", &ImGui::GetMouseCursor);
		m.set("SetMouseCursor", &ImGui::SetMouseCursor);
		m.set("CaptureKeyboardFromApp", &ImGui::CaptureKeyboardFromApp);
		m.set("CaptureMouseFromApp", &ImGui::CaptureMouseFromApp);
		m.set("GetClipboardText", &ImGui::GetClipboardText);
		m.set("SetClipboardText", &ImGui::SetClipboardText);
		m.set("LoadIniSettingsFromDisk", &ImGui::LoadIniSettingsFromDisk);
		m.set("LoadIniSettingsFromMemory", &ImGui::LoadIniSettingsFromMemory);
		m.set("SaveIniSettingsToDisk", &ImGui::SaveIniSettingsToDisk);
		m.set("SaveIniSettingsToMemory", &SaveIniSettingsToMemory);
		m.set("TreeAdvanceToLabelPos", &ImGui::TreeAdvanceToLabelPos);
		m.set("SetNextTreeNodeOpen", &ImGui::SetNextTreeNodeOpen);
		m.set("GetContentRegionAvailWidth", &ImGui::GetContentRegionAvailWidth);
		m.set("SetScrollHere", &ImGui::SetScrollHere);
		m.set("IsItemDeactivatedAfterChange", &ImGui::IsItemDeactivatedAfterChange);
		m.set("IsAnyWindowFocused", &ImGui::IsAnyWindowFocused);
		m.set("IsAnyWindowHovered", &ImGui::IsAnyWindowHovered);
		m.set("CalcItemRectClosestPoint", &ImGui::CalcItemRectClosestPoint);
		m.set("ShowTestWindow", &ImGui::ShowTestWindow);
		m.set("IsRootWindowFocused", &ImGui::IsRootWindowFocused);
		m.set("IsRootWindowOrAnyChildFocused", &ImGui::IsRootWindowOrAnyChildFocused);
		m.set("SetNextWindowContentWidth", &ImGui::SetNextWindowContentWidth);
		m.set("GetItemsLineHeightWithSpacing", &ImGui::GetItemsLineHeightWithSpacing);
		m.set("IsRootWindowOrAnyChildHovered", &ImGui::IsRootWindowOrAnyChildHovered);
		m.set("AlignFirstTextHeightToWidgets", &ImGui::AlignFirstTextHeightToWidgets);
		m.set("SetNextWindowPosCenter", &ImGui::SetNextWindowPosCenter);

		isolate->GetCurrentContext()->Global()->Set(v8str("ImGui"), m.new_instance());
	}

	void release_imgui_objects() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		//detail::classes::find<raw_ptr_traits>(i, detail::type_id<ImColor>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<ListClippingResult>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<ImGuiSizeCallbackData>()).remove_objects();
		//detail::classes::find<raw_ptr_traits>(i, detail::type_id<ImVec4>()).remove_objects();
		//detail::classes::find<raw_ptr_traits>(i, detail::type_id<ImVec2>()).remove_objects();
	}

	void reset_imgui_object_storage() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		//detail::classes::find<raw_ptr_traits>(i, detail::type_id<ImColor>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<ListClippingResult>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<ImGuiSizeCallbackData>()).reset_objects_map();
		//detail::classes::find<raw_ptr_traits>(i, detail::type_id<ImVec4>()).reset_objects_map();
		//detail::classes::find<raw_ptr_traits>(i, detail::type_id<ImVec2>()).reset_objects_map();
	}
};