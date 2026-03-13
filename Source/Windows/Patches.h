#pragma once
#include <vector>
#include <string>

inline std::vector<std::string> patchList;
inline std::vector<std::string> patchDescriptions;
inline std::vector<std::string> patchTypes; // "bool" or "int"
inline std::vector<int> patchDefaultValues; // default values for int patches

struct Patch {
    std::string name;
    std::string description;
    std::string type; // "bool" or "int"
    bool enabled;
    int intValue;
};

inline std::vector<Patch> patches;

class Patches {

  public:
  void Init();
  void LoadPatchesFromINI();
  void Render();
  std::string GetPatchLaunchConfig();
  bool HasPatches() { return !patchList.empty(); }

};
