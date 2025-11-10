#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "functions.cpp"
#include "string_func.cpp"
#include "network.h"
#include "main.h"
#include <deque>
#include <string>
#include <cstring>
#include <stdexcept>
#include <random>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <bitset>

#define DEL '\n'
#define BUFSIZE 1024

#define CHECK_SIZE if ( received.size() < size ) {  return; }

namespace {
    void _receiver ();
    void _join ();
}

namespace protocol {
    class ProtocolError : public std::runtime_error {
    public:
        ProtocolError( const std::string& message = "Unexpected Response" ) : std::runtime_error(message) {}
    };

    enum ErrorCode {
        NONMATCH   = -8,
        NORESPONSE = -7,
        EMPTY      = -6,
        OUT_OF_RNG = -5,
        NOT_A_NUM  = -4,
        NOT_FOUND  = -3,
        TIMEOUT    = -2,
        BAD_REQ    = -1,
        NOERROR    =  0,
        RENAMED    =  1,
    };
    enum FLAG {
        JOINED    = 0,
        PING      = 1,
    };

    char buffer[BUFSIZE] = {0};
    char ping[9] = {0};
    std::bitset<2> flags = {0};
    Connection* conn;
    std::deque<std::string> received;
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<uint64_t> dis;

    /**
     * @brief Join
     * @param username Player Username
     * @param response Pointer to a result response
     * @return NOERROR(8 bytes pass), RENAMED(New username, LF, 8 bytes pass)
     */
    ErrorCode Join ( const char* username, char* result ) {
        if ( flags[JOINED] ) throw ProtocolError("Can't join multiple times");

        std::string temp = "JOIN";
        conn->Send(temp + DEL + username + DEL);
        int len = conn->Receive(buffer, BUFSIZE);
        buffer[len] = 0;
        std::vector<std::string> response = split(buffer, DEL);

        if ( response.size() >= 2 && response[0] == "OK" ) {
            std::memcpy(result, response[1].data(), 8);
            for ( int i = 2 ; i < response.size() ; i++ ) received.push_back(response[i]);
            _join();

            return NOERROR;
        } else if ( response.size() >= 3 && response[0] == "RENAMED" ) {
            std::string res = response[1]+response[2];
            std::memcpy(result, &res, len-8);
            for ( int i = 3 ; i < response.size() ; i++ ) received.push_back(response[i]);
            _join();

            return RENAMED;
        } else throw ProtocolError();
    }
    /**
     * @brief Rejoin
     * @param pass Player pass
     * @return NOERROR(Player rejoined successfully), NOT_FOUND(No such player with given pass)
     */
    ErrorCode Rejoin ( const char* pass ) {
        if ( flags[JOINED] ) throw ProtocolError("Can't join multiple times");
        std::string temp = "REJOIN";
        conn->Send(temp + DEL + pass + DEL);
        int len = conn->Receive(buffer, BUFSIZE);
        buffer[len] = 0;
        std::vector<std::string> response = split(buffer, DEL);

        if ( response[0] == "OK" ) {
            for ( int i = 1 ; i < response.size() ; i++ ) received.push_back(response[i]);
            _join();
            return NOERROR;
        }
        else if ( response[0] == "NOT FOUND" ) return NOT_FOUND;
        else throw ProtocolError();
    }
    /**
     * @brief Ping
     * @return NOERROR(Received Pong), TIMEOUT(Didn't receive Pong)
     */
    ErrorCode Ping () {
        uint64_t dat = dis(rng); char n = 0;
        char data[9] = {0};
        std::memcpy(data, &dat, 8);

        std::string temp = "PING";
        conn->Send(temp + DEL + data + DEL);

        char timeout = 10;
        while ( !flags[PING] ) {
            if ( timeout > 0 ) {
                timeout-=1;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } else return TIMEOUT;
        }
        if ( std::strcmp(data, ping) ) return NOERROR;
        throw ProtocolError();
    }
}

