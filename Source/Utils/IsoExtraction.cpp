#include "IsoExtraction.h"
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <vector>
#include <direct.h>
#include <filesystem>
#include <windows.h>
#include <commdlg.h>
#include <sstream>

namespace IsoExtraction {

std::string OpenIsoFileDialog() {
    std::string originalCwd = std::filesystem::current_path().string();

    OPENFILENAME ofn;
    wchar_t szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"ISO Files\0*.iso\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    std::string result;
    if (GetOpenFileNameW(&ofn) == TRUE) {
        int len = WideCharToMultiByte(CP_UTF8, 0, szFile, -1, NULL, 0, NULL, NULL);
        result.resize(len - 1);
        WideCharToMultiByte(CP_UTF8, 0, szFile, -1, &result[0], len, NULL, NULL);
    }

    try {
        std::filesystem::current_path(originalCwd);
    } catch (const std::exception& e) {
      // eh
    }

    return result;
}

std::string EscapeForCmd(const std::string& path) {
    std::string escaped = path;
    size_t pos = 0;
    while ((pos = escaped.find("&", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "^&");
        pos += 2;
    }

    return escaped;
}

void ExtractIsoAsync(const std::string& isoPath, std::shared_ptr<IsoExtractionProgress> progress) {
    std::thread([isoPath, progress]() {
        progress->isExtracting = true;
        progress->isComplete = false;
        progress->hasError = false;

        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::string exeDir = std::filesystem::path(exePath).parent_path().string();

        std::vector<std::string> possiblePaths = {
            "Assets\\xdvdfs.exe",
            "Launcher\\Assets\\xdvdfs.exe",
            ".\\Assets\\xdvdfs.exe",
            ".\\Launcher\\Assets\\xdvdfs.exe",
            exeDir + "\\Assets\\xdvdfs.exe",
            exeDir + "\\..\\Launcher\\Assets\\xdvdfs.exe",
            exeDir + "\\..\\..\\..\\Assets\\xdvdfs.exe"
        };

        std::string exisoPath;
        bool found = false;

        for (const std::string& path : possiblePaths) {
            if (std::filesystem::exists(path)) {
                exisoPath = path;
                found = true;
                break;
            }
        }

        if (!found) {
            progress->hasError = true;
            progress->errorMessage = "xdvdfs.exe not found in any expected location";
            progress->isExtracting = false;
            return;
        }

        std::filesystem::path exisoDir = std::filesystem::path(exisoPath).parent_path();
        std::filesystem::path gameAssetsPath = exisoDir / ".." / "Game" / "assets";
        std::string absoluteGameAssetsPath = std::filesystem::absolute(gameAssetsPath).string();

        std::filesystem::path gameAssetsDir = absoluteGameAssetsPath;
        if (std::filesystem::exists(gameAssetsDir)) {
            std::filesystem::remove_all(gameAssetsDir);
        }
        std::filesystem::create_directories(gameAssetsDir);

        std::string escapedIsoPath = EscapeForCmd(isoPath);

        std::string escapedDestPath = EscapeForCmd(absoluteGameAssetsPath);

        std::string arguments = "unpack \"" + escapedIsoPath + "\" \"" + escapedDestPath + "\"";

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        ZeroMemory(&pi, sizeof(pi));

        std::string commandLine = "\"" + exisoPath + "\" " + arguments;

        BOOL success = CreateProcessA(
            NULL,
            const_cast<char*>(commandLine.c_str()),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi
        );

        if (!success) {
            progress->hasError = true;
            progress->errorMessage = "Failed to start xdvdfs.exe";
            progress->isExtracting = false;
            return;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        progress->isExtracting = false;
        if (exitCode == 0) {
            progress->isComplete = true;
        } else {
            progress->hasError = true;
            progress->errorMessage = "xdvdfs.exe failed with exit code: " + std::to_string(exitCode);
        }
    }).detach();
}

}
