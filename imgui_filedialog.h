/*
	Copyright 2020 Limeoats
	Original project: https://github.com/Limeoats/L2DFileDialog

	Changes by Vladimir Sigalkin
*/

#pragma once

#include <filesystem>
#include "imgui.h"
#include <string>
#include <vector>
#include <functional>

typedef int ImGuiFileDialogType;	// -> enum ImGuiFileDialogType_        // Enum: A file dialog type

enum ImGuiFileDialogType_
{
	ImGuiFileDialogType_OpenFile,
	ImGuiFileDialogType_SaveFile,
	ImGuiFileDialogType_COUNT
};

struct ImFileDialogInfo
{
	std::string title = "Open File";
	ImGuiFileDialogType type = ImGuiFileDialogType_OpenFile;

    std::filesystem::path fileName;
	std::filesystem::path directoryPath = std::filesystem::current_path();
	std::filesystem::path resultPath;

	bool refreshInfo;
	size_t currentIndex;
	std::vector<std::filesystem::directory_entry> currentFiles;
	std::vector<std::filesystem::directory_entry> currentDirectories;
	std::function<bool(std::filesystem::directory_entry const&)> fileFilterFunc = {};

    /// TODO: This
    std::vector<std::string> filters = { "All Files", "*" };

    std::function<void(std::filesystem::directory_entry const&)> fileActionCallback = {};

	void refreshPaths();
};

namespace ImGui
{
	IMGUI_API bool FileDialog(bool* open, ImFileDialogInfo* dialogInfo);
}
