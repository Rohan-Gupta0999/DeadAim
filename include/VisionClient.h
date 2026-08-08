#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace deadaim {

// Which gesture the vision process currently recognises. Bow and
// Fireball are declared now because the parser maps them the same way --
// they simply never arrive until those recognisers are built.
enum class GestureType {
    None,
    Gun,
    Bow,
    Fireball
};

// Purpose: what the Python vision process sends across the IPC boundary.
// Deliberately game-facing: no MediaPipe concepts leak into C++.
//
// Note: the JSON also carries the full 21-point landmark list per hand.
// Still intentionally unparsed -- nothing in the game reads it yet.
struct VisionData {
    bool tracking = false;
    int handCount = 0;
    float confidence = 0.f;
    float aimX = 0.5f;
    float aimY = 0.5f;
    GestureType gesture = GestureType::None;
    bool firing = false;
};

// Purpose: the C++ side of the Vision pipeline's IPC boundary.
//
// Responsibilities: connect on a background thread, retrying quietly
// until the vision process is up; parse newline-delimited JSON; store
// only the most recent message, safely, for the main thread to read
// without ever blocking.
//
// Dependencies: nlohmann/json; platform sockets (isolated to the .cpp).
//
// Contract: getLatestData() returns nullopt if nothing has arrived yet
// OR if the last message is older than kStaleAfterMs -- so a dead vision
// process reads as "no data" rather than freezing the crosshair, or
// worse, leaving the gun stuck firing.
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

} // namespace deadaim