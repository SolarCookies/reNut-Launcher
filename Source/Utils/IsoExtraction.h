#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>

struct IsoExtractionProgress {
    std::atomic<bool> isExtracting{false};
    std::atomic<bool> isComplete{false};
    std::atomic<bool> hasError{false};
    std::string errorMessage;
    std::string lastConsoleOutput;
    std::mutex outputMutex;

    void Reset() {
        isExtracting = false;
        isComplete = false;
        hasError = false;
        errorMessage.clear();
        lastConsoleOutput.clear();
    }
};

namespace IsoExtraction {
    std::string OpenIsoFileDialog();
    
    std::string EscapeForCmd(const std::string& path);
    
    void ExtractIsoAsync(const std::string& isoPath, std::shared_ptr<IsoExtractionProgress> progress);
}
