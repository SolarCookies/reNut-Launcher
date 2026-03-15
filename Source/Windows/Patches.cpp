#include "Patches.h"
#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <imgui.h>
#include "../Utils/INIManager.h"
#include "../Utils/ini.h"
#pragma comment(lib, "wininet.lib")


void Patches::Init() {
    patchList.clear();
    patchDescriptions.clear();
    patchTypes.clear();
    patchDefaultValues.clear();
    patches.clear();
    bool onlineSuccess = false;

    HINTERNET hInternet = InternetOpenA(INI::GetString("Title", "reTiP Launcher", "Window").c_str(),INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        HINTERNET hConnect = InternetOpenUrlA(hInternet, 
            INI::GetString("PatchesLocation","https://raw.githubusercontent.com/SolarCookies/TiP-Recomp/main/src/tip_engine/hooks.cpp","Github").c_str(), 
            NULL, 0, INTERNET_FLAG_RELOAD, 0);

        if (hConnect) {
            std::string responseData;
            char buffer[1024];
            DWORD bytesRead;

            while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                responseData += buffer;
            }

            InternetCloseHandle(hConnect);

            if (!responseData.empty()) {
                std::regex boolPattern("REXCVAR_DEFINE_BOOL\\(([^,]+),\\s*[^,]+,\\s*[^,]+,\\s*\"([^\"]+)\"\\)");
                std::regex intPattern("REXCVAR_DEFINE_INT32\\(([^,]+),\\s*([^,]+),\\s*[^,]+,\\s*\"([^\"]+)\"\\)");
                std::regex intPatternAlt("REXCVAR_DEFINE_INT32\\(([^,]+),\\s*([^,]+),\\s*\"([^\"]+)\"[^)]*\\)");

                std::sregex_iterator boolIter(responseData.begin(), responseData.end(), boolPattern);
                std::sregex_iterator end;

                for (; boolIter != end; ++boolIter) {
                    std::smatch match = *boolIter;
                    std::string patchName = match[1].str();
                    std::string description = match[2].str();

                    patchList.push_back(patchName);
                    patchDescriptions.push_back(description);
                    patchTypes.push_back("bool");
                    patchDefaultValues.push_back(0);
                }

                std::sregex_iterator intIter(responseData.begin(), responseData.end(), intPattern);

                for (; intIter != end; ++intIter) {
                    std::smatch match = *intIter;
                    std::string patchName = match[1].str();
                    std::string defaultValueStr = match[2].str();
                    std::string description = match[3].str();

                    int defaultValue = 0;
                    try {
                        defaultValue = std::stoi(defaultValueStr);
                    } catch (const std::exception& e) {
                      // log or something eventually
                    }

                    patchList.push_back(patchName);
                    patchDescriptions.push_back(description);
                    patchTypes.push_back("int");
                    patchDefaultValues.push_back(defaultValue);
                }

                std::sregex_iterator intIterAlt(responseData.begin(), responseData.end(), intPatternAlt);
                for (; intIterAlt != end; ++intIterAlt) {
                    std::smatch match = *intIterAlt;
                    std::string patchName = match[1].str();
                    std::string defaultValueStr = match[2].str();
                    std::string description = match[3].str();

                    int defaultValue = 0;
                    try {
                        defaultValue = std::stoi(defaultValueStr);
                    } catch (const std::exception& e) {
                      // log or something eventually
                    }

                    bool alreadyFound = false;
                    for (size_t i = 0; i < patchList.size(); i++) {
                        if (patchList[i] == patchName && patchTypes[i] == "int") {
                            alreadyFound = true;
                            break;
                        }
                    }

                    if (!alreadyFound) {
                        patchList.push_back(patchName);
                        patchDescriptions.push_back(description);
                        patchTypes.push_back("int");
                        patchDefaultValues.push_back(defaultValue);
                    }
                }

                std::regex simpleInt32Pattern("REXCVAR_DEFINE_INT32");
                std::sregex_iterator simpleIter(responseData.begin(), responseData.end(), simpleInt32Pattern);
                onlineSuccess = !patchList.empty();
            }
        }
        InternetCloseHandle(hInternet);
    }

    if (!onlineSuccess) {
        LoadPatchesFromINI();
    }
}

