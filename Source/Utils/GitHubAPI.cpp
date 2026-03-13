#include "GitHubAPI.h"
#include <sstream>
#include <iostream>

void GitHubAPI::FetchCommitsAsync(const std::string& repo, const std::string& branch,
                                 std::function<void(std::vector<GitCommit>)> callback) {
    std::thread([repo, branch, callback]() {
        auto commits = FetchCommits(repo, branch);
        callback(commits);
    }).detach();
}

void GitHubAPI::FetchContributorsAsync(const std::string& repo, 
                                      std::function<void(std::vector<GitContributor>)> callback) {
    std::thread([repo, callback]() {
        auto contributors = FetchContributors(repo);
        callback(contributors);
    }).detach();
}

void GitHubAPI::FetchLatestCommitShaAsync(const std::string& repo, const std::string& branch,
                                         std::function<void(std::string)> callback) {
    std::thread([repo, branch, callback]() {
        auto sha = FetchLatestCommitSha(repo, branch);
        callback(sha);
    }).detach();
}

void GitHubAPI::FetchLatestCommitShaForFileAsync(const std::string& repo, const std::string& branch,
                                                 const std::string& filePath, std::function<void(std::string)> callback) {
    std::thread([repo, branch, filePath, callback]() {
        auto sha = FetchLatestCommitShaForFile(repo, branch, filePath);
        callback(sha);
    }).detach();
}

std::vector<GitCommit> GitHubAPI::FetchCommits(const std::string& repo, const std::string& branch) {
    std::vector<GitCommit> commits;

    try {
        std::wstring path = L"/repos/" + std::wstring(repo.begin(), repo.end()) + 
                           L"/commits?sha=" + std::wstring(branch.begin(), branch.end()) + L"&per_page=10";

        std::cout << "Fetching commits from: api.github.com" << std::string(path.begin(), path.end()) << std::endl;

        std::string jsonResponse = HttpGet(L"api.github.com", path);

        if (!jsonResponse.empty()) {
            commits = ParseCommitsJson(jsonResponse);
        } else {
            std::cout << "Empty response from GitHub API for commits" << std::endl;
        }
    }
    catch (...) {
        // Handle errors eventually
    }

    return commits;
}

std::vector<GitContributor> GitHubAPI::FetchContributors(const std::string& repo) {
    std::vector<GitContributor> contributors;

    try {
        std::wstring path = L"/repos/" + std::wstring(repo.begin(), repo.end()) + L"/contributors";

        std::string jsonResponse = HttpGet(L"api.github.com", path);

        if (!jsonResponse.empty()) {
            std::cout << "GitHub API Response (first 200 chars): " << jsonResponse.substr(0, 200) << std::endl;
            contributors = ParseContributorsJson(jsonResponse);
            std::cout << "Parsed " << contributors.size() << " contributors" << std::endl;
        }
    }
    catch (...) {
      // Handle errors eventually
    }

    return contributors;
}

std::string GitHubAPI::FetchLatestCommitSha(const std::string& repo, const std::string& branch) {
    std::string latestSha;

    try {
        std::wstring path = L"/repos/" + std::wstring(repo.begin(), repo.end()) + 
                           L"/commits?sha=" + std::wstring(branch.begin(), branch.end()) + L"&per_page=1";

        std::string jsonResponse = HttpGet(L"api.github.com", path);

        if (!jsonResponse.empty()) {
            size_t shaPos = jsonResponse.find("\"sha\":");
            if (shaPos != std::string::npos) {
                latestSha = ExtractJsonString(jsonResponse, "sha", shaPos);
                std::cout << "Latest commit SHA: " << latestSha << std::endl;
            }
        }
    }
    catch (...) {
      // Handle errors eventually
    }

    return latestSha;
}

std::string GitHubAPI::FetchLatestCommitShaForFile(const std::string& repo, const std::string& branch, const std::string& filePath) {
    std::string latestSha;

    try {
        // Use the path parameter to filter commits that affect the specific file
        std::wstring path = L"/repos/" + std::wstring(repo.begin(), repo.end()) + 
                           L"/commits?sha=" + std::wstring(branch.begin(), branch.end()) + 
                           L"&path=" + std::wstring(filePath.begin(), filePath.end()) + L"&per_page=1";

        std::cout << "Fetching latest commit for file: " << filePath << std::endl;

        std::string jsonResponse = HttpGet(L"api.github.com", path);

        if (!jsonResponse.empty()) {
            size_t shaPos = jsonResponse.find("\"sha\":");
            if (shaPos != std::string::npos) {
                latestSha = ExtractJsonString(jsonResponse, "sha", shaPos);
                std::cout << "Latest commit SHA for " << filePath << ": " << latestSha << std::endl;
            } else {
                std::cout << "No commits found for file: " << filePath << std::endl;
            }
        }
    }
    catch (...) {
      // Handle errors eventually
    }

    return latestSha;
}

