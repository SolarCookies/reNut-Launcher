#include "VersionManager.h"
#include "INIManager.h"
#include <fstream>
#include <filesystem>
#include <iostream>

void VersionManager::SaveCurrentVersion(const std::string& commitSha) {
    try {
        std::string versionFilePath = GetVersionFilePath();
        
        std::filesystem::path filePath(versionFilePath);
        std::filesystem::create_directories(filePath.parent_path());
        
        std::ofstream file(versionFilePath);
        if (file.is_open()) {
            file << commitSha;
            file.close();
        } else {
            std::cout << "Failed to save version file: " << versionFilePath << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error saving version: " << e.what() << std::endl;
    }
}

std::string VersionManager::LoadSavedVersion() {
    try {
        std::string versionFilePath = GetVersionFilePath();
        
        if (!std::filesystem::exists(versionFilePath)) {
            return "";
        }
        
        std::ifstream file(versionFilePath);
        if (file.is_open()) {
            std::string version;
            std::getline(file, version);
            file.close();
            return version;
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error loading version: " << e.what() << std::endl;
    }
    
    return "";
}

bool VersionManager::VersionFileExists() {
    try {
        return std::filesystem::exists(GetVersionFilePath());
    }
    catch (const std::exception& e) {
        return false;
    }
}

void VersionManager::RemoveVersionFile() {
    try {
        std::string versionFilePath = GetVersionFilePath();
        if (std::filesystem::exists(versionFilePath)) {
            std::filesystem::remove(versionFilePath);
        }
    }
    catch (const std::exception& e) {
      // log or something eventually
    }
}

const std::string VersionManager::GetVersionFilePath() {
    std::string basePath = INI::GetString("GamePath", "Game/", "Game");
    if (!basePath.empty() && basePath.back() != '\\' && basePath.back() != '/') {
        basePath += "/";
    }
    return basePath + "Game/version.txt";
}
