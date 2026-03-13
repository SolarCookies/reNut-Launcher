#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <sstream>

class LogSystem {
private:
    static LogSystem* instance;
    std::vector<std::string> logLines;
    std::mutex logMutex;
    static const size_t maxLogLines = 1000;

    LogSystem() = default;

public:
    static LogSystem& GetInstance();
    
    void AddLog(const std::string& message);
    void Clear();
    void Render(bool* showLogWindow = nullptr);
    
    std::vector<std::string> GetLogLines();
};

class LogStreambuf : public std::streambuf {
private:
    std::ostringstream buffer;
    std::streambuf* originalBuffer;

public:
    LogStreambuf(std::streambuf* original);
    ~LogStreambuf();

protected:
    virtual int overflow(int c) override;
    virtual int sync() override;
};
