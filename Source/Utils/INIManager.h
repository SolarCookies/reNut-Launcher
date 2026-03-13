#pragma once

#include "ini.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <string>

// Manager for ini.h, Saves and reads values from ./config.ini and provides convenient helper
// functions like GetBool(name) and SetBool(name, value)

inline std::string iniFilePath = "config.ini";

namespace INI {

  inline void EnsureINIFileExists() {
    mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
    mINI::INIStructure iniData;
    if (!iniFile.read(iniData)) {
      iniFile.generate(iniData);  // Create the file if it doesn't exist
    }
  }
    
    inline void SetBool(const std::string& name, bool value, const std::string& path = "Settings") {
      mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
      mINI::INIStructure iniData;
      if (iniFile.read(iniData)) {
        iniData[path][name] = value ? "true" : "false";
        iniFile.write(iniData);
      }
    }

    inline bool GetBool(const std::string& name, const std::string& path = "Settings",
                        bool defaultValue = false) {
      mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
      mINI::INIStructure iniData;
      if (iniFile.read(iniData)) {
        std::string value = iniData[path][name];
          if (value == "true" || value == "1") {
              return true;
          } else if (value == "false" || value == "0") {
              return false;
          }
      }
      SetBool(name, defaultValue, path);
      return defaultValue;
    }

    inline void SetInt(const std::string& name, int value, const std::string& path = "Settings") {
      mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
      mINI::INIStructure iniData;
      if (iniFile.read(iniData)) {
        iniData[path][name] = std::to_string(value);
        iniFile.write(iniData);
      }
    }

    inline int GetInt(const std::string& name, int defaultValue = 0,
                      const std::string& path = "Settings") {
      mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
      mINI::INIStructure iniData;
      if (iniFile.read(iniData)) {
        std::string value = iniData[path][name];
        try {
          return std::stoi(value);
        } catch (const std::exception&) {
          SetInt(name, defaultValue, path);
          return defaultValue;
        }
      }
      SetInt(name, defaultValue, path);
      return defaultValue;
    }

    inline void SetString(const std::string& name, const std::string& value,
                          const std::string& path = "Settings") {
      mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
      mINI::INIStructure iniData;
      if (iniFile.read(iniData)) {
        iniData[path][name] = value;
        iniFile.write(iniData);
      }
    }

    inline std::string GetString(const std::string& name, const std::string& defaultValue = "",
                          const std::string& path = "Settings") {
      mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
      mINI::INIStructure iniData;
      if (iniFile.read(iniData)) {
        std::string value = iniData[path][name];
        if (!value.empty()) {
          return value;
        }
      }
      SetString(name, defaultValue, path);
      return defaultValue;
    }

    inline void SetFloat(const std::string& name, float value, const std::string& path = "Settings") {
      mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
      mINI::INIStructure iniData;
      if (iniFile.read(iniData)) {
        iniData[path][name] = std::to_string(value);
        iniFile.write(iniData);
      }
    }

    inline float GetFloat(const std::string& name, float defaultValue = 0.0f,
                          const std::string& path = "Settings") {
      mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
      mINI::INIStructure iniData;
      if (iniFile.read(iniData)) {
        std::string value = iniData[path][name];
        try {
          return std::stof(value);
        } catch (const std::exception&) {
          SetFloat(name, defaultValue, path);
          return defaultValue;
        }
      }
      SetFloat(name, defaultValue, path);
      return defaultValue;
    }

    inline void SetImColor(const std::string& name, const ImVec4& color, const std::string& path = "Settings") {
        mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
        mINI::INIStructure iniData;
        if (iniFile.read(iniData)) {
            iniData[path][name] = std::to_string(color.x) + "," + std::to_string(color.y) + "," +
                std::to_string(color.z) + "," + std::to_string(color.w);
            iniFile.write(iniData);
        }
    }

    inline ImVec4 GetImColor(const std::string& name, const ImVec4& defaultColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
        const std::string& path = "Settings") {
        mINI::INIFile iniFile = mINI::INIFile(iniFilePath);
        mINI::INIStructure iniData;
        if (iniFile.read(iniData)) {
            std::string value = iniData[path][name];
            if (!value.empty()) {
                size_t pos1 = value.find(',');
                size_t pos2 = value.find(',', pos1 + 1);
                size_t pos3 = value.find(',', pos2 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
                    try {
                        float x = std::stof(value.substr(0, pos1));
                        float y = std::stof(value.substr(pos1 + 1, pos2 - pos1 - 1));
                        float z = std::stof(value.substr(pos2 + 1, pos3 - pos2 - 1));
                        float w = std::stof(value.substr(pos3 + 1));
                        return ImVec4(x, y, z, w);
                    }
                    catch (const std::exception&) {
                        SetImColor(name, defaultColor, path);
                        return defaultColor;
                    }
                }
            }
        }
        SetImColor(name, defaultColor, path);
        return defaultColor;
    }


    }