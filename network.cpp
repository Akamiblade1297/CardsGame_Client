#include "network.h"

/**
 * @brief Vibecoded server scanner :p
 */
namespace {
    std::mutex resultMutex;
    std::atomic<bool> stopScan{false};
}

namespace ServerScanner {

    // Check a single IP address for the server
     bool checkServer(uint32_t ipAddress, uint16_t port) {
        // Create socket
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return false;

        // Set socket to non-blocking for timeout control
#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);
#else
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

        // Setup server address
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = htonl(ipAddress);

        // Try to connect with timeout
        connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        fd_set fdset;
        struct timeval tv;
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);
        tv.tv_sec = 0;
        tv.tv_usec = 500; // 500 microseconds timeout

        bool isConnected = false;
        if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1) {
            int so_error;
            socklen_t len = sizeof(so_error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len);
            isConnected = (so_error == 0);
        }

        if (!isConnected) {
            closesocket(sock);
            return false;
        }

        // Set back to blocking for data reception
#ifdef _WIN32
        mode = 0;
        ioctlsocket(sock, FIONBIO, &mode);
#else
        fcntl(sock, F_SETFL, flags);
#endif

        // Try to receive the server banner
        char buffer[14] = {0};  // 13 bytes + null terminator
        int totalReceived = 0;
        auto startTime = std::chrono::steady_clock::now();

        while (totalReceived < 13) {
            if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(2)) {
                closesocket(sock);
                return false;
            }

            int received = recv(sock, buffer + totalReceived, 13 - totalReceived, 0);
            if (received <= 0) {
                closesocket(sock);
                return false;
            }
            totalReceived += received;
        }

        closesocket(sock);

        // Verify the received string matches "NOT_MUCNHKIN\n"
        return strcmp(buffer, "NOT_MUNCHKIN\n") == 0;
    }

    // Worker function for thread pool
    void workerThread(const std::vector<uint32_t>& ipList, std::vector<uint32_t>& results,
                      size_t startIdx, size_t endIdx) {
        for (size_t i = startIdx; i < endIdx && !stopScan; ++i) {
            if (checkServer(ipList[i], 8494)) {
                std::lock_guard<std::mutex> lock(resultMutex);
                results.push_back(ipList[i]);
            }
        }
    }

    // Main scanning function
     std::vector<uint32_t> scanSubnet(uint32_t subnetAddress, uint32_t subnetMask,
                                     unsigned int maxThreads) {
        std::vector<uint32_t> serversFound;

        // Calculate network and broadcast addresses[citation:1]
        if ( subnetAddress == 0x7F000001 ) {
            if ( checkServer(subnetAddress, 8494) )
                return {subnetAddress};
            else
                return {};
        }
        uint32_t networkAddress = subnetAddress & subnetMask;
        uint32_t broadcastAddress = subnetAddress | (~subnetMask);

        // Generate list of all IPs in the subnet (excluding network and broadcast)
        std::vector<uint32_t> ipList;
        for (uint32_t ip = networkAddress + 1; ip < broadcastAddress; ++ip) {
            ipList.push_back(ip);
        }

        // Limit threads to reasonable number
        unsigned int numThreads = std::min(maxThreads, std::thread::hardware_concurrency());
        numThreads = std::min(numThreads, (unsigned int)ipList.size());

        if (numThreads == 0) numThreads = 1;

        // Calculate IPs per thread
        size_t ipsPerThread = ipList.size() / numThreads;
        std::vector<std::thread> threads;

        // Create worker threads
        for (unsigned int i = 0; i < numThreads; ++i) {
            size_t startIdx = i * ipsPerThread;
            size_t endIdx = (i == numThreads - 1) ? ipList.size() : startIdx + ipsPerThread;

            threads.emplace_back(&ServerScanner::workerThread,
                                std::cref(ipList), std::ref(serversFound), startIdx, endIdx);
        }

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }

        return serversFound;
    }

    // Stop scanning prematurely
     void stop() {
        stopScan = true;
    }
}
