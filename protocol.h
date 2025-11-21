#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "functions.cpp"
#include "string_func.cpp"
#include "network.h"
#include "main.h"
#include <deque>
#include <mutex>
#include <string>
#include <cstring>
#include <stdexcept>
#include <random>
#include <cstdlib>
#include <sys/socket.h>
#include <thread>
#include <chrono>
#include <bitset>

#define BUFSIZE 1024

#define CHECK_SIZE if ( received.size() < size ) {  return; }

namespace {
    enum FLAG {
        JOINED = 0,
        PONG_R = 1,

        WHIS_R = 2,
        WHIS_S = 3,

        ROLL_R = 4,
        ROLL_S = 5,

        DECK_R = 6,

        MOVE_R = 7,

        ROT_R  = 8,

        FLIP_R = 9,

        SHUF_R = 10,
        SHUF_S = 11,

        SET_R  = 12,

        SEE_R  = 13,
        SEE_S  = 14,

        CRDS_R = 15,
        CRDS_S = 16,

        STAT_R = 17,
        STAT_S = 18,

        PLRS_R = 19,
    };
    std::bitset<20> flags = {0};

    int8_t ErDeck;
    int8_t ErMove;
    int8_t ErRot;
    int8_t ErFlip;
    int8_t ErSet;

    std::mutex muPing;
    std::mutex muWhisper;
    std::mutex muRoll;
    std::mutex muDeck;
    std::mutex muMove;
    std::mutex muRot;
    std::mutex muFlip;
    std::mutex muShuffle;
    std::mutex muSet;
    std::mutex muSee;
    std::mutex muCards;
    std::mutex muStat;
    std::mutex muPlayers;

    char ping_data[9] = {0};
    uint8_t* _rolls;
    std::vector<Card>* _see;
    std::vector<Card>* _cards;
    std::vector<std::string>* _stats;
    std::vector<std::string>* _players;

    Player* localplayer;
    Connection* conn;

    char buffer[BUFSIZE] = {0};
    std::deque<std::string> received;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<uint64_t> dis;

    void _receiver ();
    void _join ();
}

namespace protocol {
    enum CritErrorCode {
        CRIT__UNEXPECTED_RESPONSE,
        CRIT__INVALID_PLAYER, 
        CRIT__ALREADY_JOINED,
        CRIT__BAD_CONTAINER,
        CRIT__NOT_JOINED,
        CRIT__EMPTY_DECK,
        CRIT__NUM_ERROR,
        CRIT__BAD_CARD,
        CRIT__BAD_STAT,
    };
    class CriticalError : public std::runtime_error {
        private:
            CritErrorCode code;
            static std::string str_code ( CritErrorCode cod ) {
                switch (cod) {
                    case CRIT__UNEXPECTED_RESPONSE:
                        return "UNEXPECTED_RESPONSE";
                    case CRIT__INVALID_PLAYER:
                        return "INVALID_PLAYER";
                    case CRIT__ALREADY_JOINED:
                        return "ALREADY_JOINED";
                    case CRIT__BAD_CONTAINER:
                        return "BAD_CONTAINER";
                    case CRIT__NOT_JOINED:
                        return "NOT_JOINED";
                    case CRIT__EMPTY_DECK:
                        return "EMPTY_DECK";
                    case CRIT__NUM_ERROR:
                        return "NUM_ERROR";
                    case CRIT__BAD_STAT:
                        return "BAD_STAT";
                    case CRIT__BAD_CARD:
                        return "BAD_CARD";
                }
            }
        public:
            CriticalError ( CritErrorCode cod, std::string breakpoint ) : code(cod), std::runtime_error(str_code(cod)+' '+breakpoint) {
                flags[JOINED] = false;
                conn->Close();
            }
    };

    enum ErrorCode {
        PROTOCOL_ERR =-10,
        SEND_ERROR   = -9,
        NONMATCH     = -8,
        NORESPONSE   = -7,
        EMPTY        = -6,
        OUT_OF_RNG   = -5,
        NOT_A_NUM    = -4,
        NOT_FOUND    = -3,
        TIMEOUT      = -2,
        BAD_REQ      = -1,
        NOERROR      =  0,
        RENAMED      =  1,
    };

