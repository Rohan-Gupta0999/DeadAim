#include "VisionClient.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <iostream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketHandle = SOCKET;
    constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    using SocketHandle = int;
    constexpr SocketHandle kInvalidSocket = -1;
#endif

namespace deadaim {

namespace {

void closeSocket(SocketHandle socketHandle) {
#ifdef _WIN32
    closesocket(socketHandle);
#else
    close(socketHandle);
#endif
}

GestureType parseGesture(const std::string& name) {
    if (name == "gun")      return GestureType::Gun;
    if (name == "bow")      return GestureType::Bow;
    if (name == "fireball") return GestureType::Fireball;
    return GestureType::None;
}


class PlatformSocketGuard {
public:
    PlatformSocketGuard() {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
    ~PlatformSocketGuard() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

} // namespace

VisionClient::VisionClient() = default;

VisionClient::~VisionClient() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void VisionClient::connect(const std::string& host, unsigned short port) {
    m_running = true;
    m_thread = std::thread(&VisionClient::receiveLoop, this, host, port);
}

std::optional<VisionData> VisionClient::getLatestData() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    if (!m_latestData) {
        return std::nullopt;
    }

    auto age = std::chrono::steady_clock::now() - m_lastMessageTime;
    if (age > std::chrono::milliseconds(kStaleAfterMs)) {
        return std::nullopt; // vision process died or stalled
    }
    return m_latestData;
}

void VisionClient::receiveLoop(std::string host, unsigned short port) {
    static PlatformSocketGuard platformGuard;

    std::string lineBuffer;
    char recvChunk[4096];

    while (m_running) {
        SocketHandle socketHandle = socket(AF_INET, SOCK_STREAM, 0);
        if (socketHandle == kInvalidSocket) {
            std::cerr << "[VisionClient] Failed to create socket.\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &serverAddress.sin_addr);

        if (::connect(socketHandle, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) != 0) {
            closeSocket(socketHandle);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        std::cerr << "[VisionClient] Connected to vision process.\n";

#ifdef _WIN32
        DWORD timeoutMs = 500;
        setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
#else
        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 500 * 1000;
        setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif

        while (m_running) {
            int bytesReceived = recv(socketHandle, recvChunk, sizeof(recvChunk), 0);

            if (bytesReceived > 0) {
                lineBuffer.append(recvChunk, static_cast<std::size_t>(bytesReceived));

                std::size_t newlinePos;
                while ((newlinePos = lineBuffer.find('\n')) != std::string::npos) {
                    std::string line = lineBuffer.substr(0, newlinePos);
                    lineBuffer.erase(0, newlinePos + 1);

                    try {
                        nlohmann::json parsed = nlohmann::json::parse(line);

                        VisionData data;
                        data.tracking = parsed.value("tracking", false);
                        data.handCount = parsed.value("hand_count", 0);
                        data.confidence = parsed.value("confidence", 0.f);
                        if (parsed.contains("aim")) {
                            data.aimX = parsed["aim"].value("x", 0.5f);
                            data.aimY = parsed["aim"].value("y", 0.5f);
                        }
                        data.gesture = parseGesture(parsed.value("gesture", std::string("none")));
                        data.firing = parsed.value("firing", false);

                        std::lock_guard<std::mutex> lock(m_dataMutex);
                        m_latestData = data;
                        m_lastMessageTime = std::chrono::steady_clock::now();
                    } catch (const nlohmann::json::exception&) {
                        // Malformed line -- skip rather than crash the game.
                    }
                }
            } else if (bytesReceived == 0) {
                break; // server closed the connection cleanly
            }
        }

        closeSocket(socketHandle);
        if (m_running) {
            std::cerr << "[VisionClient] Disconnected -- retrying...\n";
        }
    }
}

}