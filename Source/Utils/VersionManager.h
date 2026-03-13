#pragma once

#include <string>

class VersionManager {
public:
    static void SaveCurrentVersion(const std::string& commitSha);
    
    static std::string LoadSavedVersion();
    
    static bool VersionFileExists();
    
    static void RemoveVersionFile();

private:
    static const std::string GetVersionFilePath();
};
