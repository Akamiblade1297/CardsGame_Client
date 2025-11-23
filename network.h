#ifndef NETWORK_H
#define NETWORK_H

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <csignal>
#endif
#include <string>

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

    /**
     * @brief Initialize networking
     */
  static void NetworkInit () {
#ifdef _WIN32
    WSAData wsaData;
    WORD DllVersion = MAKEWORD(2,2);
    if ( WSAStartup(DllVersion, &wsaData) != 0 ) exit(1);
  }
#else
  signal(SIGPIPE, SIG_IGN);
#endif
    }
  /**
   * @brief Connection constructor
   *
   * @param ip IP Address to connect
   * @param port Port to connect
   * @param success Pointer to a Bool value, Result
   */
  Connection ( const char* ip, int port, bool* success ) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if ( sockfd == INVALID_SOCKET )
#else
    if ( sockfd == 0 )
#endif
      goto failed;
    if ( connect( sockfd, (struct sockaddr*)&addr, sizeof(addr) ) == -1 )
      goto failed;

    FD_ZERO(&readfd);
    FD_SET(sockfd, &readfd);
    *success = true;
    return;

  failed:
    *success = false;
  }
  ~Connection () {
#ifdef _WIN32
      closesocket(sockfd);
#else
      close(sockfd);
#endif
  }

  /**
   * @brief Send message to peer
   *
   * @param message Message to send
   * @return Message length if Success, -1 if Failure
   */
  int Send ( std::string message ) {
    return send(sockfd, (message + DEL).data(), message.size()+1, 0);
  }

  /**
   * @brief Receive message from peer
   *
   * @param buffer Buffer to store received message
   * @param bufsize Size of a buffer, Maximum size of received message
   * @return Message length if success, -1 if Failure
   */
  int Receive ( char* buffer, size_t bufsize, int t_sec = -1, int t_usec = -1 ) {
    if ( t_sec > 0 || t_usec > 0 ) {
      timeout.tv_sec = t_sec;
      timeout.tv_usec = t_usec;
      if ( select(sockfd+1, &readfd, NULL, NULL, &timeout) <= 0 ) return -1;
    }
    return recv(sockfd, buffer, bufsize, 0);
  }

  void Close () { this->~Connection(); }
};

#endif // NETWORK_H