void Patches::LoadPatchesFromINI() {
    mINI::INIFile iniFile = mINI::INIFile("config.ini");
    mINI::INIStructure iniData;

    if (iniFile.read(iniData)) {
        if (iniData.has("Patches")) {
            auto& patchesSection = iniData["Patches"];
            for (auto it = patchesSection.begin(); it != patchesSection.end(); ++it) {
                std::string patchName = it->first;

                std::string description = "Patch loaded from config";
                std::string type = "bool";

                if (iniData.has("PatchDescriptions")) {
                    auto& descSection = iniData["PatchDescriptions"];
                    std::string descKey = patchName + "_desc";
                    if (descSection.has(descKey)) {
                        description = descSection[descKey];
                    }
                }
                if (iniData.has("PatchTypes")) {
                    auto& typeSection = iniData["PatchTypes"];
                    std::string typeKey = patchName + "_type";
                    if (typeSection.has(typeKey)) {
                        type = typeSection[typeKey];
                    }
                }

                patchList.push_back(patchName);
                patchDescriptions.push_back(description);
                patchTypes.push_back(type);
                patchDefaultValues.push_back(0);
            }
        }
    }
}

void Patches::Render() {
    if (patchList.empty()) {
        ImGui::Text("No patches available");
        return;
    }

    for (size_t i = 0; i < patchList.size(); i++) {
        const std::string& patchName = patchList[i];
        const std::string& description = patchDescriptions[i];
        const std::string& type = patchTypes[i];
        const int defaultValue = patchDefaultValues[i];

        auto it = std::find_if(patches.begin(), patches.end(), [&](const Patch& p) {
            return p.name == patchName;
        });
        if (it == patches.end()) {
            bool enabledValue = false;
            int intValue = 0;

            if (type == "bool") {
                enabledValue = INI::GetBool(patchName, "Patches", false);
            }
            else if (type == "int") {
                enabledValue = true;
                intValue = INI::GetInt(patchName, defaultValue, "Patches");
            }

            patches.push_back({ patchName, description, type, enabledValue, intValue });
            it = std::prev(patches.end());
        } else {
            it->type = type;
            it->description = description;
            if (type == "int") {
                it->enabled = true;
            }
        }
        if (it->type == "bool") {
            ImGui::BeginChild((patchName + "_child").c_str(), ImVec2(0, 50), true);
            ImGui::Checkbox(patchName.c_str(), &it->enabled);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", it->description.c_str());
            }
            ImGui::EndChild();
        }
        else if (it->type == "int") {
            ImGui::BeginChild((patchName + "_child").c_str(), ImVec2(0, 70), true);
            ImGui::Text("%s", patchName.c_str());
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", it->description.c_str());
            }
            ImGui::InputInt(("##" + patchName).c_str(), &it->intValue, 1, 10, ImGuiInputTextFlags_None);
            if (it->intValue < 0) {
                it->intValue = 0;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", it->description.c_str());
            }
            ImGui::EndChild();
        }
    }
}

std::string Patches::GetPatchLaunchConfig() {
  std::string config;
  for (Patch& patch : patches) {
    if (patch.type == "bool") {
      if (patch.enabled) {
        config += " --" + patch.name + "=true";
        INI::SetBool(patch.name, true, "Patches");
      } else {
        config += " --" + patch.name + "=false";
        INI::SetBool(patch.name, false, "Patches");
      }
    }
    else if (patch.type == "int") {
      config += " --" + patch.name + "=" + std::to_string(patch.intValue);
      INI::SetInt(patch.name, patch.intValue, "Patches");
    }

    INI::SetString(patch.name + "_desc", patch.description, "PatchDescriptions");
    INI::SetString(patch.name + "_type", patch.type, "PatchTypes");
  }

  config += INI::GetString("launch-args", " --gpu_allow_invalid_fetch_constants=true --enable_console=false --scribble_heap=true --vsync=off", "launch");

  return config;
}
