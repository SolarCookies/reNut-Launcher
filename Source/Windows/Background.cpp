#include "Background.h"

BackgroundManager::BackgroundManager() {
    for (int i = 0; i < numberOfBackgrounds; i++) {
        backgroundTextures[i] = 0;
        backgroundsLoaded[i] = false;
    }

    titleTexture = 0;
    titleLoaded = false;
    titleWidth = 0;
    titleHeight = 0;
}

BackgroundManager::~BackgroundManager() {
    Cleanup();
}

void BackgroundManager::Initialize() {
    if (isInitialized) return;

    LoadTitleTexture();
    LoadRandomBackgroundOnMainThread();
    StartBackgroundLoadingThread();

    isInitialized = true;
}

void BackgroundManager::LoadRandomBackgroundOnMainThread() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, numberOfBackgrounds);
    currentBgIndex = dis(gen) - 1;
    nextBgIndex = (currentBgIndex + 1) % numberOfBackgrounds;

    std::string firstBgPath = "Assets/Backgrounds/" + std::to_string(currentBgIndex + 1) + ".png";
    unsigned char* firstImageData = stbi_load(firstBgPath.c_str(), &bgWidth, &bgHeight, &bgChannels, 4);

    if (firstImageData == nullptr) {
        firstBgPath = "Assets/Backgrounds/" + std::to_string(currentBgIndex + 1) + ".jpg";
        firstImageData = stbi_load(firstBgPath.c_str(), &bgWidth, &bgHeight, &bgChannels, 4);
    }

    if (firstImageData != nullptr) {
        glGenTextures(1, &backgroundTextures[currentBgIndex]);
        glBindTexture(GL_TEXTURE_2D, backgroundTextures[currentBgIndex]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bgWidth, bgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, firstImageData);
        stbi_image_free(firstImageData);
        backgroundsLoaded[currentBgIndex] = true;
    }
}

