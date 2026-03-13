#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <thread>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

struct DownloadProgress {
    std::atomic<size_t> bytesDownloaded;
    std::atomic<size_t> totalBytes;
    std::atomic<bool> isComplete;
    std::atomic<bool> hasError;
    std::string errorMessage;
    
    DownloadProgress() : bytesDownloaded(0), totalBytes(0), isComplete(false), hasError(false) {}
    
    float GetPercentage() const {
        if (totalBytes.load() == 0) return 0.0f;
        return (float)bytesDownloaded.load() / (float)totalBytes.load() * 100.0f;
    }
};

class FileDownloader {
public:
    static void DownloadFileAsync(const std::string& url, const std::string& localPath, 
                                 std::shared_ptr<DownloadProgress> progress,
                                 std::function<void()> onSuccessCallback = nullptr);
    
private:
    static bool DownloadFile(const std::string& url, const std::string& localPath, 
                           std::shared_ptr<DownloadProgress> progress);
    static bool ParseUrl(const std::string& url, std::wstring& server, std::wstring& path);
    static bool EnsureDirectoryExists(const std::string& filePath);
};
