#include "LogSystem.h"
#include <imgui.h>
#include <iostream>
#include <chrono>
#include <iomanip>

LogSystem* LogSystem::instance = nullptr;

LogSystem& LogSystem::GetInstance() {
    if (!instance) {
        instance = new LogSystem();
    }
    return *instance;
}

void LogSystem::AddLog(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream ss;
    
    ss << message;
    
    logLines.push_back(ss.str());
    
    if (logLines.size() > maxLogLines) {
        logLines.erase(logLines.begin());
    }
}

void LogSystem::Clear() {
    std::lock_guard<std::mutex> lock(logMutex);
    logLines.clear();
}

void LogSystem::Render(bool* showLogWindow) {
    if (!showLogWindow || !*showLogWindow) {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    if (ImGui::Begin("Console Log", showLogWindow)) {
        if (ImGui::Button("Clear")) {
            Clear();
        }
        ImGui::SameLine();

        static bool autoScroll = true;
        ImGui::Checkbox("Auto-scroll", &autoScroll);

        ImGui::Separator();

        if (ImGui::BeginChild("LogContent", ImVec2(0, 0), false)) {
            std::lock_guard<std::mutex> lock(logMutex);

            for (const auto& line : logLines) {
                ImGui::TextWrapped("%s", line.c_str());
            }

            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::PopStyleColor(3);
}

std::vector<std::string> LogSystem::GetLogLines() {
    std::lock_guard<std::mutex> lock(logMutex);
    return logLines;
}

LogStreambuf::LogStreambuf(std::streambuf* original) : originalBuffer(original) {
  // eh
}

LogStreambuf::~LogStreambuf() {
  // eh
}

int LogStreambuf::overflow(int c) {
    if (c != EOF) {
        buffer << static_cast<char>(c);
        if (originalBuffer) {
            originalBuffer->sputc(c);
        }
    }
    return c;
}

int LogStreambuf::sync() {
    std::string line = buffer.str();
    if (!line.empty()) {
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
            line.pop_back();
        }
        if (!line.empty()) {
            LogSystem::GetInstance().AddLog(line);
        }
        buffer.str("");
        buffer.clear();
    }
    
    if (originalBuffer) {
        return originalBuffer->pubsync();
    }
    return 0;
}
