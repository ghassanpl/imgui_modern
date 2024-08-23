#include "imgui_modern.h"

//#include <glm/vec2.hpp>

//using namespace glm;

namespace ghassanpl::ig
{
	struct InputTextCallback_UserData
	{
		std::string* Str;
		ImGuiInputTextCallback  ChainCallback;
		void* ChainCallbackUserData;
		size_t MaxSize = 0;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
			std::string* str = user_data->Str;
			IM_ASSERT(data->Buf == str->c_str());
			if (user_data->MaxSize != 0 && data->BufTextLen > user_data->MaxSize)
			{
				data->BufTextLen = user_data->MaxSize;
				data->BufSize = data->BufTextLen + 1;
				return 1;
			}
			str->resize(data->BufTextLen);
			data->Buf = str->data();
		}
        else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
        {
            while (!user_data->Str->empty() && user_data->Str->ends_with('\0'))
                user_data->Str->pop_back();
        }
		else if (user_data->ChainCallback)
		{
			// Forward to user callback, if any
			data->UserData = user_data->ChainCallbackUserData;
			return user_data->ChainCallback(data);
		}
		return 0;
	}

    IMGUI_API bool InputText(ImStrv label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data, size_t max_size)
    {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        if (max_size == 0 || str->size() < max_size)
            flags |= ImGuiInputTextFlags_CallbackResize;
        flags |= ImGuiInputTextFlags_CallbackEdit;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		cb_user_data.MaxSize = max_size;
		if (max_size > 0)
			return ImGui::InputText(label, str->data(), std::min(str->capacity() + 1, max_size + 1), flags, InputTextCallback, &cb_user_data);
		else
			return ImGui::InputText(label, str->data(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}

	Change InputText(id_label label, std::string& str, enum_flags<InputTextFlags> flags, size_t max_size)
	{
		ImGui::PushID(label.LocationHash);
		ValueColumn(label.Object);
		auto result = InputText("", &str, (ImGuiInputTextFlags)flags.bits, nullptr, nullptr, max_size);
		ImGui::PopID();
		return EditResult(result);
	}

	bool WrapIfNoRoomFor(float min_width)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		const auto cpx = (window->DC.CursorPosPrevLine.x + g.Style.ItemSpacing.x) - window->Pos.x + window->Scroll.x;
		//const auto cpx = ImGui::GetCursorPosX();
		const auto crm = ImGui::GetContentRegionMax().x;
		if (cpx > crm - min_width)
		{
			if (window->DC.IsSameLine)
				NewLine();
			return false;
		}
		if (!window->DC.IsSameLine)
			ig::SameLine(cpx);
		return true;
	}

	bool WrapIfNoRoomFor(std::string_view label, float plus)
	{
		return WrapIfNoRoomFor(ImGui::CalcTextSize(label.data()).x + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().FramePadding.x * 2 + plus);
	}

	static ImVector<ImRect> s_GroupPanelLabelStack;

	void BeginGroupPanel(std::string_view name, const ImVec2& size)
	{
		ImGui::BeginGroup();

		auto cursorPos = ImGui::GetCursorScreenPos();
		auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();
		ImGui::BeginGroup();

		ImVec2 effectiveSize = size;
		if (size.x < 0.0f)
			effectiveSize.x = ImGui::GetContentRegionAvail().x;
		else
			effectiveSize.x = size.x;
		ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextUnformatted(name);
		auto labelMin = ImGui::GetItemRectMin();
		auto labelMax = ImGui::GetItemRectMax();
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
		ImGui::BeginGroup();

		//ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x -= frameHeight;

		auto itemWidth = ImGui::CalcItemWidth();
		ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

		s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
	}

	void EndGroupPanel()
	{
		ImGui::PopItemWidth();

		auto itemSpacing = ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();

		//ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

		ImGui::EndGroup();

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

		ImGui::EndGroup();

		auto itemMin = ImGui::GetItemRectMin();
		auto itemMax = ImGui::GetItemRectMax();
		//ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

		auto labelRect = s_GroupPanelLabelStack.back();
		s_GroupPanelLabelStack.pop_back();

		ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
		ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
		labelRect.Min.x -= itemSpacing.x;
		labelRect.Max.x += itemSpacing.x;
		for (int i = 0; i < 4; ++i)
		{
			switch (i)
			{
				// left half-plane
			case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
				// right half-plane
			case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
				// top
			case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
				// bottom
			case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
			}

			ImGui::GetWindowDrawList()->AddRect(
				frameRect.Min, frameRect.Max,
				ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
				halfFrame.x);

			ImGui::PopClipRect();
		}

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x += frameHeight;

		ImGui::Dummy(ImVec2(0.0f, 0.0f));

		ImGui::EndGroup();
	}

	/*
	static std::vector<Toast> mToasts;

	/// <summary>
	/// Insert a new toast in the list
	/// </summary>
	size_t PushToast(Toast toast)
	{
		mToasts.push_back(std::move(toast));
		return mToasts.size() - 1;
	}


	/// <summary>
	/// Remove a toast from the list by its index
	/// </summary>
	/// <param name="index">index of the toast to remove</param>
	void RemoveToast(size_t index)
	{
		mToasts.erase(mToasts.begin() + index);
	}

	/// <summary>
	/// Render toasts, call at the end of your rendering!
	/// </summary>
	void RenderToasts()
	{
		using namespace ImGui;
		const auto vp_size = GetMainViewport()->Size;

		float height = 0.f;

		auto now = std::chrono::system_clock::now();

		for (auto i = 0; i < mToasts.size(); i++)
		{
			auto* current_toast = &mToasts[i];

			current_toast->Update(now);

			// Get icon, title and other data
			const auto& icon = current_toast->icon;
			const auto& title = current_toast->title;
			const auto& content_string = current_toast->content_string;
			const auto& content_func = current_toast->content_func;
			const auto has_content = !content_string.empty() || content_func;
			const auto default_title = current_toast->DefaultTitle();
			const auto opacity = (float)current_toast->FadePercent(); // Get opacity based of the current phase

			auto text_color = current_toast->Color();
			text_color.w = opacity;

			// Generate new unique name for this toast
			auto window_name = std::format("##TOAST{}", i);

			//PushStyleColor(ImGuiCol_Text, text_color);
			SetNextWindowBgAlpha(opacity);
			SetNextWindowPos(ImVec2(vp_size.x - ToastPadding.x, vp_size.y - ToastPadding.y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
			Begin(window_name.c_str(), NULL, current_toast->WindowFlags | ImGuiWindowFlags_NoSavedSettings);
			BringWindowToDisplayFront(ImGui::GetCurrentWindow()); // needs imgui_internal.h

			// Here we render the toast content
			{
				PushTextWrapPos(vp_size.x / 3.f); // We want to support multi-line text, this will wrap the text after 1/3 of the screen width

				bool was_title_rendered = false;

				// If an icon is set
				if (!icon.empty())
				{
					//Text(icon); // Render icon text
					TextColored(text_color, icon.c_str());
					was_title_rendered = true;
				}

				// If a title is set
				if (!title.empty())
				{
					// If a title and an icon is set, we want to render on same line
					if (!icon.empty())
						SameLine();

					TextUnformatted(ImStrv{ title }); // Render title text
					was_title_rendered = true;
				}
				else if (!default_title.empty())
				{
					if (!icon.empty())
						SameLine();

					Text(default_title.data(), default_title.data() + default_title.size()); // Render default title text (ImGuiToastType_Success -> "Success", etc...)
					was_title_rendered = true;
				}

				// In case ANYTHING was rendered in the top, we want to add a small padding so the text (or icon) looks centered vertically
				if (was_title_rendered && has_content)
				{
					SetCursorPosY(GetCursorPosY() + 5.f); // Must be a better way to do this!!!!
				}

				// If a content is set
				if (has_content)
				{
					if (was_title_rendered && current_toast->Separator)
						Separator();

					if (!content_string.empty())
						TextUnformatted(ImStrv{ content_string }); // Render content text
					else
					{
						bool close = content_func(*current_toast);
						if (close)
							current_toast->SetPhase(ToastPhase::Expired);
					}
				}

				PopTextWrapPos();
			}

			// Save height for next toasts
			height += GetWindowHeight() + ToastSpacing;

			// End
			End();
		}

		std::erase_if(mToasts, [](auto& toast) { return toast.Phase() == ToastPhase::Expired; });
	}
	*/

	bool VerticalToolbar(std::span<std::pair<std::string_view, std::string_view>const> const items, int& item_index)
	{
		bool value_changed = false;
		for (size_t i = 0; i < items.size(); ++i)
		{
			const auto selected = (i == item_index);
			if (ImGui::Selectable(items[i].second, selected))
			{
				item_index = (int)i;
				value_changed = true;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%.*s", (int)items[i].first.size(), items[i].first.data());
		}
		return value_changed;
	}

	static int propose(ImGuiInputTextCallbackData* data, std::span<std::string_view> items)
	{
		//We don't want to "preselect" anything
		if (strlen(data->Buf) == 0) return 0;

		//Get our items back
		//const char** items = static_cast<std::pair<const char**, size_t>*> (data->UserData)->first;
		//size_t length = static_cast<std::pair<const char**, size_t>*> (data->UserData)->second;

		/*
		//We need to give the user a chance to remove wrong input
		//We use SFML Keycodes here, because the Imgui Keys aren't working the way I thought they do...
		if (key == sf::Keyboard::BackSpace) { //TODO: Replace with imgui key
		//We delete the last char automatically, since it is what the user wants to delete, but only if there is something (selected/marked/hovered)
		//FIXME: This worked fine, when not used as helper function
		if (data->SelectionEnd != data->SelectionStart)
		if (data->BufTextLen > 0) //...and the buffer isn't empty
		if (data->CursorPos > 0) //...and the cursor not at pos 0
		data->DeleteChars(data->CursorPos - 1, 1);
		return 0;
		}
		if (key == sf::Keyboard::Key::Delete) return 0; //TODO: Replace with imgui key
		*/

		for (auto& item : items)
		{
			if (item.starts_with(data->Buf))
			{
				const int cursor = data->CursorPos;
				//Insert the first match
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, item);
				//Reset the cursor position
				data->CursorPos = cursor;
				//Select the text, so the user can simply go on writing
				data->SelectionStart = cursor;
				data->SelectionEnd = data->BufTextLen;
				break;
			}
		}
		return 0;
	}

	bool TextInputComboBox(ImStrv id, std::string& str, std::span<std::string_view> items, short showMaxItems)
	{
		//Check if both strings matches
		if (showMaxItems == 0)
			showMaxItems = items.size();

		ig::ID _(id);
		ValueColumn(id);
		ImGui::SetNextItemWidth(-32);
		bool ret = ig::InputText(id_label{ "##in" }, str,
			[&](ImGuiInputTextCallbackData& data) { return propose(&data, items); },
			{ InputTextFlags::CallbackAlways, InputTextFlags::EnterReturnsTrue });

		ImGui::OpenPopupOnItemClick("combobox"); //Enable right-click
		ImVec2 pos = ImGui::GetItemRectMin();
		ImVec2 size = ImGui::GetItemRectSize();

		ImGui::SameLine(0, 0);
		if (ImGui::ArrowButton("##openCombo", ImGuiDir_Down)) {
			ImGui::OpenPopup("combobox");
		}
		ImGui::OpenPopupOnItemClick("combobox"); //Enable right-click

		pos.y += size.y;
		size.x += ImGui::GetItemRectSize().x;
		size.y += 20 + (size.y * showMaxItems);
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		if (ImGui::BeginPopup("combobox", ImGuiWindowFlags_::ImGuiWindowFlags_NoMove)) {

			//ImGui::Text("Select one item or type");
			static std::string filter;
			ig::InputText("Filter", filter);
			ImGui::Separator();
			for (auto& item : items)
			{
				if (item.empty()) continue;
				if (ImGui::Selectable(item))
				{
					str.assign_range(item);
					ret = true;
				}
			}

			ImGui::EndPopup();
		}

		return ret;
	}

	/*
	bool ImageButtonWithText(std::function<std::shared_ptr<Texture>(intptr_t)> const& texture_getter, intptr_t arg, ImStrv label, const ImVec2& imageSize, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImVec2 size = imageSize;
		if (size.x <= 0 && size.y <= 0) { size.x = size.y = ImGui::GetTextLineHeightWithSpacing(); }
		else {
			if (size.x <= 0)          size.x = size.y;
			else if (size.y <= 0)     size.y = size.x;
			size *= window->FontWindowScale * ImGui::GetIO().FontGlobalScale;
		}

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		const ImGuiID id = window->GetID(label);
		const ImVec2 textSize = ImGui::CalcTextSize(label, true, size.x);
		const bool hasText = textSize.x > 0;

		const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
		const ImVec2 totalSizeWithoutPadding(std::max(size.x, textSize.x), size.y + innerSpacing + textSize.y);
		const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding * 2);

		ImVec2 start(0, 0);
		start = window->DC.CursorPos + padding;
		const rec2 image_bb(start, start + size);
		start = window->DC.CursorPos + padding;
		start.y += size.y + innerSpacing;

		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered = false, held = false;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		// Render
		const ImU32 col = ImGui::GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
		if (bg_col.w > 0.0f)
			window->DrawList->AddRectFilled(ImVec2{ image_bb.p1 }, ImVec2{ image_bb.p2 }, ImGui::GetColorU32(bg_col));

		auto tex = texture_getter(arg);
		if (tex)
		{
			auto centered = ghassanpl::aligned(ImVec2{ tex->Size }, image_bb, align::center);
			window->DrawList->AddImage((ImTextureID)tex->Handle, ImVec2 { centered.p1 }, ImVec2{ centered.p2 }, uv0, uv1, ImGui::GetColorU32(tint_col));
		}
		else
			window->DrawList->AddImage({}, ImVec2{ image_bb.p1 }, ImVec2{ image_bb.p2 }, uv0, uv1, 0);

		if (textSize.x > 0) ImGui::RenderTextWrapped(start, label, size.x);
		return pressed;
	}
	*/
}
