#include "network.h"


Connection::Connection ( const char* ip, unsigned short port, bool* success ) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd == INVALID_SOCKET || connect( sockfd, (struct sockaddr*)&addr, sizeof(addr) ) == -1 ) {
        *success = false;
        return;
    }

    FD_ZERO(&readfd);
    FD_SET(sockfd, &readfd);
    timeout_opt.tv_sec = 0;
    timeout_opt.tv_usec = 50;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_opt, sizeof(timeout_opt));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_opt, sizeof(timeout_opt));
    *success = true;
}

Connection::~Connection () {
#ifdef _WIN32
      shutdown(sockfd, SD_BOTH);
#else
      shutdown(sockfd, SHUT_RDWR);
#endif
      closesocket(sockfd);
}

int Connection::Send ( std::string message ) {
    return send(sockfd, (message + DEL).data(), message.size()+1, 0);
}

int Connection::Receive ( char* buffer, size_t bufsize, int t_sec, int t_usec ) {
    if ( t_sec > 0 || t_usec > 0 ) {
      timeout.tv_sec = (t_sec >= 0) ? t_sec : 0;
      timeout.tv_usec = (t_usec >= 0) ? t_usec : 0;
      if      ( select(sockfd+1, &readfd, NULL, NULL, &timeout) <= 0 ) return -1;
    } else if ( select(sockfd+1, &readfd, NULL, NULL, NULL) <= 0 ) return -1;
    return recv(sockfd, buffer, bufsize, 0);
}

std::string Connection::Address() {
      char ip[INET_ADDRSTRLEN];

      inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
      unsigned short port = ntohs(addr.sin_port);

      return std::string(ip)+':'+std::to_string(port);
}

void Connection::Close () { this->~Connection(); }

namespace {
    std::mutex resultMutex;
    std::atomic<bool> stopScan{false};
}

/**
 * @brief Vibecoded server scanner :p
 */
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
        tv.tv_sec = 2;
        tv.tv_usec = 0;

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
    void workerThread(uint32_t startIp, uint32_t ipCount, std::function<void(uint32_t)> func ) {
        for (uint32_t i = 0; i < ipCount ; i++) {
            uint32_t ip = startIp+i;
            if (checkServer(ip, 8494)) {
                std::lock_guard<std::mutex> lock(resultMutex);
                func(ip);
            }
        }
    }

    // Main scanning function
     void scanSubnet(uint32_t subnetAddress, uint32_t subnetMask, std::function<void(uint32_t)> func,
                                     unsigned int maxThreads) {
         stopScan = false;

        // Calculate network and broadcast addresses[citation:1]
        if ( subnetAddress == 0x7F000001 ) {
            if ( checkServer(subnetAddress, 8494) )
                func(subnetAddress);
            return;
        }
        uint32_t networkAddress = subnetAddress & subnetMask;
        uint32_t broadcastAddress = subnetAddress | (~subnetMask);
        uint32_t totalIps = ( broadcastAddress - networkAddress - 1 );

        // Limit threads to reasonable number
        unsigned int numThreads = std::min(maxThreads, std::thread::hardware_concurrency());
        numThreads = std::min(numThreads, totalIps);
        if (numThreads == 0) numThreads = 1;

        // Calculate IPs per thread
        uint32_t ipsPerThread = totalIps / numThreads;
        std::vector<std::thread> threads;

        // Create worker threads
        for (unsigned int i = 0; i < numThreads; ++i) {
            uint32_t startIp = (networkAddress+1) + (i*ipsPerThread);
            uint32_t ipCount = std::min((broadcastAddress - startIp), ipsPerThread);

            threads.emplace_back(&ServerScanner::workerThread, startIp, ipCount, func);
        }

        // Detach all threads
        for ( std::thread& thread : threads ) {
            thread.detach();
        }
    }

    // Stop scanning prematurely
     void stop() {
        stopScan = true;
    }
}