std::string GitHubAPI::HttpGet(const std::wstring& server, const std::wstring& path) {
    std::string result;

    HINTERNET hSession = WinHttpOpen(L"GitHubAPI/1.0",
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS,
                                    0);

    if (!hSession) {
        return result;
    }

    HINTERNET hConnect = WinHttpConnect(hSession, server.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);

    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return result;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
                                           WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           WINHTTP_FLAG_SECURE);

    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    std::wstring headers = L"User-Agent: NB-Launcher/1.0\r\n";
    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

    BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (!bResults) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    bResults = WinHttpReceiveResponse(hRequest, NULL);

    if (!bResults) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                       WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);

    if (statusCode != 200) {
        std::cout << "HTTP request failed with status code: " << statusCode << std::endl;
    }

    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;

    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;

        if (dwSize == 0) break;

        char* buffer = new char[dwSize + 1];
        ZeroMemory(buffer, dwSize + 1);

        if (WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded)) {
            result.append(buffer, dwDownloaded);
        }

        delete[] buffer;
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return result;
}

std::vector<GitCommit> GitHubAPI::ParseCommitsJson(const std::string& json) {
    std::vector<GitCommit> commits;
    
    size_t pos = 0;
    while ((pos = json.find("\"sha\":", pos)) != std::string::npos) {
        GitCommit commit;
        
        commit.sha = ExtractJsonString(json, "sha", pos);
        if (commit.sha.length() > 8) {
            commit.sha = commit.sha.substr(0, 8);
        }
        
        size_t commitPos = json.find("\"commit\":", pos);
        if (commitPos != std::string::npos) {
            size_t messagePos = json.find("\"message\":", commitPos);
            if (messagePos != std::string::npos) {
                commit.message = ExtractJsonString(json, "message", messagePos);
                size_t newlinePos = commit.message.find("\\n");
                if (newlinePos != std::string::npos) {
                    commit.message = commit.message.substr(0, newlinePos);
                }
            }
            
            size_t authorPos = json.find("\"author\":", commitPos);
            if (authorPos != std::string::npos) {
                size_t namePos = json.find("\"name\":", authorPos);
                if (namePos != std::string::npos) {
                    commit.author = ExtractJsonString(json, "name", namePos);
                }
                
                size_t datePos = json.find("\"date\":", authorPos);
                if (datePos != std::string::npos) {
                    commit.date = ExtractJsonString(json, "date", datePos);
                    if (commit.date.length() >= 10) {
                        commit.date = commit.date.substr(0, 10);
                    }
                }
            }
        }
        
        if (!commit.sha.empty() && !commit.message.empty()) {
            commits.push_back(commit);
        }
        
        pos = commitPos != std::string::npos ? commitPos + 1 : pos + 1;
        
        // Limit to reasonable number of commits to avoid UI clutter, Im going to say 15 for now but yeah
        if (commits.size() >= 15) break;
    }

    return commits;
}

std::vector<GitContributor> GitHubAPI::ParseContributorsJson(const std::string& json) {
    std::vector<GitContributor> contributors;

    size_t pos = 0;
    while ((pos = json.find("\"login\":", pos)) != std::string::npos) {
        GitContributor contributor;

        contributor.login = ExtractJsonString(json, "login", pos);

        contributor.contributions = ExtractJsonInt(json, "contributions", pos);

        contributor.avatar_url = ExtractJsonString(json, "avatar_url", pos);

        if (!contributor.login.empty()) {
            contributors.push_back(contributor);
        }

        pos++;

        if (contributors.size() >= 30) break;
    }

    return contributors;
}

std::string GitHubAPI::ExtractJsonString(const std::string& json, const std::string& key, size_t startPos) {
    size_t keyPos = json.find("\"" + key + "\":", startPos);
    if (keyPos == std::string::npos) return "";
    
    size_t valueStart = json.find("\"", keyPos + key.length() + 2);
    if (valueStart == std::string::npos) return "";
    valueStart++;
    
    size_t valueEnd = valueStart;
    while (valueEnd < json.length() && json[valueEnd] != '"') {
        if (json[valueEnd] == '\\') {
            valueEnd += 2;
        } else {
            valueEnd++;
        }
    }
    
    if (valueEnd == std::string::npos) return "";

    return json.substr(valueStart, valueEnd - valueStart);
}

int GitHubAPI::ExtractJsonInt(const std::string& json, const std::string& key, size_t startPos) {
    size_t keyPos = json.find("\"" + key + "\":", startPos);
    if (keyPos == std::string::npos) return 0;

    size_t valueStart = json.find(":", keyPos + key.length() + 2);
    if (valueStart == std::string::npos) return 0;
    valueStart++;

    while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\t' || json[valueStart] == '\n' || json[valueStart] == '\r')) {
        valueStart++;
    }

    size_t valueEnd = valueStart;
    while (valueEnd < json.length() && json[valueEnd] >= '0' && json[valueEnd] <= '9') {
        valueEnd++;
    }

    if (valueEnd == valueStart) return 0;

    try {
        return std::stoi(json.substr(valueStart, valueEnd - valueStart));
    } catch (...) {
        return 0;
    }
}
