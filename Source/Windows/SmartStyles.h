#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stack>
#include <imgui_internal.h>

//The goal is to convert a single color input into a theme for imgui

// Stack to store previous styles for restoration
static std::stack<ImGuiStyle> g_styleStack;

// Helper function to create color variations
inline ImVec4 CreateColorVariant(ImColor base, float brightnessMult, float saturationMult, float alpha = 1.0f) {
	float h, s, v;
	ImGui::ColorConvertRGBtoHSV(base.Value.x, base.Value.y, base.Value.z, h, s, v);

	v *= brightnessMult;
	s *= saturationMult;

	// Clamp values
	v = ImClamp(v, 0.0f, 1.0f);
	s = ImClamp(s, 0.0f, 1.0f);

	float r, g, b;
	ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);

	return ImVec4(r, g, b, alpha);
}

inline void PushSmartStyle(ImColor backgroundColor, ImColor foregroundColor) {
	// Store the current style
	g_styleStack.push(ImGui::GetStyle());

	ImVec4* colors = ImGui::GetStyle().Colors;

	// Base color variations - using separate background and foreground colors
	ImVec4 bgBase = backgroundColor.Value;
	ImVec4 fgBase = foregroundColor.Value;

	// Text colors
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

	// Background colors - using background color variants
	colors[ImGuiCol_WindowBg] = CreateColorVariant(backgroundColor, 0.35f, 0.8f, 0.54f);
	colors[ImGuiCol_ChildBg] = CreateColorVariant(backgroundColor, 0.35f, 0.8f, 0.54f);
	colors[ImGuiCol_PopupBg] = CreateColorVariant(backgroundColor, 0.15f, 0.9f, 0.94f);

	// Border colors - using foreground color
	colors[ImGuiCol_Border] = CreateColorVariant(foregroundColor, 0.6f, 1.0f, 0.50f);
	colors[ImGuiCol_BorderShadow] = CreateColorVariant(backgroundColor, 0.05f, 0.9f, 0.00f);

	// Frame colors - background based
	colors[ImGuiCol_FrameBg] = CreateColorVariant(backgroundColor, 0.35f, 0.8f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = CreateColorVariant(foregroundColor, 0.65f, 0.9f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = CreateColorVariant(foregroundColor, 0.75f, 1.0f, 0.67f);

	// Title colors - background based
	colors[ImGuiCol_TitleBg] = CreateColorVariant(backgroundColor, 0.12f, 0.9f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = CreateColorVariant(backgroundColor, 0.20f, 0.9f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

	// Menu colors - background based
	colors[ImGuiCol_MenuBarBg] = CreateColorVariant(backgroundColor, 0.15f, 0.9f, 1.00f);

	// Scrollbar colors - background base with foreground accents
	colors[ImGuiCol_ScrollbarBg] = CreateColorVariant(backgroundColor, 0.10f, 0.9f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = CreateColorVariant(foregroundColor, 0.30f, 0.8f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = CreateColorVariant(foregroundColor, 0.60f, 1.0f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = CreateColorVariant(foregroundColor, 0.80f, 1.0f, 1.00f);

	// Interactive element colors - foreground based
	colors[ImGuiCol_CheckMark] = CreateColorVariant(foregroundColor, 0.85f, 1.0f, 1.00f);
	colors[ImGuiCol_SliderGrab] = CreateColorVariant(foregroundColor, 0.70f, 0.9f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = CreateColorVariant(foregroundColor, 0.85f, 1.0f, 1.00f);

	// Button colors - foreground based
	colors[ImGuiCol_Button] = CreateColorVariant(foregroundColor, 0.70f, 0.9f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = CreateColorVariant(foregroundColor, 0.80f, 1.0f, 1.00f);
	colors[ImGuiCol_ButtonActive] = CreateColorVariant(foregroundColor, 0.65f, 0.9f, 1.00f);

	// Header colors - foreground based
	colors[ImGuiCol_Header] = CreateColorVariant(foregroundColor, 0.55f, 0.9f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = CreateColorVariant(foregroundColor, 0.70f, 0.9f, 0.80f);
	colors[ImGuiCol_HeaderActive] = CreateColorVariant(foregroundColor, 0.80f, 1.0f, 1.00f);

	// Separator colors - foreground based
	colors[ImGuiCol_Separator] = CreateColorVariant(foregroundColor, 0.40f, 0.8f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = CreateColorVariant(foregroundColor, 0.60f, 1.0f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = CreateColorVariant(foregroundColor, 0.70f, 1.0f, 1.00f);

	// Resize grip colors - foreground based
	colors[ImGuiCol_ResizeGrip] = CreateColorVariant(foregroundColor, 0.75f, 1.0f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = CreateColorVariant(foregroundColor, 0.85f, 1.0f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = CreateColorVariant(foregroundColor, 0.90f, 1.0f, 0.95f);

	// Text cursor
	colors[ImGuiCol_InputTextCursor] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

	// Tab colors - mixed background and foreground
	colors[ImGuiCol_TabHovered] = CreateColorVariant(foregroundColor, 0.80f, 1.0f, 0.80f);
	colors[ImGuiCol_Tab] = CreateColorVariant(backgroundColor, 0.45f, 0.8f, 0.86f);
	colors[ImGuiCol_TabSelected] = CreateColorVariant(foregroundColor, 0.58f, 0.9f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline] = CreateColorVariant(foregroundColor, 0.80f, 1.0f, 1.00f);
	colors[ImGuiCol_TabDimmed] = CreateColorVariant(backgroundColor, 0.15f, 0.7f, 0.97f);
	colors[ImGuiCol_TabDimmedSelected] = CreateColorVariant(backgroundColor, 0.35f, 0.8f, 1.00f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = CreateColorVariant(backgroundColor, 0.42f, 0.8f, 0.00f);

	// Docking colors - foreground based
	colors[ImGuiCol_DockingPreview] = CreateColorVariant(foregroundColor, 0.85f, 1.0f, 0.70f);
	colors[ImGuiCol_DockingEmptyBg] = CreateColorVariant(backgroundColor, 0.15f, 0.8f, 0.00f);

	// Plot colors - foreground based
	colors[ImGuiCol_PlotLines] = CreateColorVariant(foregroundColor, 0.90f, 1.0f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = CreateColorVariant(foregroundColor, 1.00f, 1.0f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = CreateColorVariant(foregroundColor, 0.70f, 1.0f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = CreateColorVariant(foregroundColor, 0.85f, 1.0f, 1.00f);

	// Table colors - background based
	colors[ImGuiCol_TableHeaderBg] = CreateColorVariant(backgroundColor, 0.22f, 0.8f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = CreateColorVariant(backgroundColor, 0.25f, 0.9f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = CreateColorVariant(backgroundColor, 0.20f, 0.8f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);

	// Link and selection colors - foreground based
	colors[ImGuiCol_TextLink] = CreateColorVariant(foregroundColor, 0.75f, 1.0f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = CreateColorVariant(foregroundColor, 0.70f, 0.9f, 0.35f);

	// Drag drop and navigation - foreground based
	colors[ImGuiCol_DragDropTarget] = CreateColorVariant(foregroundColor, 1.00f, 1.0f, 0.90f);
	colors[ImGuiCol_NavCursor] = CreateColorVariant(foregroundColor, 0.80f, 1.0f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

inline void PopSmartStyle() {
	if (!g_styleStack.empty()) {
		ImGui::GetStyle() = g_styleStack.top();
		g_styleStack.pop();
	}
}