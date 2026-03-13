#pragma once

#include "../Utils/OpenGL_Stuff.h"
#include <string>
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>
#include <imgui.h>
#include "../Utils/INIManager.h"

struct PendingTexture {
    int index;
    unsigned char* imageData;
    int width;
    int height;
    int channels;
};

class BackgroundManager {
private:
    static const int numberOfBackgrounds = 16;
    GLuint backgroundTextures[numberOfBackgrounds];
    std::atomic<bool> backgroundsLoaded[numberOfBackgrounds];
    std::atomic<bool> allBackgroundsLoaded{ false };
    std::mutex textureMutex;
    std::queue<PendingTexture> pendingTextures;

    int bgWidth = 0;
    int bgHeight = 0; 
    int bgChannels = 0;

    int currentBgIndex;
    int nextBgIndex;
    float backgroundTimer = 0.0f;

    const float transitionDuration = INI::GetFloat("Duration", 7.0f, "Background");
    const float fadeDuration = INI::GetFloat("FadeDuration", 1.0f, "Background");

    GLuint titleTexture = 0;
    bool titleLoaded = false;
    int titleWidth = 0;
    int titleHeight = 0;

    const float titlePosX = INI::GetFloat("TitlePosX", 0.25f, "Title");
    const float titlePosY = INI::GetFloat("TitlePosY", 0.3f, "Title");
    const float titleRotation = INI::GetFloat("TitleRotation", 0.0f, "Title");
    const float titleScale = INI::GetFloat("TitleScale", 0.4f, "Title");

    std::thread backgroundLoader;
    bool isInitialized = false;

public:
    BackgroundManager();
    ~BackgroundManager();

    void Initialize();
    void Update(float deltaTime);
    void Render(ImGuiViewport* viewport);
    void Cleanup();

private:
    void LoadRandomBackgroundOnMainThread();
    void StartBackgroundLoadingThread();
    void ProcessPendingTextures();
    void UpdateBackgroundTransition(float deltaTime);
    void LoadTitleTexture();
};
