#ifndef NETWORK_H
#define NETWORK_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <csignal>
#define SOCKET int
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif
#include <string>
#include <mutex>
#include <chrono>
#include <cstring>
#include <atomic>
#include <thread>
#include <vector>

#define DEL '\n'

/**
 * @brief Socket wrapper
 */
class Connection {
private:
  int sockfd;
  fd_set readfd;
  sockaddr_in addr;
  struct timeval timeout;
public:
  struct timeval timeout_opt;

 /**
  * @brief Initialize networking
  */
  static void NetworkInit () {
#ifdef _WIN32
    WSAData wsaData;
    WORD DllVersion = MAKEWORD(2,2);
    if ( WSAStartup(DllVersion, &wsaData) != 0 ) exit(1);
#else
  signal(SIGPIPE, SIG_IGN);
#endif
  }

  /**
   * @brief Cleanup networking
   */
  static void NetworkClean () {
#ifdef _WIN32
    WSACleanup();
#endif
  }

  /**
   * @brief Connection constructor
   *
   * @param ip IP Address to connect
   * @param port Port to connect
   * @param success Pointer to a Bool value, Result
   */
  Connection ( const char* ip, unsigned short port, bool* success );

  ~Connection ();
  /**
   * @brief Send message to peer
   *
   * @param message Message to send
   * @return Message length if Success, -1 if Failure
   */
  int Send ( std::string message );

  /**
   * @brief Receive message from peer
   *
   * @param buffer Buffer to store received message
   * @param bufsize Size of a buffer, Maximum size of received message
   * @return Message length if success, -1 if Failure
   */
  int Receive ( char* buffer, size_t bufsize, int t_sec = -1, int t_usec = -1 );

  void Close ();

  std::string Address();
};

/**
 * @brief Vibecoded server scanner :p
 */
namespace ServerScanner {

    // Check a single IP address for the server
    bool checkServer(uint32_t ipAddress, uint16_t port);

    // Worker function for thread pool
    void workerThread(const std::vector<uint32_t>& ipList, std::vector<uint32_t>& results,
                      size_t startIdx, size_t endIdx);

    std::vector<uint32_t> scanSubnet(uint32_t subnetAddress, uint32_t subnetMask,
                                     unsigned int maxThreads = 50);
    void stop();
}

#endif // NETWORK_H
