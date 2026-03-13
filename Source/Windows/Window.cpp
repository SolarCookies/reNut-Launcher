#include "Window.h"
//#include "Log.hpp"
#include "../Utils/OpenGL_Stuff.h"
#include "../Utils/INIManager.h"
#include "Fonts.h"

void VinceWindow::init()
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		assert(false && "Failed to initialize GLFW");

	glsl_version = "#version 330";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

	window = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>(
    glfwCreateWindow(1280, 720, INI::GetString("Title", "reNut Launcher", "Window").c_str(),nullptr, nullptr),
		&glfwDestroyWindow
	);
	if (!window) {
		assert(false && "Failed to create GLFW window");
	}

	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window.get(), &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(window.get());

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		assert(false && "Failed to initialize OpenGL context");
	}

	glViewport(0, 0, bufferWidth, bufferHeight);
	EnableBlur();
}

void VinceWindow::SetupImGuiIO()
{
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui::StyleColorsDark();

		BasicTextFont = io.Fonts->AddFontFromFileTTF("Assets/Comiccrazy.ttf", 18.0f);
		BasicTitleFont = io.Fonts->AddFontFromFileTTF("Assets/Lithos.ttf", 30.0f);

		ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		//ImGui Style Colors
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.35f, 0.20f, 0.0f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.35f, 0.20f, 0.0f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.15f, 0.08f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.00f, 0.60f, 0.30f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.05f, 0.02f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.35f, 0.20f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.65f, 0.35f, 0.40f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.75f, 0.40f, 0.67f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.12f, 0.06f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.05f, 0.20f, 0.12f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.15f, 0.08f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.10f, 0.05f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.05f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.10f, 0.60f, 0.30f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.80f, 0.40f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.85f, 0.45f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.18f, 0.70f, 0.38f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.22f, 0.85f, 0.48f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.18f, 0.70f, 0.38f, 0.40f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.80f, 0.45f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.10f, 0.65f, 0.30f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.15f, 0.55f, 0.30f, 0.31f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.70f, 0.38f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.80f, 0.45f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.05f, 0.40f, 0.20f, 0.50f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.60f, 0.30f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.15f, 0.70f, 0.35f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.75f, 0.40f, 0.20f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.85f, 0.48f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.28f, 0.90f, 0.52f, 0.95f);
			colors[ImGuiCol_InputTextCursor] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.80f, 0.45f, 0.80f);
			colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.45f, 0.25f, 0.86f);
			colors[ImGuiCol_TabSelected] = ImVec4(0.16f, 0.58f, 0.32f, 1.00f);
			colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.22f, 0.80f, 0.45f, 1.00f);
			colors[ImGuiCol_TabDimmed] = ImVec4(0.05f, 0.15f, 0.08f, 0.97f);
			colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.10f, 0.35f, 0.18f, 1.00f);
			colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.12f, 0.42f, 0.22f, 0.00f);
			colors[ImGuiCol_DockingPreview] = ImVec4(0.25f, 0.85f, 0.48f, 0.70f);
			colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.02f, 0.15f, 0.08f, 0.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.20f, 0.90f, 0.45f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.35f, 1.00f, 0.60f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.70f, 0.30f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.85f, 0.40f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.02f, 0.22f, 0.12f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.03f, 0.25f, 0.13f, 1.00f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.02f, 0.20f, 0.10f, 1.00f);
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextLink] = ImVec4(0.10f, 0.75f, 0.35f, 1.00f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.18f, 0.70f, 0.38f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.30f, 1.00f, 0.50f, 0.90f);
			colors[ImGuiCol_NavCursor] = ImVec4(0.22f, 0.80f, 0.45f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		}
	}
}