    /**
     * @brief Join
     * @param socket Connection socket
     * @param username Player Username
     * @param response Pointer to a result response
     * @return NOERROR, RENAMED, SEND_ERROR, PROTOCOL_ERR
     */
    ErrorCode join ( Connection* socket, const char* username, char* result ) {
        if ( flags[JOINED] ) return PROTOCOL_ERR;

        conn = socket;
        std::string temp = "JOIN";
        if ( conn->Send(temp + DEL + username) == -1 ) return SEND_ERROR;
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
        } else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "join");
    }
    /**
     * @brief Rejoin
     * @param socket Connection socket
     * @param pass Player pass
     * @return NOERROR, NOT_FOUND, SEND_ERROR, PROTOCOL_ERR
     */
    ErrorCode rejoin ( Connection* socket, const char* pass ) {
        if ( flags[JOINED] ) return PROTOCOL_ERR;

        conn = socket;
        std::string temp = "REJOIN";
        if ( conn->Send(temp + DEL + pass) == -1 ) return SEND_ERROR;
        int len = conn->Receive(buffer, BUFSIZE);
        buffer[len] = 0;
        std::vector<std::string> response = split(buffer, DEL);

        if ( response[0] == "OK" ) {
            for ( int i = 1 ; i < response.size() ; i++ ) received.push_back(response[i]);
            _join();
            return NOERROR;
        }
        else if ( response[0] == "NOT FOUND" ) return NOT_FOUND;
        else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "rejoin");
    }
    /**
     * @brief Ping
     * @param timeout Connection timeout in secconds
     * @return NOERROR, TIMEOUT, SEND_ERROR
     */
    ErrorCode ping ( int timeout = 10 ) {
        uint64_t dat = dis(rng);
        char data[9] = {0};
        std::memcpy(data, &dat, 8);

        std::lock_guard<std::mutex> lock(muPing);
        std::string temp = "PING";
        if ( conn->Send(temp + DEL + data) == -1 ) return SEND_ERROR;

        flags[PONG_R] = false;
        while ( !flags[PONG_R] ) {
            if ( timeout > 0 ) {
                timeout-=1;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } else return TIMEOUT;
        }
        if ( std::strcmp(data, ping_data) == 0 ) return NOERROR;
        else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "ping");
    }
    
    /**
     * @brief Send message to Chat
     * @param msg Message to send
     * @return NOERROR, SEND_ERROR
     */
    ErrorCode chat ( std::string msg ) {
        std::string temp = "CHAT";
        if ( conn->Send(temp + DEL + msg) == -1 ) return SEND_ERROR;
        return NOERROR;
    }
    /**
     * @brief Send action to Chat
     * @param act Action to send
     * @return NOERROR, SEND_ERROR
     */
    ErrorCode action ( std::string act ) {
        std::string temp = "ACT";
        if ( conn->Send(temp + DEL + act) == -1 ) return SEND_ERROR;
        return NOERROR;
    }
    /**
     * @brief Whisper message to playert
     * @param player Receiver
     * @param msg Message to whisper
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode whisper ( std::string player, std::string msg ) {
        std::lock_guard<std::mutex> lock(muWhisper);       
        std::string temp = "WHISPER";
        int res = conn->Send(temp + DEL + player + DEL + msg);

        if ( res == -1 ) return SEND_ERROR;
        flags[WHIS_R] = false;
        while ( !flags[WHIS_R] ) {}

        if ( !flags[WHIS_S] ) return NOT_FOUND;
        return NOERROR;
    }
    /**
     * @brief Roll a dice
     * @param rolls Pointer to result array of rolled numbers
     * @param dice Number of sides of each dice
     * @param num Number of dices to roll (1-10)
     * @return NOERROR, OUT_OF_RNG, NOT_A_NUM, SEND_ERROR
     */
    ErrorCode roll ( uint8_t* rolls, std::string dice = "6", std::string num = "1" ) {
        std::lock_guard<std::mutex> lock(muRoll);
        std::string temp = "ROLL";
        _rolls = rolls;
        if ( conn->Send(temp + DEL + dice + DEL + num) == -1 ) return SEND_ERROR;
        
        flags[ROLL_R] = false;
        while ( !flags[WHIS_R] ) {}

        if ( flags[WHIS_S] ) return NOERROR;
        else return (ErrorCode)*rolls;
    }
    /**
     * @brief Pop card from Deck
     * @param src Source deck
     * @param dest Destinition container
     * @param x New X coordinate
     * @param y New Y coordinate
     * @return NOERROR, EMPTY, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode deck ( std::string src, std::string dest, std::string x, std::string y ) {
        std::lock_guard<std::mutex> lock(muDeck);
        std::string temp = "DECK";
        if ( conn->Send(temp + DEL + src + DEL + x + DEL + y) == -1 ) return SEND_ERROR;
        
        flags[DECK_R] = false;
        while ( !flags[DECK_R] ) {}
        return (ErrorCode)ErDeck;
    }
    /**
     * @brief Move card from Spatial
     * @param src Source spatial
     * @param card_id Card id inside Source container
     * @param dest Destinition container
     * @param x New X coordinate
     * @param y New Y coordinate
     * @return NOERROR, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode move ( std::string src, std::string card_id, std::string dest, std::string x, std::string y ) {
        std::lock_guard<std::mutex> lock(muMove);
        std::string temp = "MOVE";
        if ( conn->Send(temp + DEL + src + DEL + card_id + DEL + dest + DEL + x + DEL + y) == -1 ) return SEND_ERROR;

        flags[MOVE_R] = false;
        while ( !flags[MOVE_R] ) {}
        return (ErrorCode)ErMove;
    }
    /**
     * @brief Rotate card in Spatial
     * @param spatial Container
     * @param card_id Card id inside Container
     * @param rot New rotation angle
     * @return NOERROR, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode rotate ( std::string spatial, std::string card_id, std::string rot) {
        std::lock_guard<std::mutex> lock(muRot);
        std::string temp = "ROTATE";
        if ( conn->Send(temp + DEL + spatial + DEL + card_id + DEL + rot ) == -1 ) return SEND_ERROR;

        flags[ROT_R] = false;
        while ( !flags[ROT_R] ) {}
        return (ErrorCode)ErRot;
    }
    /**
     * @brief Flip card in Spatial
     * @param spatial Container
     * @param card_id Card id insite Container
     * @return NOERROR, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode flip ( std::string spatial, std::string card_id ) {
        std::lock_guard<std::mutex> lock(muFlip);
        std::string temp = "FLIP";
        if ( conn->Send(temp + DEL + spatial + DEL + card_id ) == -1 ) return SEND_ERROR;

        flags[FLIP_R] = false;
        while ( !flags[FLIP_R] ) {}
        return (ErrorCode)ErFlip;
    }
    /**
     * @brief Shuffle deck
     * @param deck Deck to shuffle
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode shuffle ( std::string deck ) {
        std::lock_guard<std::mutex> lock(muShuffle);
        std::string temp = "SHUFFLE";
        if ( conn->Send(temp + DEL + deck ) == -1 ) return SEND_ERROR;

        flags[SHUF_R] = false;
        while ( !flags[SHUF_R] ) {}
        return flags[SHUF_S] ? NOERROR : NOT_FOUND;
    }
    /**
     * @brief Set localplayer stat
     * @param stat Stat to set
     * @param value Value to set
     * @return NOERROR, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode set ( std::string stat, std::string value ) {
        std::lock_guard<std::mutex> lock(muSet);
        std::string temp = "SET";
        if ( conn->Send(temp+DEL+stat+DEL+value ) == -1) return SEND_ERROR;

        flags[SET_R] = false;
        while ( !flags[SET_R] ) {}
        return (ErrorCode)ErSet;
    }
    /**
     * @brief Rename localplayer
     * @param name New name
     * @return NOERROR, SEND_ERROR
     */
    ErrorCode rename ( std::string name ) {
        std::string temp = "RENAME";
        if ( conn->Send(temp+DEL+name ) == -1 ) return SEND_ERROR;
        else return NOERROR;
    }
    /**
     * @brief Get all cards in visible containers
     * @param container Container
     * @param res Cards array to store gotten cards
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode see ( std::string container, std::vector<Card>* res ) {
        std::lock_guard<std::mutex> lock(muSee);
        _see = res;

        std::string temp = "SEE";
        if ( conn->Send(temp+DEL+container ) == -1 ) return SEND_ERROR;

        flags[SEE_R] = false;
        while ( ! flags[SEE_R] ) {}
        return flags[SEE_S] ? NOERROR : NOT_FOUND;
    }
    /**
     * @brief Get all cards in non-visible containers
     * @param container Container
     * @param res Cards array to store gotten cards
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode cards ( std::string container, std::vector<Card>* res ) {
        std::lock_guard<std::mutex> lock(muCards);
        _cards = res;

        std::string temp = "SEE";
        if ( conn->Send(temp+DEL+container ) == -1 ) return SEND_ERROR;

        flags[CRDS_R] = false;
        while ( ! flags[CRDS_R] ) {}
        return flags[CRDS_S] ? NOERROR : NOT_FOUND;
    }
    /**
     * @brief Get all stats of a player
     * @param player Player
     * @param res Array of stats values
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode stat ( std::string player, std::vector<std::string>* res ) {
        std::lock_guard<std::mutex> lock(muStat);
        _stats = res;

        std::string temp = "STAT";
        if ( conn->Send(temp+DEL+player ) == -1 ) return SEND_ERROR;

        flags[STAT_R] = false;
        while ( ! flags[STAT_R] ) {}
        return flags[STAT_S] ? NOERROR : NOT_FOUND;
    }
    /**
     * @brief Get all players
     * @param res Array of player names
     * @return NOERROR, SEND_ERROR
     */
    ErrorCode players ( std::vector<std::string>* res ) {
        std::lock_guard<std::mutex> lock(muPlayers);
        _players = res;

        if ( conn->Send("PLAYERS") == -1 ) return SEND_ERROR;

        flags[PLRS_R] = false;
        while ( ! flags[PLRS_R] ) {}
        return NOERROR;
    }
}

