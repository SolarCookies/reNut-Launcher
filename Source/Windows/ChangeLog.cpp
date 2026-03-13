#include "ChangeLog.h"
#include <imgui.h>
#include <string>
#include "../Utils/INIManager.h"

ChangeLog::ChangeLog() 
    : commitsLoaded(false), commitsLoading(false) {
}

ChangeLog::~ChangeLog() {
  // eh, maybe eventually ill care to do cleanup
}

void ChangeLog::Initialize(const std::string& repoName, const std::string& branchName) {
    if (!commitsLoading && !commitsLoaded) {
        commitsLoading = true;
        
        GitHubAPI::FetchCommitsAsync(
            repoName,
            branchName,
            [this](std::vector<GitCommit> commits) {
                OnCommitsLoaded(std::move(commits));
            });
    }
}

void ChangeLog::OnCommitsLoaded(std::vector<GitCommit> commits) {
    githubCommits = std::move(commits);
    commitsLoaded = true;
    commitsLoading = false;
}

void ChangeLog::Render() {
    ImGui::Begin("Change Log");
    ImGui::BeginChild("ChangeLogChild", ImVec2(0, ImGui::GetWindowHeight() - 40), true);

    if (commitsLoading) {
        ImGui::Text("Loading commits from GitHub...");
    }
    else if (!commitsLoaded) {
        ImGui::Text("Failed to load commits from GitHub");
        ImGui::Text("Please check your internet connection and try restarting the application.");
    }
    else if (githubCommits.empty()) {
        ImGui::Text("No commits found for this repository");
    }
    else {
        for (int i = 0; i < githubCommits.size(); i++) {
            const auto& commit = githubCommits[i];

            std::string childId = "commit_" + std::to_string(i);

            ImGui::BeginChild(childId.c_str(), ImVec2(0, 80), true);

            ImGui::TextWrapped("- [%s] %s", commit.sha.c_str(), commit.message.c_str());
            ImGui::Indent();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui::TextWrapped("by %s on %s", commit.author.c_str(), commit.date.c_str());
            ImGui::PopStyleColor();
            ImGui::Unindent();

            ImGui::EndChild();
            ImGui::Spacing();
        }
    }

    ImGui::EndChild();
    ImGui::End();
}