void BackgroundManager::StartBackgroundLoadingThread() {
    backgroundLoader = std::thread([this]() {
        int loadedCount = 1;

        for (int i = 0; i < numberOfBackgrounds; i++) {
            if (i == currentBgIndex) continue;

            std::string bgPath = "Assets/Backgrounds/" + std::to_string(i + 1) + ".png";
            int localWidth, localHeight, localChannels;
            unsigned char* imageData = stbi_load(bgPath.c_str(), &localWidth, &localHeight, &localChannels, 4);

            if (imageData == nullptr) {
                bgPath = "Assets/Backgrounds/" + std::to_string(i + 1) + ".jpg";
                imageData = stbi_load(bgPath.c_str(), &localWidth, &localHeight, &localChannels, 4);
            }

            if (imageData != nullptr) {
                {
                    std::lock_guard<std::mutex> lock(textureMutex);
                    pendingTextures.push({ i, imageData, localWidth, localHeight, localChannels });
                }
                loadedCount++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    backgroundLoader.detach();
}

void BackgroundManager::LoadTitleTexture() {
    std::string titlePath = "Assets/title.png";
    int channels;
    unsigned char* imageData = stbi_load(titlePath.c_str(), &titleWidth, &titleHeight, &channels, 4);

    if (imageData != nullptr) {
        glGenTextures(1, &titleTexture);
        glBindTexture(GL_TEXTURE_2D, titleTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, titleWidth, titleHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
        stbi_image_free(imageData);
        titleLoaded = true;
    } else {
        titleLoaded = false;
    }
}

void BackgroundManager::ProcessPendingTextures() {
    std::lock_guard<std::mutex> lock(textureMutex);
    while (!pendingTextures.empty()) {
        PendingTexture pending = pendingTextures.front();
        pendingTextures.pop();

        glGenTextures(1, &backgroundTextures[pending.index]);
        glBindTexture(GL_TEXTURE_2D, backgroundTextures[pending.index]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pending.width, pending.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pending.imageData);

        stbi_image_free(pending.imageData);

        backgroundsLoaded[pending.index] = true;
    }
}

void BackgroundManager::UpdateBackgroundTransition(float deltaTime) {
    if (!allBackgroundsLoaded) {
        bool allLoaded = true;
        for (int i = 0; i < numberOfBackgrounds; i++) {
            if (!backgroundsLoaded[i]) {
                allLoaded = false;
                break;
            }
        }
        if (allLoaded) {
            allBackgroundsLoaded = true;
        }
    }

    if (allBackgroundsLoaded) {
        backgroundTimer += deltaTime;

        if (backgroundTimer >= transitionDuration) {
            backgroundTimer -= transitionDuration;
            currentBgIndex = nextBgIndex;

            int originalNext = nextBgIndex;
            do {
                nextBgIndex = (nextBgIndex + 1) % numberOfBackgrounds;
            } while (!backgroundsLoaded[nextBgIndex] && nextBgIndex != originalNext);

            if (!backgroundsLoaded[nextBgIndex] && nextBgIndex == originalNext) {
                nextBgIndex = currentBgIndex;
            }
        }
    }
}

void BackgroundManager::Update(float deltaTime) {
    if (!isInitialized) return;
    
    ProcessPendingTextures();
    UpdateBackgroundTransition(deltaTime);
}

void BackgroundManager::Render(ImGuiViewport* viewport) {
    if (!isInitialized) return;
    
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;

    ImGui::Begin("BackgroundImage", nullptr, window_flags);

    float fadeTime = backgroundTimer - (transitionDuration - fadeDuration);
    float alpha = 1.0f;
    bool inFade = false;

    if (fadeTime > 0.0f && fadeDuration > 0.0f) {
        alpha = 1.0f - (fadeTime / fadeDuration);
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        inFade = true;
    }

    if (backgroundsLoaded[currentBgIndex] && backgroundTextures[currentBgIndex] != 0) {
        ImGui::Image((ImTextureID)(intptr_t)backgroundTextures[currentBgIndex], viewport->Size);

        if (inFade && backgroundsLoaded[nextBgIndex] && backgroundTextures[nextBgIndex] != 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::Image((ImTextureID)(intptr_t)backgroundTextures[currentBgIndex], viewport->Size);
            ImGui::PopStyleVar();

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f - alpha);
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::Image((ImTextureID)(intptr_t)backgroundTextures[nextBgIndex], viewport->Size);
            ImGui::PopStyleVar();
        }
    }

    if (titleLoaded && titleTexture != 0) {
        float titlePosPixelX = titlePosX * viewport->Size.x;
        float titlePosPixelY = titlePosY * viewport->Size.y;

        float scaledWidth = titleWidth * titleScale;
        float scaledHeight = titleHeight * titleScale;

        float titleX = titlePosPixelX - (scaledWidth * 0.5f);
        float titleY = titlePosPixelY - (scaledHeight * 0.5f);

        if (titleRotation != 0.0f) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 center = ImVec2(titlePosPixelX + viewport->Pos.x, titlePosPixelY + viewport->Pos.y);

            float cos_a = cosf(titleRotation * 3.14159f / 180.0f);
            float sin_a = sinf(titleRotation * 3.14159f / 180.0f);

            ImVec2 corners[4] = {
                ImVec2(-scaledWidth * 0.5f, -scaledHeight * 0.5f),
                ImVec2(scaledWidth * 0.5f, -scaledHeight * 0.5f),
                ImVec2(scaledWidth * 0.5f, scaledHeight * 0.5f),
                ImVec2(-scaledWidth * 0.5f, scaledHeight * 0.5f)
            };

            for (int i = 0; i < 4; i++) {
                float x = corners[i].x * cos_a - corners[i].y * sin_a;
                float y = corners[i].x * sin_a + corners[i].y * cos_a;
                corners[i] = ImVec2(center.x + x, center.y + y);
            }

            ImVec2 uv0 = ImVec2(0.0f, 0.0f);
            ImVec2 uv1 = ImVec2(1.0f, 0.0f);
            ImVec2 uv2 = ImVec2(1.0f, 1.0f);
            ImVec2 uv3 = ImVec2(0.0f, 1.0f);

            drawList->AddImageQuad((ImTextureID)(intptr_t)titleTexture,
                                   corners[0], corners[1], corners[2], corners[3],
                                   uv0, uv1, uv2, uv3);
        } else {
            ImGui::SetCursorPos(ImVec2(titleX, titleY));
            ImGui::Image((ImTextureID)(intptr_t)titleTexture, ImVec2(scaledWidth, scaledHeight));
        }
    }

    ImGui::End();
    ImGui::PopStyleVar(3);
}

void BackgroundManager::Cleanup() {
    for (int i = 0; i < numberOfBackgrounds; i++) {
        if (backgroundsLoaded[i] && backgroundTextures[i] != 0) {
            glDeleteTextures(1, &backgroundTextures[i]);
        }
    }

    if (titleLoaded && titleTexture != 0) {
        glDeleteTextures(1, &titleTexture);
        titleTexture = 0;
        titleLoaded = false;
    }
}
