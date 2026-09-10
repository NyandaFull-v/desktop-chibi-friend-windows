#pragma once
#include <windows.h>
#include <atomic>
#include <string>
#include <thread>

class ObsServer {
public:
    ~ObsServer();
    bool start(int preferredPort, const std::wstring& root, int petSize, int coinMinMinutes, int coinMaxMinutes);
    void stop();
    int port() const { return port_; }
private:
    void run();
    std::string responseFor(const std::string& request);
    std::atomic<bool> running_{false};
    std::thread thread_;
    uintptr_t listener_{};
    int port_{}; int petSize_{180},coinMinMinutes_{15},coinMaxMinutes_{30};
    std::wstring root_;
};

bool IsObsRunning();
