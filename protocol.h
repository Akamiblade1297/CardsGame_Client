#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "network.h"
#include "main.h"
#include <queue>
#include <string>
#include <stdexcept>

#define DEL "\n"
#define BUFSIZE 1024

namespace Protocol {

class ProtocolError : public std::runtime_error {
public:
  ProtocolError(const std::string& message) : std::runtime_error(message) {}
};

char buffer[BUFSIZE] = {0};
bool joined = false;
Connection conn;
std::queue<std::string> received;

char* Join ( const char* username ) {
  if ( ! joined ) {
    conn.Send("JOIN" + DEL + username);
    joined = true;
    conn.Receive(buffer, BUFSIZE);
  } else throw ProtocolError("Can't join multiple times");
}

void Rejoin ( const char* pass ) {
    conn.Send("REJOIN" + DEL + pass);
}

#endif // PROTOCOL_H