using namespace protocol;
namespace {
    void _worker () {
        uint8_t size;
        while ( !received.empty() ) {
            if ( received[0] == "JOIN" ) { // JOIN <player>
                size = 2; CHECK_SIZE;

                Player new_player(received[1]);
                playerMgr.Players.push_back(new_player);
            } else if ( received[0] == "CHAT" ) { // CHAT <message>
                size = 2; CHECK_SIZE;
            } else if ( received[0] == "ACT" ) { // ACT <action>
                size = 2; CHECK_SIZE;
            } else if ( received[0] == "ROLL" ) { // ROLL <player> <dice> <n> <r1> <r2> <r3> ... <rn>
                size = 4; CHECK_SIZE;
                size += std::stoi(received[3]); CHECK_SIZE;
            } else if ( received[0] == "WHISPER" ) { // WHISPER <sender> <message>
                size = 3; CHECK_SIZE;
            } else if ( received[0] == "PONG" ) { // PONG <data>
                size = 2; CHECK_SIZE;
                
                std::memcpy(ping, received[1].data(), 8);
                flags[PING] = true;
            } else if (  received[0] == "NOTIFY " ) { // NOTIFY <player> ...
                size = 3; CHECK_SIZE;
                Player* sender = playerMgr.playerByName(received[1]) ;
                constexpr int i = 2;
                if ( sender == nullptr ) throw ProtocolError();

                if ( received[i] == "DECK" ) { // DECK <deck src> <dest> <x> <y> <card>
                    size += 5; CHECK_SIZE;
                    Deck* src = deckByName(received[i+ 1]);
                    CardContainer* dest = containerByName(received[i+ 2], sender);
                    
                    if ( src->pop_and_move(received[i+ 3], received[i+ 4], dest, received[i+ 5]) != 0 ) throw ProtocolError("Failed to 6");
                } else if ( received[i] == "MOVE" ) { // MOVE <spatial src> <card_id> <dest> <x> <y> <card>
                    size += 6; CHECK_SIZE;
                    CardContainer* src     = containerByName(received[i+ 1], sender);
                    CardContainer* dest    = containerByName(received[i+ 3], sender); 

                    src->move(received[i+ 2], received[i+ 4], received[i+ 5], dest, received[i+ 6]);
                } else if ( received[i] == "ROTATE" ) { // ROTATE <spatial> <card_id> <rot>
                    size += 3; CHECK_SIZE;
                    CardContainer* container = spatialByName(received[i+ 1], sender);
                    container->rotate(received[i+ 2], received[i+ 3]);
                } else if ( received[i] == "FLIP" ) { // FLIP <spatial> <card_id> <card>
                    size += 3; CHECK_SIZE;
                    CardContainer* container = spatialByName(received[i+ 1], sender);
                    Card* card = (*container)[received[i+ 2]];

                    card->parse_card(received[i+ 3]);
                } else if ( received[i] == "SHUFFLE" ) { // SHUFFLE <deck>
                    size += 1; CHECK_SIZE;
                } else if ( received[i] == "SET" ) { // SET <stat> <value>
                    size += 2; CHECK_SIZE;
                    std::string* sStat = sStatByName(received[i+ 1], sender);
                    int* iStat = iStatByName(received[i+ 1], sender);
                    if ( sStat == nullptr ) {
                        if ( iStat == nullptr ) throw ProtocolError("Bad Stat");
                        *iStat = std::stoi(received[i+ 2]);
                    } else *sStat = received[i+ 2];
                } else if ( received[i] == "RENAME" ) { // RENAME <new_name>
                    size += 1; CHECK_SIZE;
                    sender->Name = received[i+ 1];
                }
            }
        }
        for ( int _ = 0 ; _ < size ; _++ ) received.pop_front();
    }
    void _receiver () {
        if ( ! flags[JOINED] ) throw ProtocolError("Can't launch a receiver on unestablished connection");
        while (1) {
            int len = conn->Receive(buffer, BUFSIZE);
            if ( len <= 0 ) {
                flags[JOINED] = false;
                return;
            }
            buffer[len] = 0;
            std::vector<std::string> recv = split(buffer, DEL);
            for ( int i = 0 ; i < recv.size() ; i++ ) received.push_back(recv[i]);
            _worker();
        }
    }
    void _join ( std::string username ) {
        if ( flags[JOINED] ) throw ProtocolError("Can't join, while joined already");
        flags[JOINED] = true;
        std::thread receiver_thread(_receiver);

        receiver_thread.detach();
    }
}
#endif // PROTOCOL_H
