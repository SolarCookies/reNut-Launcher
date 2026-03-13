#pragma once
#ifndef __gl_h_
#include "glad/glad.h"
#endif
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GL_SILENCE_DEPRECATION
#define GLFW_EXPOSE_NATIVE_WIN32

#pragma comment(lib, "dwmapi.lib")

#include <GLFW/glfw3.h>
#include <dwmapi.h>
#include <GLFW/glfw3native.h> // Include this for native access


#include <memory>


class VinceWindow {
	public:
		VinceWindow(int width, int height, const char* title)
			: width(width),
			height(height),
			title(title),
			window(nullptr, glfwDestroyWindow) // Properly initialize unique_ptr with deleter
		{
			init();
		}
	void init();
	void SetupImGuiIO();

    void EnableBlur()
    {
        HWND hwnd = glfwGetWin32Window(window.get());
        if (!hwnd) return;

        MARGINS margins = { -1 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        const HINSTANCE hModule = LoadLibrary(TEXT("user32.dll"));
        if (hModule)
        {
            typedef struct _ACCENT_POLICY
            {
                int nAccentState;
                int nFlags;
                int nColor;
                int nAnimationId;
            } ACCENT_POLICY;

            typedef struct _WINDOWCOMPOSITIONATTRIBDATA
            {
                int nAttribute;
                PVOID pData;
                SIZE_T ulDataSize;
            } WINDOWCOMPOSITIONATTRIBDATA;

            enum AccentState
            {
                ACCENT_DISABLED = 0,
                ACCENT_ENABLE_BLURBEHIND = 3,
                ACCENT_ENABLE_ACRYLICBLURBEHIND = 4
            };

            auto SetWindowCompositionAttribute = (BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*))GetProcAddress(hModule, "SetWindowCompositionAttribute");
            if (SetWindowCompositionAttribute)
            {
                ACCENT_POLICY policy = { ACCENT_ENABLE_BLURBEHIND, 0, 0, 0 };
                WINDOWCOMPOSITIONATTRIBDATA data = { 19, &policy, sizeof(policy) };
                SetWindowCompositionAttribute(hwnd, &data);
            }
            FreeLibrary(hModule);
        }
    }
	void NewFrame() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		glClearColor(0.0f, 0.0f, 0.0f, 0.00f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);
	}
	void EndFrame() {
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window.get());
	}
	GLFWwindow* getWindow() const {
		return window.get();
	}
	const char* glsl_version;

	int width, height;
private:
		const char* title;
		HWND hwnd = glfwGetWin32Window(window.get());
		std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> window; // Use custom deleter type
};

