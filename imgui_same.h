namespace ghassanpl::ig
{
	using ImGui::SameLine;
	using ImGui::NewLine;
	using ImGui::SmallButton;
	using ImGui::CreateContext;
	using ImGui::GetStyle;
	using ImGui::GetIO;
	using ImGui::NewFrame;
	using ImGui::EndFrame;
	using ImGui::GetDrawData;
	using ImGui::Render;
	using ImGui::Separator;
	using ImGui::SeparatorText;
	using ImGui::Value;
	using ImGui::IsItemDeactivatedAfterEdit;
	using ImGui::PushStyleColor;
	using ImGui::PopStyleColor;
	using ImGui::ImageButton;
	using ImGui::Selectable;
	using ImGui::IsMouseDoubleClicked;
	using ImGui::IsPopupOpen;
	using ImGui::CloseCurrentPopup;
	using ImGui::CollapsingHeader;
	using ImGui::Image;
	/// TODO: Create wrappers for Input* that take references and return `Change`
	using ImGui::InputInt2;
	using ImGui::SliderInt;
	using ImGui::SliderInt2;
	using ImGui::InputDouble; /// TODO: change default format to "%g"
	using ImGui::ColorEdit4;
	using ImGui::Checkbox;
}
