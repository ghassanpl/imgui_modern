#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <ghassanpl/with_sl.h>
#include <ghassanpl/align.h>
#include <ghassanpl/enum_flags.h>
#include <chrono>
#include <span>
#include <functional>
#include <magic_enum.hpp>
#include "imgui_same.h"

namespace ghassanpl::ig
{
    inline void HelpMarker(std::string_view desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

	template <typename T>
	constexpr ImGuiDataType ScalarTypeFor()
	{
		if constexpr (std::is_same_v<uint8_t, std::remove_cvref_t<T>>) return ImGuiDataType_U8;
		if constexpr (std::is_same_v<int8_t, std::remove_cvref_t<T>>) return ImGuiDataType_S8;
		if constexpr (std::is_same_v<uint16_t, std::remove_cvref_t<T>>) return ImGuiDataType_U16;
		if constexpr (std::is_same_v<int16_t, std::remove_cvref_t<T>>) return ImGuiDataType_S16;
		if constexpr (std::is_same_v<uint32_t, std::remove_cvref_t<T>>) return ImGuiDataType_U32;
		if constexpr (std::is_same_v<int32_t, std::remove_cvref_t<T>>) return ImGuiDataType_S32;
		if constexpr (std::is_same_v<uint64_t, std::remove_cvref_t<T>>) return ImGuiDataType_U64;
		if constexpr (std::is_same_v<int64_t, std::remove_cvref_t<T>>) return ImGuiDataType_S64;
		if constexpr (std::is_same_v<float, std::remove_cvref_t<T>>) return ImGuiDataType_Float;
		if constexpr (std::is_same_v<double, std::remove_cvref_t<T>>) return ImGuiDataType_Double;
	}

	enum class InputTextFlags
	{
		CharsDecimal,
		CharsHexadecimal,
		CharsUppercase,
		CharsNoBlank,
		AutoSelectAll,
		EnterReturnsTrue,
		CallbackCompletion,
		CallbackHistory,
		CallbackAlways,
		CallbackCharFilter,
		AllowTabInput,
		CtrlEnterForNewLine,
		NoHorizontalScroll,
		AlwaysOverwrite,
		ReadOnly,
		Password,
		NoUndoRedo,
		CharsScientific,
		CallbackResize,
		CallbackEdit,
	};

	enum Change
	{
		NoChange,
		Changed,
		ChangedAndApplied
	};

	struct imgui_source_location_hasher
	{
		constexpr uint32_t operator()(const std::source_location& k) const
		{
			return crc32(std::string_view{ k.file_name() }) ^ k.line() ^ k.column();
		}
	};

	inline Change EditResult(bool result) { return ImGui::IsItemDeactivatedAfterEdit() ? ChangedAndApplied : (result ? Changed : NoChange); }

	using id_label = with_slh<std::string_view, imgui_source_location_hasher>;

	bool InputText(ImStrv label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data, size_t max_size);

	template <typename FUNC>
	Change InputText(id_label label, std::string& str, FUNC&& func, enum_flags<InputTextFlags> flags = {}, size_t max_size = 0)
	{
		ImGui::PushID(label.LocationHash);
		auto result = InputText(label.Object, &str, (ImGuiInputTextFlags)flags.bits, [](ImGuiInputTextCallbackData* data) {
			auto& func = *reinterpret_cast<FUNC*>(data->UserData);
			return func(*data);
		}, std::addressof(func), max_size);
		ImGui::PopID();
		return EditResult(result);
	}

	Change InputText(id_label label, std::string& str, enum_flags<InputTextFlags> flags = {}, size_t max_size = 0);

	template <typename... ARGS>
	auto Button(id_label label, ARGS&&... args)
	{
		ImGui::PushID(label.LocationHash);
		auto result = ImGui::Button(label.Object, std::forward<ARGS>(args)...);
		ImGui::PopID();
		return result;
	}

	template <typename... ARGS>
	void Text(const std::format_string<ARGS...> fmt, ARGS&&... args)
	{
		auto s = std::format(fmt, std::forward<ARGS>(args)...);
		ImGui::TextUnformatted(ImStrv{ s });
	}

	template <typename... ARGS>
	void TextRight(const std::format_string<ARGS...> fmt, ARGS&&... args)
	{
		auto s = std::format(fmt, std::forward<ARGS>(args)...);
		auto w = ImGui::CalcTextSize(s).x;

		auto posX = ImGui::GetContentRegionMax().x - (w + ImGui::GetStyle().ItemSpacing.x);
		ImGui::SetCursorPosX(posX);
		ImGui::TextUnformatted(ImStrv{ s });
	}


	template <typename... ARGS>
	void Text(ImVec4 const& color, std::string_view fmt, ARGS&&... args)
	{
		auto s = std::vformat(fmt, std::make_format_args(std::forward<ARGS>(args)...));
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::TextUnformatted(ImStrv{ s });
		ImGui::PopStyleColor();
	}

	inline ImRect GetWindowContentRegion()
	{
		return { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };
	}
	inline float GetRemainingX()
	{
		return (ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX());
	}

	//bool SameLineIfRoom(float offset);
	bool WrapIfNoRoomFor(float min_width);
	bool WrapIfNoRoomFor(std::string_view label, float plus = 0.0f);

	template <typename... ARGS>
	void SetItemTooltip(std::format_string<ARGS...> fmt, ARGS&&... args)
	{
		auto s = std::format(fmt, std::forward<ARGS>(args)...);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
		{
			if (!ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
				return;
			ImGui::TextUnformatted(ImStrv{ s });
			ImGui::EndTooltip();
		}
	}

	void BeginGroupPanel(std::string_view name, const ImVec2& size = ImVec2(0.0f, 0.0f));

	void EndGroupPanel();

	/// TODO: Custom content (as a callback function)
	/// TODO: OnClick callback
	/// TODO: Close button
	/// TODO: OnDismiss callback
	/// TODO: Maybe just a general event callback

	/*
	enum class ToastType
	{
		Normal,
		Success,
		Info,
		Warning,
		Error,
		COUNT
	};

	enum class ToastPhase
	{
		FadeIn,
		Wait,
		FadeOut,
		Expired,
		COUNT
	};

	inline ImVec2 ToastPadding{ 20.0f, 20.0f };
	inline float ToastSpacing = 10.0f;

	using namespace std::chrono_literals;

	struct Toast
	{
		using clock_type = std::chrono::system_clock;
		using timepoint = clock_type::time_point;
		ToastType type = ToastType::Normal;
		std::string icon;
		std::string title;
		std::function<bool(Toast const&)> content_func;
		std::string content_string;
		std::chrono::duration<double> dismiss_time = 3s;
		std::chrono::system_clock::time_point phase_start = std::chrono::system_clock::now();
		ghassanpl::align ScreenAlignment = {};

		std::chrono::duration<double> FadeInOutTime = 150ms;			// Fade in and out duration
		std::chrono::duration<double> DefaultDismissTime = 3000ms;		// Auto dismiss after X ms (default, applied only of no data provided in constructors)
		double Opacity = 1.0;
		bool Separator = true;

		int WindowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration
		/// | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav
			| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;

		void SetPhase(ToastPhase phase) noexcept
		{
			phase_start = mTime;
			mPhase = phase;
		}

		ToastPhase Phase() const noexcept { return mPhase; }

		static std::string DefaultIconFor(ToastType type)
		{
			switch (type)
			{
			case ToastType::Normal: return {};
			case ToastType::Success: return "\xef\x81\x98";
			case ToastType::Warning: return "\xef\x81\xb1";
			case ToastType::Error: return "\xef\x81\x97";
			case ToastType::Info: return "\xef\x81\x9a";
			}
		}

		inline auto DefaultTitle() const noexcept -> std::string_view
		{
			if (title.empty())
			{
				switch (this->type)
				{
				case ToastType::Normal: return "";
				case ToastType::Success: return "Success";
				case ToastType::Warning: return "Warning";
				case ToastType::Error: return "Error";
				case ToastType::Info: return "Info";
				}
			}

			return this->title;
		};

		inline auto Color() const noexcept -> ImVec4
		{
			switch (this->type)
			{
			case ToastType::Normal: return { 1, 1, 1, 1 }; // White
			case ToastType::Success: return { 0, 1, 0, 1 }; // Green
			case ToastType::Warning: return { 1, 1, 0, 1 }; // Yellow
			case ToastType::Error: return { 1, 0, 0, 1 }; // Error
			case ToastType::Info: return { 0, 157.0f / 255.0f, 1, 1 }; // Blue
			}
			return {};
		}

		inline auto PhaseTime() const noexcept { return mTime - this->phase_start; }

		inline void Update(timepoint now)  noexcept
		{
			mTime = now;

			const auto elapsed = PhaseTime();

			switch (mPhase)
			{
			case ToastPhase::FadeIn:
				if (elapsed > FadeInOutTime)
					SetPhase(ToastPhase::Wait);
				break;
			case ToastPhase::Wait:
				if (elapsed > this->dismiss_time)
					SetPhase(ToastPhase::FadeOut);
				break;
			case ToastPhase::FadeOut:
				if (elapsed > FadeInOutTime)
					SetPhase(ToastPhase::Expired);
				break;
			case ToastPhase::Expired:
				break;
			}
		}

		inline auto FadePercent() const noexcept -> double
		{
			const auto elapsed = PhaseTime();

			if (mPhase == ToastPhase::FadeIn)
				return (elapsed / FadeInOutTime) * Opacity;
			else if (mPhase == ToastPhase::FadeOut)
				return (1.0 - (elapsed / FadeInOutTime)) * Opacity;

			return Opacity;
		}

		//private:

		timepoint mTime{};
		ToastPhase mPhase = ToastPhase::FadeIn;
	};
	/// <summary>
	/// Insert a new toast in the list
	/// </summary>
	size_t PushToast(Toast toast);

	/// <summary>
	/// Remove a toast from the list by its index
	/// </summary>
	/// <param name="index">index of the toast to remove</param>
	void RemoveToast(size_t index);

	/// <summary>
	/// Render toasts, call at the end of your rendering!
	/// </summary>
	void RenderToasts();
	*/

	bool VerticalToolbar(std::span<std::pair<std::string_view, std::string_view> const> const items, int& item_index);

    inline void ValueColumn(ImStrv label, std::string_view desc = {}, float fract = 0.15f)
	{
        if (label.empty()) return;

		auto rx = ig::GetRemainingX();
		ImGui::TextUnformatted(label);
        if (!desc.empty() && desc != "null")
        {
            ig::SameLine(0);
            if (ImGui::BeginItemTooltip())
            {
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            HelpMarker(desc);
        }
		ig::SameLine(rx * fract + ImGui::GetCurrentWindowRead()->DC.Indent.x);
	}

	template <typename T>
	requires std::is_enum_v<T>
	bool EnumBox(ImStrv label, T& eval, const ImVec2& size = { 0, 0 })
	{
		bool result = false;
		ValueColumn(label);
		if (ImGui::BeginListBox("", size))
		{
			for (auto& [value, name] : magic_enum::enum_entries<T>())
			{
				if (ImGui::Selectable(name, eval == value))
				{
					eval = value;
					result = true;
				}
			}
			ImGui::EndListBox();
		}
		return result;
	}

	template <typename T>
	bool EnumCombo(ImStrv label, T& eval, const ImVec2& size = { 0, 0 })
	{
		bool result = false;

        ImGui::PushID(&eval);
	    ValueColumn(label);

		if (ImGui::BeginCombo("", magic_enum::enum_name(eval)))
		{
			for (auto& [value, name] : magic_enum::enum_entries<T>())
			{
				if (ImGui::Selectable(name, eval == value))
				{
					eval = value;
					result = true;
				}
			}
			ImGui::EndCombo();
		}
        ImGui::PopID();
		return result;
	}

	template <typename E>
	bool EnumSelect(ImStrv label, E& eval, const ImVec2& size = { 0, 0 })
	{
        ValueColumn(label);

		bool result = false;
		bool first = true;
		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 6.0f, 2.0f });
		for (auto& [value, name] : magic_enum::enum_entries<E>())
		{
			/*
			if (ImGui::RadioButton(name, eval == value))
			{
				eval = value;
				result = true;
			}
			*/
			//if (!std::exchange(first, false))
				WrapIfNoRoomFor(name);
			auto w = ImGui::CalcTextSize(name).x + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().FramePadding.x * 2;
			if (ig::Selectable(name, eval == value, ImGuiSelectableFlags_NoPadWithHalfSpacing, { w, 0 }))
			{
				eval = value;
				result = true;
			}
		}
		ImGui::PopStyleVar(2);
		return result;
	}

	template <typename E, typename V>
	bool FlagCheckboxes(ImStrv label, enum_flags<E, V>& eval, const ImVec2& size = { 0, 0 })
	{
        ValueColumn(label);

		bool result = false, first = true;
		for (auto& [value, name] : magic_enum::enum_entries<E>())
		{
			//if (!std::exchange(first, false))
				WrapIfNoRoomFor(name, ImGui::GetFrameHeight());
			bool current = eval.contains(value);
			if (ImGui::Checkbox(name, &current))
			{
				eval.set_to(current, value);
				result = true;
			}
		}
		return result;
	}

	template <typename E, typename I>
	requires (std::is_integral_v<I> && !std::is_enum_v<I>)
	bool FlagCheckboxes(ImStrv label, I& eval, const ImVec2& size = { 0, 0 })
	{
        ValueColumn(label);

		bool result = false, first = true;
		for (auto& [value, name] : magic_enum::enum_entries<E>())
		{
			//if (!std::exchange(first, false))
				WrapIfNoRoomFor(name, ImGui::GetFrameHeight());
			bool current = bool(eval & (I(1) << I(value)));
			if (ImGui::Checkbox(name, &current))
			{
				if (current)
					eval |= (I(1) << I(value));
				else
					eval &= ~(I(1) << I(value));

				result = true;
			}
		}
		return result;
	}

	bool TextInputComboBox(ImStrv id, std::string& str, std::span<std::string_view> items, short showMaxItems = 0);

	inline bool SmallButton(ImStrv label, float width)
	{
		ImGuiContext& g = *GImGui;
		float backup_padding_y = g.Style.FramePadding.y;
		g.Style.FramePadding.y = 0.0f;
		bool pressed = ImGui::ButtonEx(label, ImVec2(width, 0), ImGuiButtonFlags_AlignTextBaseLine);
		g.Style.FramePadding.y = backup_padding_y;
		return pressed;
	}

	//bool ImageButtonWithText(std::function<std::shared_ptr<Texture>(intptr_t)> const& texture_getter, intptr_t arg, ImStrv label, const ImVec2& imageSize = ImVec2(0, 0), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));


#define IMGUI_DELETE_MOVE_COPY(Base)                             \
		Base(Base&&) = delete;                \
		Base &operator=(Base&&) = delete;     \
		Base(const Base&) = delete;           \
		Base& operator=(const Base&) = delete 

	struct Window
	{
		bool IsContentVisible;

		Window(ImStrv name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) { IsContentVisible = ImGui::Begin(name, p_open, flags); }
		~Window() { ImGui::End(); }

		explicit operator bool() const { return IsContentVisible; }

		IMGUI_DELETE_MOVE_COPY(Window);
	};

	struct Child
	{
		bool IsContentVisible;

		Child(ImStrv str_id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0) {
            if ((IsContentVisible = ImGui::BeginChild(str_id, size, child_flags, window_flags)))
                ImGui::PushItemWidth(ImGui::GetCurrentWindowRead()->ContentSize.x * 0.85f - ImGui::GetStyle().ItemSpacing.x * 2);
        }
		Child(ImGuiID id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0) {
            if ((IsContentVisible = ImGui::BeginChild(id, size, child_flags, window_flags)))
                ImGui::PushItemWidth(ImGui::GetCurrentWindowRead()->ContentSize.x * 0.85f - ImGui::GetStyle().ItemSpacing.x * 2);
        }
		~Child() {
            if (IsContentVisible)
                ImGui::PopItemWidth();
            ImGui::EndChild();
        }

		explicit operator bool() const { return IsContentVisible; }

		IMGUI_DELETE_MOVE_COPY(Child);
	};

	struct List
	{
		bool IsContentVisible;

		List(ImStrv str_id, const ImVec2& size = ImVec2(0, 0)) { IsContentVisible = ImGui::BeginListBox(str_id, size); }
		~List() { if (IsContentVisible) ImGui::EndListBox(); }

		explicit operator bool() const { return IsContentVisible; }

		IMGUI_DELETE_MOVE_COPY(List);
	};

	struct Font
	{
		Font(ImFont* font) { ImGui::PushFont(font); }
		~Font() { ImGui::PopFont(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(Font);
	};

	struct StyleColor
	{
		StyleColor(ImGuiCol idx, ImU32 col) { ImGui::PushStyleColor(idx, col); }
		StyleColor(ImGuiCol idx, const ImVec4& col) { ImGui::PushStyleColor(idx, col); }
		~StyleColor() { ImGui::PopStyleColor(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(StyleColor);
	};

	struct StyleVar
	{
		StyleVar(ImGuiStyleVar idx, float val) { ImGui::PushStyleVar(idx, val); }
		StyleVar(ImGuiStyleVar idx, const ImVec2& val) { ImGui::PushStyleVar(idx, val); }
		~StyleVar() { ImGui::PopStyleVar(); }
		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(StyleVar);
	};

	struct ItemWidth
	{
		ItemWidth(float item_width) { ImGui::PushItemWidth(item_width); }
		~ItemWidth() { ImGui::PopItemWidth(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(ItemWidth);
	};

	struct TextWrapPos
	{
		TextWrapPos(float wrap_pos_x = 0.0f) { ImGui::PushTextWrapPos(wrap_pos_x); }
		~TextWrapPos() { ImGui::PopTextWrapPos(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(TextWrapPos);
	};

	struct AllowKeyboardFocus
	{
		AllowKeyboardFocus(bool allow_keyboard_focus) { ImGui::PushAllowKeyboardFocus(allow_keyboard_focus); }
		~AllowKeyboardFocus() { ImGui::PopAllowKeyboardFocus(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(AllowKeyboardFocus);
	};

	struct ButtonRepeat
	{
		ButtonRepeat(bool repeat) { ImGui::PushButtonRepeat(repeat); }
		~ButtonRepeat() { ImGui::PopButtonRepeat(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(ButtonRepeat);
	};

	struct Group
	{
		Group() { ImGui::BeginGroup(); }
		~Group() { ImGui::EndGroup(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(Group);
	};

	struct ID
	{
		ID(ImStrv str_id) { ImGui::PushID(str_id); }
		ID(const void* ptr_id) { ImGui::PushID(ptr_id); }
		ID(int int_id) { ImGui::PushID(int_id); }
		~ID() { ImGui::PopID(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(ID);
	};

	struct Combo
	{
		bool IsOpen;

		Combo(ImStrv label, ImStrv preview_value, ImGuiComboFlags flags = 0) { IsOpen = ImGui::BeginCombo(label, preview_value, flags); }
		~Combo() { if (IsOpen) ImGui::EndCombo(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(Combo);
	};

	struct TreeNode
	{
		bool IsOpen;

		TreeNode(ImStrv label) { IsOpen = ImGui::TreeNode(label); }
		~TreeNode() { if (IsOpen) ImGui::TreePop(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(TreeNode);
	};

	struct TreeNodeEx
	{
		bool IsOpen;

		TreeNodeEx(ImStrv label, ImGuiTreeNodeFlags flags = 0) { IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)); IsOpen = ImGui::TreeNodeEx(label, flags); }
		~TreeNodeEx() { if (IsOpen) ImGui::TreePop(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(TreeNodeEx);
	};

	struct MainMenuBar
	{
		bool IsOpen;

		MainMenuBar() { IsOpen = ImGui::BeginMainMenuBar(); }
		~MainMenuBar() { if (IsOpen) ImGui::EndMainMenuBar(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(MainMenuBar);
	};

	struct MenuBar
	{
		bool IsOpen;

		MenuBar() { IsOpen = ImGui::BeginMenuBar(); }
		~MenuBar() { if (IsOpen) ImGui::EndMenuBar(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(MenuBar);
	};

	struct Menu
	{
		bool IsOpen;

		Menu(ImStrv label, bool enabled = true) { IsOpen = ImGui::BeginMenu(label, enabled); }
		~Menu() { if (IsOpen) ImGui::EndMenu(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(Menu);
	};

	struct TabBar
	{
		bool IsOpen;

		TabBar(ImStrv str_id, ImGuiTabBarFlags flags = 0) { IsOpen = ImGui::BeginTabBar(str_id, flags); }
		~TabBar() { if (IsOpen) ImGui::EndTabBar(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(TabBar);
	};

	struct TabItem
	{
		bool IsOpen;

		TabItem(ImStrv label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0) { IsOpen = ImGui::BeginTabItem(label, p_open, flags); }
		~TabItem() { if (IsOpen) ImGui::EndTabItem(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(TabItem);
	};

	struct Tooltip
	{
		Tooltip() { ImGui::BeginTooltip(); }
		~Tooltip() { ImGui::EndTooltip(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(Tooltip);
	};

	struct Popup
	{
		bool IsOpen;

		Popup(ImStrv str_id, ImGuiWindowFlags flags = 0) { IsOpen = ImGui::BeginPopup(str_id, flags); }
		~Popup() { if (IsOpen) ImGui::EndPopup(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(Popup);
	};

	struct PopupContextItem
	{
		bool IsOpen;

		PopupContextItem(ImStrv str_id = {}, int mouse_button = 1) { IsOpen = ImGui::BeginPopupContextItem(str_id, mouse_button); }
		~PopupContextItem() { if (IsOpen) ImGui::EndPopup(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(PopupContextItem);
	};

	struct PopupContextWindow
	{
		bool IsOpen;

		PopupContextWindow(ImStrv str_id = {}, ImGuiPopupFlags flags = 1) { IsOpen = ImGui::BeginPopupContextWindow(str_id, flags); }
		~PopupContextWindow() { if (IsOpen) ImGui::EndPopup(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(PopupContextWindow);
	};

	struct PopupContextVoid
	{
		bool IsOpen;

		PopupContextVoid(ImStrv str_id = {}, int mouse_button = 1) { IsOpen = ImGui::BeginPopupContextVoid(str_id, mouse_button); }
		~PopupContextVoid() { if (IsOpen) ImGui::EndPopup(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(PopupContextVoid);
	};

	struct PopupModal
	{
		bool IsOpen;

		PopupModal(ImStrv name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) { IsOpen = ImGui::BeginPopupModal(name, p_open, flags); }
		~PopupModal() { if (IsOpen) ImGui::EndPopup(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(PopupModal);
	};

	struct DragDropSource
	{
		bool IsOpen;

		DragDropSource(ImGuiDragDropFlags flags = 0) { IsOpen = ImGui::BeginDragDropSource(flags); }
		~DragDropSource() { if (IsOpen) ImGui::EndDragDropSource(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(DragDropSource);
	};

	struct DragDropTarget
	{
		bool IsOpen;

		DragDropTarget() { IsOpen = ImGui::BeginDragDropTarget(); }
		~DragDropTarget() { if (IsOpen) ImGui::EndDragDropTarget(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(DragDropTarget);
	};

	struct ClipRect
	{
		ClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect) { ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect); }
		~ClipRect() { ImGui::PopClipRect(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(ClipRect);
	};

	struct ChildFrame
	{
		bool IsOpen;

		ChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags flags = 0) { IsOpen = ImGui::BeginChildFrame(id, size, flags); }
		~ChildFrame() { ImGui::EndChildFrame(); }

		explicit operator bool() const { return IsOpen; }

		IMGUI_DELETE_MOVE_COPY(ChildFrame);
	};


	struct Disabled
	{
		bool IsDisabled;

		Disabled(bool disabled) : IsDisabled(disabled) { if (disabled) ImGui::BeginDisabled(); }
		~Disabled() { if (IsDisabled) ImGui::EndDisabled(); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(Disabled);
	};

	struct Indent
	{
		float W = 0.0f;
		Indent(float w = 0.0f) : W(w) { ImGui::Indent(w); }
		~Indent() { ImGui::Unindent(W); }

		explicit operator bool() const { return true; }

		IMGUI_DELETE_MOVE_COPY(Indent);
	};


#undef IMGUI_DELETE_MOVE_COPY
}