using namespace protocol;
namespace {
    void _worker () {
        uint8_t size;
        while ( !received.empty() ) {
            if ( received[0] == "JOIN" ) { // JOIN <player>
                size = 2; CHECK_SIZE;

                if ( !playerMgr.playerByName(received[1]) ) {
                    Player new_player(received[1]);
                    playerMgr.Players.push_back(new_player);
                }
            } else if ( received[0] == "WHIS_SUC" ) { // WHISPER_SUC
                size = 1;
                flags[WHIS_S] = true;
                flags[WHIS_R] = true;
            } else if ( received[0] == "WHISPER_ERR") { // WHISPER_ERR
                size = 1;
                flags[WHIS_S] = false;
                flags[WHIS_R] = true;
            } else if ( received[0] == "ROLL_ERR" ) { // ROLL_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) *_rolls = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) *_rolls = NOT_A_NUM;
                else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_ROLL_ERR");
                flags[ROLL_S] = false;
                flags[ROLL_R] = true;
            } else if ( received[0] == "DECK_ERR" ) { // DECK_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "NOT FOUND" ) ErDeck = NOT_FOUND;
                else if ( received[1] == "EMPTY"     ) ErDeck = EMPTY;
                else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_DECK_ERR");
                flags[DECK_R] = true;
            } else if ( received[0] == "MOVE_ERR" ) { // MOVE_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) ErMove = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) ErMove = NOT_A_NUM;
                else if ( received[1] == "NOT FOUND"    ) ErMove = NOT_FOUND;
                else if ( received[1] == "EMPTY"        ) ErMove = EMPTY;
                else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_MOVE_ERR");
                flags[MOVE_R] = true;
            } else if ( received[0] == "ROTATE_ERR" ) { // ROTATE_ERR <error> 
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) ErRot = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) ErRot = NOT_A_NUM;
                else if ( received[1] == "NOT FOUND"    ) ErRot = NOT_FOUND;
                else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_ROTATE_ERR");
                flags[ROT_R] = true;
            } else if ( received[0] == "FLIP_ERR" ) { // FLIP_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) ErFlip = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) ErFlip = NOT_A_NUM;
                else if ( received[1] == "NOT FOUND"    ) ErFlip = NOT_FOUND;
                else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_FLIP_ERR");
                flags[FLIP_R] = true;
            } else if ( received[0] == "SHUFFLE_ERR" ) { // SHUFFLE_ERR
                size = 1;
                flags[SHUF_S] = false;
                flags[SHUF_R] = true;
            } else if ( received[0] == "SET_ERR" ) { // SET_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) ErSet = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) ErSet = NOT_A_NUM;
                else if ( received[1] == "NOT FOUND"    ) ErSet = NOT_FOUND;
                else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_SET_ERR");
                flags[SET_R] = true;
            } else if ( received[0] == "SEE_ERR" ) { // SEE_ERR
                size = 1;
                flags[SEE_S] = false;
                flags[SEE_R] = true;
            } else if ( received[0] == "CARDS_ERR" ) { // CARDS_ERR
                size = 1;
                flags[SEE_S] = false;
                flags[SEE_R] = true;
            } else if ( received[0] == "STAT_ERR" ) { // STAT_ERR
                size = 1;
                flags[STAT_S] = false;
                flags[STAT_R] = true;
            } else if ( received[0] == "CHAT" ) { // CHAT <message>
                size = 2; CHECK_SIZE;
            } else if ( received[0] == "ACT" ) { // ACT <action>
                size = 2; CHECK_SIZE;
            } else if ( received[0] == "WHISPER" ) { // WHISPER <sender> <message>
                size = 3; CHECK_SIZE;
            } else if ( received[0] == "ROLL" ) { // ROLL <player> <dice> <n> <r1> <r2> <r3> ... <rn>
                size = 4; CHECK_SIZE;
                int num = std::stoi(received[3]);
                size += num; CHECK_SIZE;
                if ( received[1] == localplayer->Name ) {
                    for ( int i = 0 ; i < num ; i++ ) {
                        _rolls[i] = std::stoi(received[4+i]);
                        flags[ROLL_S] = true;
                        flags[ROLL_R] = true;
                    }
                }
            } else if ( received[0] == "SEE" ) { // SEE <card1> <card2> ... <cardn> END
                size = 2;
                std::string buf;
                while (1) {
                    if ( received[size-1] == "END" ) break;
                    std::vector<std::string> scard = split(received[size-1].data(), ' ');
                    Card card;
                    if ( card.parse_card(scard[0]) == -1 ) goto SEE_ERROR;

                    int x, y, rot;
                    try {
                        x = std::stoi(scard[1]);
                        y = std::stoi(scard[2]);
                        rot = std::stoi(scard[3]);
                    } catch (std::invalid_argument) {goto SEE_ERROR;} catch (std::out_of_range) {goto SEE_ERROR;}
                    card.transform(x, y);
                    card.rotate(rot);
                    _see->push_back(card);

                    size++;CHECK_SIZE;
                }

                flags[SEE_S] = true;
                flags[SEE_R] = true;
                return;
            SEE_ERROR:
                throw CriticalError(CRIT__NUM_ERROR, "_worker_SEE");
            } else if ( received[0] == "CARDS" ) { // CARDS <card1> <card2> ... <cardn> END
                size = 2;CHECK_SIZE;
                std::string buf;
                while (1) {
                    if ( received[size-1] == "END" ) break;
                    std::vector<std::string> scard = split(received[size-1].data(), ' ');
                    Card card;
                    if ( card.parse_card(scard[0]) == -1 ) goto CARDS_ERROR;

                    int x, y, rot;
                    try {
                        x = std::stoi(scard[1]);
                        y = std::stoi(scard[2]);
                        rot = std::stoi(scard[3]);
                    } catch (std::invalid_argument) {goto CARDS_ERROR;} catch (std::out_of_range) {goto CARDS_ERROR;}
                    card.transform(x, y);
                    card.rotate(rot);
                    _cards->push_back(card);

                    size++;CHECK_SIZE;
                }

                flags[CRDS_S] = true;
                flags[CRDS_R] = true;
                return;
            CARDS_ERROR:
                throw CriticalError(CRIT__NUM_ERROR, "_worker_CARDS");
            } else if ( received[0] == "STAT" ) { // STAT <LEVEL> <POWER> <GOLD>
                size = 4;CHECK_SIZE;

                for ( int i = 1 ; i < size ; i++ ) _stats->push_back(received[i]);

                flags[STAT_S] = true;
                flags[STAT_R] = true;
            } else if ( received[0] == "PLAYERS" ) { // PLAYERS <player1> <player2> ... <playern> END
                size = 2;CHECK_SIZE;
                std::string buf;
                while (1) {
                    if ( received[size-1] == "END" ) break;
                    _players->push_back(received[size-1]);
                    
                    size++;CHECK_SIZE;
                }
                flags[PLRS_R] = true;
            } else if ( received[0] == "PONG" ) { // PONG <data>
                size = 2; CHECK_SIZE;
                
                std::memcpy(ping_data, received[1].data(), 8);
                flags[PONG_R] = true;
            } else if ( received[0] == "NOTIFY " ) { // NOTIFY <player> ...
                size = 3; CHECK_SIZE;
                Player* sender = playerMgr.playerByName(received[1]) ;
                constexpr int i = 2;
                if ( sender == nullptr ) throw CriticalError(CRIT__INVALID_PLAYER, "_worker_NOTIFY");

                if ( received[i] == "DECK" ) { // DECK <deck src> <dest> <x> <y> <card>
                    size += 5; CHECK_SIZE;
                    Deck* src = deckByName(received[i+ 1]);
                    CardContainer* dest = containerByName(received[i+ 2], sender);
                    if ( dest == nullptr || src == nullptr ) throw CriticalError(CRIT__BAD_CONTAINER, "_worker_DECK");
                    
                    if ( src->pop_and_move(received[i+ 3], received[i+ 4], dest, received[i+ 5]) != 0 ) throw CriticalError(CRIT__EMPTY_DECK, "_worker_DECK");
                    if ( sender == localplayer ) {
                        ErDeck = NOERROR;
                        flags[DECK_R] = true;
                    }
                } else if ( received[i] == "MOVE" ) { // MOVE <spatial src> <card_id> <dest> <x> <y> <card>
                    size += 6; CHECK_SIZE;
                    CardContainer* src     = containerByName(received[i+ 1], sender);
                    CardContainer* dest    = containerByName(received[i+ 3], sender); 
                    if ( dest == nullptr || src == nullptr ) throw CriticalError(CRIT__BAD_CONTAINER, "_worker_MOVE");

                    if ( src->move(received[i+ 2], received[i+ 4], received[i+ 5], dest, received[i+ 6]) != 0 ) throw CriticalError(CRIT__NUM_ERROR, "_worker_MOVE");
                    if ( sender == localplayer ) {
                        ErMove = NOERROR;
                        flags[MOVE_R] = true;
                    }
                } else if ( received[i] == "ROTATE" ) { // ROTATE <spatial> <card_id> <rot>
                    size += 3; CHECK_SIZE;
                    CardContainer* container = spatialByName(received[i+ 1], sender);
                    if ( container == nullptr ) throw CriticalError(CRIT__BAD_CONTAINER, "_worker_ROTATE");
                    container->rotate(received[i+ 2], received[i+ 3]);

                    if ( sender == localplayer ) {
                        ErRot = NOERROR;
                        flags[ROT_R] = true;
                    }
                } else if ( received[i] == "FLIP" ) { // FLIP <spatial> <card_id> <card>
                    size += 3; CHECK_SIZE;
                    CardContainer* container = spatialByName(received[i+ 1], sender);
                    Card* card = (*container)[received[i+ 2]];
                    if ( container == nullptr ) throw CriticalError(CRIT__BAD_CONTAINER, "_worker_FLIP");
                    else if ( card == nullptr ) throw CriticalError(CRIT__NUM_ERROR, "_worker_FLIP");
                    else if ( card->parse_card(received[i+ 3]) == -1 ) throw CriticalError(CRIT__BAD_CARD, "_worker_FLIP");

                    if ( sender == localplayer ) {
                        ErFlip = NOERROR;
                        flags[FLIP_R] = true;
                    }
                } else if ( received[i] == "SHUFFLE" ) { // SHUFFLE <deck>
                    size += 1; CHECK_SIZE;
                    if ( sender == localplayer ) {
                        flags[SHUF_S] = true;
                        flags[SHUF_R] = true;
                    }
                } else if ( received[i] == "SET" ) { // SET <stat> <value>
                    size += 2; CHECK_SIZE;
                    std::string* sStat = sStatByName(received[i+ 1], sender);
                    int* iStat = iStatByName(received[i+ 1], sender);
                    if ( sStat == nullptr ) {
                        if ( iStat == nullptr ) throw CriticalError(CRIT__BAD_STAT, "_worker_SET");
                        *iStat = std::stoi(received[i+ 2]);
                    } else *sStat = received[i+ 2];

                    if ( sender == localplayer ) {
                        ErSet = NOERROR;
                        flags[SET_R] = true;
                    }
                } else if ( received[i] == "RENAME" ) { // RENAME <new_name>
                    size += 1; CHECK_SIZE;
                    sender->Name = received[i+ 1];
                } else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_NOTIFY");
            } else throw CriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker");
        }
        for ( int _ = 0 ; _ < size ; _++ ) received.pop_front();
    }
    void _receiver () {
        if ( ! flags[JOINED] ) throw CriticalError(CRIT__NOT_JOINED, "_receiver");
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
        if ( flags[JOINED] ) throw CriticalError(CRIT__ALREADY_JOINED, "_join");
        flags[JOINED] = true;

        localplayer = playerMgr.playerByName(username);
        if ( localplayer == nullptr ) {
            Player plr(username);
            playerMgr.Players.push_back(plr);
            localplayer = playerMgr.playerByName(username);
        }

        std::thread receiver_thread(_receiver);
        receiver_thread.detach();
    }
}
#endif // PROTOCOL_H
