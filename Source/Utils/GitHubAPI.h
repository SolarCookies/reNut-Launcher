#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <winhttp.h>
#include <thread>
#include <functional>

#pragma comment(lib, "winhttp.lib")

struct GitCommit {
    std::string sha;
    std::string message;
    std::string author;
    std::string date;
};

struct GitContributor {
    std::string login;
    std::string name;
    std::string avatar_url;
    int contributions;
};

class GitHubAPI {
public:
    static void FetchCommitsAsync(const std::string& repo, const std::string& branch, 
                                 std::function<void(std::vector<GitCommit>)> callback);
    static void FetchContributorsAsync(const std::string& repo, 
                                      std::function<void(std::vector<GitContributor>)> callback);
    static void FetchLatestCommitShaAsync(const std::string& repo, const std::string& branch,
                                         std::function<void(std::string)> callback);
    static void FetchLatestCommitShaForFileAsync(const std::string& repo, const std::string& branch,
                                                 const std::string& filePath, std::function<void(std::string)> callback);

private:
    static std::vector<GitCommit> FetchCommits(const std::string& repo, const std::string& branch);
    static std::vector<GitContributor> FetchContributors(const std::string& repo);
    static std::string FetchLatestCommitSha(const std::string& repo, const std::string& branch);
    static std::string FetchLatestCommitShaForFile(const std::string& repo, const std::string& branch,
                                                   const std::string& filePath);
    static std::string HttpGet(const std::wstring& server, const std::wstring& path);
    static std::vector<GitCommit> ParseCommitsJson(const std::string& json);
    static std::vector<GitContributor> ParseContributorsJson(const std::string& json);
    static std::string ExtractJsonString(const std::string& json, const std::string& key, size_t startPos);
    static int ExtractJsonInt(const std::string& json, const std::string& key, size_t startPos);
};
