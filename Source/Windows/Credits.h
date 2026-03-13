#pragma once

#include <vector>
#include "../Utils/GitHubAPI.h"

class Credits {
public:
    Credits();
    ~Credits();

    void Initialize(const std::string& repoName);
    
    void Render();
    
    bool IsLoading() const { return contributorsLoading; }
    
    bool IsLoaded() const { return contributorsLoaded; }
    
private:
    std::vector<GitContributor> githubContributors;
    bool contributorsLoaded;
    bool contributorsLoading;
    
    void OnContributorsLoaded(std::vector<GitContributor> contributors);
};
