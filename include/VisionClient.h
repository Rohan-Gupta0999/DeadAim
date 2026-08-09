#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace deadaim {


enum class GestureType {
    None,
    Gun,
    Bow,
    Fireball
};


struct VisionData {
    bool tracking = false;
    int handCount = 0;
    float confidence = 0.f;
    float aimX = 0.5f;
    float aimY = 0.5f;
    GestureType gesture = GestureType::None;
    bool firing = false;
};


class VisionClient {
public:
    VisionClient();
    ~VisionClient();

    VisionClient(const VisionClient&) = delete;
    VisionClient& operator=(const VisionClient&) = delete;

    void connect(const std::string& host, unsigned short port);
    std::optional<VisionData> getLatestData() const;

private:
    void receiveLoop(std::string host, unsigned short port);

    std::thread m_thread;
    std::atomic<bool> m_running{false};

    mutable std::mutex m_dataMutex;
    std::optional<VisionData> m_latestData;
    std::chrono::steady_clock::time_point m_lastMessageTime;

    static constexpr int kStaleAfterMs = 500;
};

} 