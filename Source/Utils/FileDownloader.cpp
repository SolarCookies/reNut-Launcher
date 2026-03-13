#include "FileDownloader.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

void FileDownloader::DownloadFileAsync(const std::string& url, const std::string& localPath, 
                                      std::shared_ptr<DownloadProgress> progress,
                                      std::function<void()> onSuccessCallback) {
    std::thread([url, localPath, progress, onSuccessCallback]() {
        bool success = DownloadFile(url, localPath, progress);
        progress->isComplete.store(true);
        if (success && onSuccessCallback) {
            onSuccessCallback();
        } else if (!success && !progress->hasError.load()) {
            progress->hasError.store(true);
            progress->errorMessage = "Download failed";
        }
    }).detach();
}

bool FileDownloader::DownloadFile(const std::string& url, const std::string& localPath, 
                                 std::shared_ptr<DownloadProgress> progress) {
    std::wstring server, path;
    if (!ParseUrl(url, server, path)) {
        progress->hasError.store(true);
        progress->errorMessage = "Invalid URL";
        return false;
    }

    if (!EnsureDirectoryExists(localPath)) {
        progress->hasError.store(true);
        progress->errorMessage = "Failed to create directory";
        return false;
    }

    HINTERNET hSession = WinHttpOpen(L"NB-Launcher/1.0",
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS,
                                    0);
    
    if (!hSession) {
        progress->hasError.store(true);
        progress->errorMessage = "Failed to initialize HTTP session";
        return false;
    }

    HINTERNET hConnect = WinHttpConnect(hSession, server.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        progress->hasError.store(true);
        progress->errorMessage = "Failed to connect to server";
        return false;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
                                           WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           WINHTTP_FLAG_SECURE);
    
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        progress->hasError.store(true);
        progress->errorMessage = "Failed to create request";
        return false;
    }

    std::wstring headers = L"User-Agent: NB-Launcher/1.0\r\n";
    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

    BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    
    if (!bResults) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        progress->hasError.store(true);
        progress->errorMessage = "Failed to send request";
        return false;
    }

    bResults = WinHttpReceiveResponse(hRequest, NULL);
    
    if (!bResults) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        progress->hasError.store(true);
        progress->errorMessage = "Failed to receive response";
        return false;
    }

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                       WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);

    if (statusCode != 200) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        progress->hasError.store(true);
        progress->errorMessage = "HTTP Error " + std::to_string(statusCode);
        return false;
    }

    DWORD contentLength = 0;
    DWORD contentLengthSize = sizeof(contentLength);
    if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
                           WINHTTP_HEADER_NAME_BY_INDEX, &contentLength, &contentLengthSize, WINHTTP_NO_HEADER_INDEX)) {
        progress->totalBytes.store(contentLength);
    }

    std::ofstream outFile(localPath, std::ios::binary);
    if (!outFile) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        progress->hasError.store(true);
        progress->errorMessage = "Failed to create output file";
        return false;
    }

    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    size_t totalDownloaded = 0;
    
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        
        if (dwSize == 0) break;
        
        char* buffer = new char[dwSize];
        
        if (WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded)) {
            outFile.write(buffer, dwDownloaded);
            totalDownloaded += dwDownloaded;
            progress->bytesDownloaded.store(totalDownloaded);
        }
        
        delete[] buffer;
    } while (dwSize > 0);

    outFile.close();

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return true;
}

bool FileDownloader::ParseUrl(const std::string& url, std::wstring& server, std::wstring& path) {
    std::string prefix = "https://";
    if (url.substr(0, prefix.length()) != prefix) {
        return false;
    }

    std::string remaining = url.substr(prefix.length());
    size_t slashPos = remaining.find('/');
    
    if (slashPos == std::string::npos) {
        return false;
    }

    std::string serverStr = remaining.substr(0, slashPos);
    std::string pathStr = remaining.substr(slashPos);

    server = std::wstring(serverStr.begin(), serverStr.end());
    path = std::wstring(pathStr.begin(), pathStr.end());

    return true;
}

bool FileDownloader::EnsureDirectoryExists(const std::string& filePath) {
    try {
        std::filesystem::path path(filePath);
        std::filesystem::path dir = path.parent_path();
        
        if (!dir.empty() && !std::filesystem::exists(dir)) {
            return std::filesystem::create_directories(dir);
        }
        
        return true;
    } catch (...) {
        return false;
    }
}
