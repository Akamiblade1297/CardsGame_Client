#ifndef NETWORK_H
#define NETWORK_H

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <csignal>
#endif

/**
 * @brief Socket wrapper
 */
class Connection {
private:
  int sockfd;
  sockaddr_in addr;
public:
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
    if ( sockfd == 0 )
      goto failed;
    if ( connect( sockfd, (struct sockaddr*)&addr, sizeof(addr) ) == -1 )
      goto failed;

    *success = true;
    return;

  failed:
    *success = false;
  }

  /**
   * @brief Send message to peer
   *
   * @param message Message to send
   * @return Message length if Success, -1 if Failure
   */
  int Send ( std::string message ) {
    return send(sockfd, message.data(), message.size(), 0);
  }

  /**
   * @brief Receive message from peer
   *
   * @param buffer Buffer to store received message
   * @param bufsize Size of a buffer, Maximum size of received message
   * @return Message length if success, -1 if Failure
   */
  int Receive ( char* buffer, size_t bufsize ) {
    return recv(sockfd, buffer, bufsize, 0);
  }
};

/**
 * @brief Initialize networking
 */
void NetworkInit () {
#ifdef _WIN32
  WSAData wsaData;
  WORD DllVersion = MAKEWORD(2,2);
  if ( WSAStartup(DllVersion, &wsaData) != 0 ) {
    exit(1);
  }
#else
  signal(SIGPIPE, SIG_IGN);
#endif
}

#endif // NETWORK_H
