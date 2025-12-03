#include "protocol.h"
#include "functions.h"
#include "string_func.h"
#include "network.h"
#include "main.h"
#include <deque>
#include <mutex>
#include <string>
#include <cstring>
#include <stdexcept>
#include <random>
#include <thread>
#include <chrono>
#include <bitset>
#include <iostream>
#include "mainwindow.h"

#define BUFSIZE 1128

#define CHECK_SIZE if ( received.size() < size ) {  return; }

namespace {
    Connection* conn = nullptr;

    char buffer[BUFSIZE+1] = {0};
    std::deque<std::string> received;

    enum FLAG {
        JOINED = 0,
        PONG_R = 1,

        WHIS_R = 2,
        WHIS_S = 3,

        ROLL_R = 4,
        // ROLL_S = 5,

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

        RENAMING=20,

        RNM_R  = 21,
        RNM_S  = 22,
    };
    std::bitset<23> flags = {0};

}

namespace protocol {
    class ProtocolCriticalError : public std::runtime_error {
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
            ProtocolCriticalError ( CritErrorCode cod, std::string breakpoint ) : code(cod), std::runtime_error(str_code(cod)+' '+breakpoint) {
                flags[JOINED] = false;
                conn->Close();
            }
    };
}

using namespace protocol;
namespace {
    int8_t ErRoll;
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
    std::mutex muRename;
    std::mutex muSee;
    std::mutex muCards;
    std::mutex muStat;
    std::mutex muPlayers;

    char ping_data[9] = {0};
    std::vector<Card>* _see;
    std::vector<Card>* _cards;
    std::string* _stats;
    std::vector<std::string>* _players;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<uint64_t> dis;

    void _worker () {
        uint8_t size;
        while ( !received.empty() ) {
            for ( int i = 0 ; i < received.size() ; i++ ) std::cout << received[i] << ' ';
            std::cout << std::endl;
            if ( received[0] == "JOIN" ) { // JOIN <player>
                size = 2; CHECK_SIZE;

                emit mainWindow->consoleOut(QString::fromStdString(received[1]+" joined the server"));
                if ( !playerMgr.playerByName(received[1]) ) {
                    playerMgr.Players.push_back(Player(received[1]));
                }
            } else if ( received[0] == "WHISPER_SUC" ) { // WHISPER_SUC
                size = 1;
                flags[WHIS_S] = true;
                flags[WHIS_R] = true;
            } else if ( received[0] == "WHISPER_ERR") { // WHISPER_ERR
                size = 1;
                flags[WHIS_S] = false;
                flags[WHIS_R] = true;
            } else if ( received[0] == "ROLL_ERR" ) { // ROLL_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) ErRoll = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) ErRoll = NOT_A_NUM;
                else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_ROLL_ERR");
                flags[ROLL_R] = true;
            } else if ( received[0] == "DECK_ERR" ) { // DECK_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "NOT FOUND" ) ErDeck = NOT_FOUND;
                else if ( received[1] == "EMPTY"     ) ErDeck = EMPTY;
                else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_DECK_ERR");
                flags[DECK_R] = true;
            } else if ( received[0] == "MOVE_ERR" ) { // MOVE_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) ErMove = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) ErMove = NOT_A_NUM;
                else if ( received[1] == "NOT FOUND"    ) ErMove = NOT_FOUND;
                else if ( received[1] == "EMPTY"        ) ErMove = EMPTY;
                else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_MOVE_ERR");
                flags[MOVE_R] = true;
            } else if ( received[0] == "ROTATE_ERR" ) { // ROTATE_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) ErRot = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) ErRot = NOT_A_NUM;
                else if ( received[1] == "NOT FOUND"    ) ErRot = NOT_FOUND;
                else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_ROTATE_ERR");
                flags[ROT_R] = true;
            } else if ( received[0] == "FLIP_ERR" ) { // FLIP_ERR <error>
                size = 2; CHECK_SIZE;
                if      ( received[1] == "OUT OF RANGE" ) ErFlip = OUT_OF_RNG;
                else if ( received[1] == "NOT A NUMBER" ) ErFlip = NOT_A_NUM;
                else if ( received[1] == "NOT FOUND"    ) ErFlip = NOT_FOUND;
                else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_FLIP_ERR");
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
                else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_SET_ERR");
                flags[SET_R] = true;
            } else if ( received[0] == "RENAME_ERR" ) { // RENAME_ERR
                size = 1;
                flags[RNM_S] = false;
                flags[RNM_R] = true;
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
            } else if ( received[0] == "CHAT" ) { // CHAT <player> <message>
                size = 3; CHECK_SIZE;
                emit mainWindow->consoleOut(QString::fromStdString(received[1]+": \""+received[2]+"\""));
            } else if ( received[0] == "ACT" ) { // ACT <player> <action>
                size = 3; CHECK_SIZE;
                emit mainWindow->consoleOut(QString::fromStdString(received[1]+" *"+received[2]+"*"));
            } else if ( received[0] == "WHISPER" ) { // WHISPER <sender> <message>
                size = 3; CHECK_SIZE;
                emit mainWindow->consoleOut(QString::fromStdString(received[1]+" whispered: \""+received[2]+"\""));
            } else if ( received[0] == "ROLL" ) { // ROLL <player> <dice> <n> <r1> <r2> <r3> ... <rn>
                size = 4; CHECK_SIZE;
                int num = std::stoi(received[3]);
                size += num; CHECK_SIZE;
                std::string message = received[1]+" rolled "+received[3]+"d"+received[2]+": ";
                for ( int i = 4 ; i < size ; i++ ) message += received[i] + " ";
                emit mainWindow->consoleOut(QString::fromStdString(message));

                ErRoll = _NOERROR;
                flags[ROLL_R] = true;
            } else if ( received[0] == "SEE" ) { // SEE <card1> <card2> ... <cardn> END
                size = 2;
                while (1) {
                    if ( received[size-1] == "END" ) break;
                    std::vector<std::string> scard = split(received[size-1].data(), ' ');
                    Card card;
                    if ( card.parse_card(scard[0]) == -1 ) throw ProtocolCriticalError(CRIT__NUM_ERROR, "_worker_SEE");

                    int x, y, rot;
                    try {
                        x = std::stoi(scard[1]);
                        y = std::stoi(scard[2]);
                        rot = std::stoi(scard[3]);
                    } catch (std::invalid_argument) {throw ProtocolCriticalError(CRIT__NUM_ERROR, "_worker_SEE");} catch (std::out_of_range) {throw ProtocolCriticalError(CRIT__NUM_ERROR, "_worker_SEE");}
                    card.transform(x, y);
                    card.rotate(rot);
                    _see->push_back(card);

                    size++;CHECK_SIZE;
                }

                flags[SEE_S] = true;
                flags[SEE_R] = true;
            } else if ( received[0] == "CARDS" ) { // CARDS <card1> <card2> ... <cardn> END
                size = 2;CHECK_SIZE;
                while (1) {
                    if ( received[size-1] == "END" ) break;
                    std::vector<std::string> scard = split(received[size-1].data(), ' ');
                    Card card;
                    if ( card.parse_card(scard[0]) == -1 ) throw ProtocolCriticalError(CRIT__NUM_ERROR, "_worker_CARDS");

                    int x, y, rot;
                    try {
                        x = std::stoi(scard[1]);
                        y = std::stoi(scard[2]);
                        rot = std::stoi(scard[3]);
                    } catch (std::invalid_argument) {throw ProtocolCriticalError(CRIT__NUM_ERROR, "_worker_CARDS");} catch (std::out_of_range) {throw ProtocolCriticalError(CRIT__NUM_ERROR, "_worker_CARDS");}
                    card.transform(x, y);
                    card.rotate(rot);
                    _cards->push_back(card);

                    size++;CHECK_SIZE;
                }

                flags[CRDS_S] = true;
                flags[CRDS_R] = true;
            } else if ( received[0] == "STAT" ) { // STAT <LEVEL> <POWER> <GOLD>
                size = 4;CHECK_SIZE;

                for ( int i = 1 ; i < size ; i++ ) _stats[i-1] = received[i];

                flags[STAT_S] = true;
                flags[STAT_R] = true;
            } else if ( received[0] == "PLAYERS" ) { // PLAYERS <player1> <player2> ... <playern> END
                size = 2;CHECK_SIZE;
                std::string buf;
                while (1) {
                    if ( received[size-1] == "END" ) break;
                    _players->push_back(received[size-1]);
                    if ( playerMgr.playerByName(received[size-1]) == nullptr )
                        playerMgr.Players.push_back(Player(received[size-1]));

                    size++;CHECK_SIZE;
                }
                flags[PLRS_R] = true;
            } else if ( received[0] == "PONG" ) { // PONG <data>
                size = 2; CHECK_SIZE;

                std::memcpy(ping_data, received[1].data(), 8);
                flags[PONG_R] = true;
            } else if ( received[0] == "NOTIFY" ) { // NOTIFY <player> ...
                size = 3; CHECK_SIZE;
                Player* sender = playerMgr.playerByName(received[1]) ;
                constexpr int i = 2;
                if ( sender == nullptr ) {
                    playerMgr.Players.push_back(Player(received[1]));
                    sender = &playerMgr.Players.back();
                }

                if ( received[i] == "DECK" ) { // DECK <deck src> <dest> <x> <y> <card>
                    size += 5; CHECK_SIZE;
                    Deck* src = deckByName(received[i+ 1]);
                    CardContainer* dest = containerByName(received[i+ 2], sender);
                    if ( dest == nullptr || src == nullptr ) throw ProtocolCriticalError(CRIT__BAD_CONTAINER, "_worker_DECK");
                    if ( src->pop_and_move(received[i+ 3], received[i+ 4], dest, received[i+ 5]) != 0 ) throw ProtocolCriticalError(CRIT__EMPTY_DECK, "_worker_DECK");

                    emit mainWindow->consoleOut(QString::fromStdString(received[1]+" drawed Card_"+received[i+ 5]+" from "+received[i+ 1]+" to "+received[i+ 2]+"["+std::to_string(dest->Cards.size()-1)+"] (X: "+received[i+ 3]+", Y: "+received[i+ 4]+")"));
                    if ( sender == &LOCALPLAYER ) {
                        ErDeck = _NOERROR;
                        flags[DECK_R] = true;
                    }
                } else if ( received[i] == "MOVE" ) { // MOVE <spatial src> <card_id> <dest> <x> <y> <card>
                    size += 6; CHECK_SIZE;
                    CardContainer* src     = containerByName(received[i+ 1], sender);
                    CardContainer* dest    = containerByName(received[i+ 3], sender);
                    if ( dest == nullptr || src == nullptr ) throw ProtocolCriticalError(CRIT__BAD_CONTAINER, "_worker_MOVE");
                    if ( src->move(received[i+ 2], received[i+ 4], received[i+ 5], dest, received[i+ 6]) != 0 ) throw ProtocolCriticalError(CRIT__NUM_ERROR, "_worker_MOVE");

                    emit mainWindow->consoleOut(QString::fromStdString(received[1]+" moved Card_"+received[i+ 6]+" from "+received[i+ 1]+"["+received[i+ 2]+"] to "+received[i+ 3]+"["+std::to_string(dest->Cards.size()-1)+"] (X: "+received[i+ 4]+", Y: "+received[i+ 5]+")"));
                    if ( sender == &LOCALPLAYER ) {
                        ErMove = _NOERROR;
                        flags[MOVE_R] = true;
                    }
                } else if ( received[i] == "ROTATE" ) { // ROTATE <spatial> <card_id> <rot>
                    size += 3; CHECK_SIZE;
                    CardContainer* container = spatialByName(received[i+ 1], sender);
                    if ( container == nullptr ) throw ProtocolCriticalError(CRIT__BAD_CONTAINER, "_worker_ROTATE");
                    container->rotate(received[i+ 2], received[i+ 3]);

                    emit mainWindow->consoleOut(QString::fromStdString(received[1]+" rotated Card_"+container->at(received[i+ 2])->unparse_card()+" on "+received[i+ 1]+"["+received[i+ 2]+"] (Rot: "+received[i+ 3]+")"));
                    if ( sender == &LOCALPLAYER ) {
                        ErRot = _NOERROR;
                        flags[ROT_R] = true;
                    }
                } else if ( received[i] == "FLIP" ) { // FLIP <spatial> <card_id> <card>
                    size += 3; CHECK_SIZE;
                    CardContainer* container = spatialByName(received[i+ 1], sender);
                    Card* card = container->at(received[i+ 2]);
                    std::string old_num = card->unparse_card();
                    if ( container == nullptr ) throw ProtocolCriticalError(CRIT__BAD_CONTAINER, "_worker_FLIP");
                    else if ( card == nullptr ) throw ProtocolCriticalError(CRIT__NUM_ERROR, "_worker_FLIP");
                    else if ( card->parse_card(received[i+ 3]) == -1 ) throw ProtocolCriticalError(CRIT__BAD_CARD, "_worker_FLIP");

                    emit mainWindow->consoleOut(QString::fromStdString(received[1]+" flipped Card on "+received[i+ 1]+"["+received[i+ 2]+"] ("+old_num+" -> "+card->unparse_card()+")"));
                    if ( sender == &LOCALPLAYER ) {
                        ErFlip = _NOERROR;
                        flags[FLIP_R] = true;
                    }
                } else if ( received[i] == "SHUFFLE" ) { // SHUFFLE <deck>
                    size += 1; CHECK_SIZE;

                    emit mainWindow->consoleOut(QString::fromStdString(received[1]+" shuffled "+received[i+ 1]));
                    if ( sender == &LOCALPLAYER ) {
                        flags[SHUF_S] = true;
                        flags[SHUF_R] = true;
                    }
                } else if ( received[i] == "SET" ) { // SET <stat> <value>
                    size += 2; CHECK_SIZE;
                    std::string* Stat = StatByName(received[i+ 1], sender);
                    if ( Stat == nullptr ) {
                        throw ProtocolCriticalError(CRIT__BAD_STAT, "_worker_SET");
                    } else *Stat = received[i+ 2];

                    emit mainWindow->consoleOut(QString::fromStdString(received[1]+" set "+received[i+ 1]+" to "+received[i+ 2]));
                    if ( sender == &LOCALPLAYER ) {
                        ErSet = _NOERROR;
                        flags[SET_R] = true;
                    }
                } else if ( received[i] == "RENAME" ) { // RENAME <new_name>
                    size += 1; CHECK_SIZE;
                    sender->Name = received[i+ 1];
                    emit mainWindow->consoleOut(QString::fromStdString(received[1]+" renamed to "+received[i+ 1]));
                    if ( sender == &LOCALPLAYER ) {
                        flags[RNM_S] = true;
                        flags[RNM_R] = true;
                    }
                } else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker_NOTIFY");
            } else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "_worker");
            for ( int _ = 0 ; _ < size ; _++ ) received.pop_front();
        }
    }
    void _receiver () {
        if ( ! flags[JOINED] ) throw ProtocolCriticalError(CRIT__NOT_JOINED, "_receiver");
        while (1) {
            int len = conn->Receive(buffer, BUFSIZE);
            if ( len <= 0 ) {
                flags[JOINED] = false;
                return;
            }
            buffer[len] = 0;
            std::vector<std::string> recv = split(buffer, DEL);
            for ( int i = 0 ; i < recv.size() ; i++ ) {
                if (recv[i] != "" )
                    received.push_back(recv[i]);
            }
            _worker();
        }
    }
    void _join ( std::string username ) {
        if ( flags[JOINED] ) throw ProtocolCriticalError(CRIT__ALREADY_JOINED, "_join");
        flags[JOINED] = true;

        playerMgr.Players.erase(playerMgr.Players.begin(), playerMgr.Players.end());
        playerMgr.Players.push_back(Player(username));

        std::thread receiver_thread(_receiver);
        receiver_thread.detach();
    }
    std::string parse_pass(char* buffer, int& i) {
        char pass[8];
        std::memcpy(pass, buffer+3, 8);

        for ( int k = 0 ; k < 8 ; k++ ) {
            if ( pass[k] == DEL ) i++;
        } i++;

        std::string cpass = "";
        cpass.append(pass, 8);
        return cpass;
    }
}

namespace protocol {
    ErrorCode connect ( const char* ip, unsigned short port ) {
        if ( conn != nullptr )
            delete conn;
        bool suc;
        conn = new Connection( ip, port, &suc );
        return suc ? _NOERROR : SEND_ERROR;
    }
    ErrorCode join ( const char* username, char* result ) {
        if ( flags[JOINED] ) return PROTOCOL_ERR;
        if ( flags[RENAMING] ) {
            if ( conn->Send(username) == -1 ) return SEND_ERROR;
        } else {
            std::string temp = "JOIN";
            if ( conn->Send(temp + DEL + username) == -1 ) return SEND_ERROR;
        }
        int len = conn->Receive(buffer, BUFSIZE);
        if ( len < 6 ) throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "join"); // min response: "RENAME"
        buffer[len] = 0;
        std::cout << buffer << std::endl;
        std::vector<std::string> response = split(buffer, DEL);

        if (  len >= 11 && response[0] == "OK" ) { // OK <pass>
            flags[RENAMING] = false;

            int i = 1;
            std::string pass = parse_pass(buffer, i);
            if ( pass.size() != 8 ) throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "join_OK");
            std::memcpy(result, pass.data(), 8);
            for (; i < response.size() ; i++ ) received.push_back(response[i]);
            _join(username);

            return _NOERROR;
        } else if ( response.size() == 1 && response[0] == "RENAME"  ) { // RENAME
            flags[RENAMING] = true;
            return RENAME;
        } else if ( response.size() == 1 && response[0] == "TIMEOUT" ) { // TIMEOUT
            flags[RENAMING] = false;
            return TIMEOUT;
        } else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "join");
    }
    ErrorCode rejoin ( const char* pass ) {
        std::string temp = "REJOIN";
        temp+=DEL;
        temp.append(pass, 8);
        if ( conn->Send(temp) == -1 ) return SEND_ERROR;
        int len = conn->Receive(buffer, BUFSIZE);
        if ( len < 2 ) throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "rejoin");
        buffer[len] = 0;
        std::vector<std::string> response = split(buffer, DEL);

        if ( response[0] == "OK" ) {
            for ( int i = 2 ; i < response.size() ; i++ ) received.push_back(response[i]);
            _join(response[1]);
            return _NOERROR;
        }
        else if ( response[0] == "NOT FOUND" ) return NOT_FOUND;
        else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "rejoin");
    }
    ErrorCode ping ( int& time, int timeout ) {
        uint64_t dat = dis(rng);
        char data[9] = {0};
        std::memcpy(data, &dat, 8);

        std::lock_guard<std::mutex> lock(muPing);
        time = timeout;
        std::string temp = "PING";
        if ( conn->Send(temp + DEL + data) == -1 ) return SEND_ERROR;

        flags[PONG_R] = false;
        while ( !flags[PONG_R] ) {
            if ( time > 0 ) {
                time-=1;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } else {
                time = timeout - time;
                return TIMEOUT;
            }
        }

        if ( std::strcmp(data, ping_data) == 0 ) {
            time = timeout - time;
            return _NOERROR;
        }
        else throw ProtocolCriticalError(CRIT__UNEXPECTED_RESPONSE, "ping");
    }
    ErrorCode chat ( std::string msg ) {
        std::string temp = "CHAT";
        if ( conn->Send(temp + DEL + msg) == -1 ) return SEND_ERROR;
        return _NOERROR;
    }
    ErrorCode action ( std::string act ) {
        std::string temp = "ACT";
        if ( conn->Send(temp + DEL + act) == -1 ) return SEND_ERROR;
        return _NOERROR;
    }
    ErrorCode whisper ( std::string player, std::string msg ) {
        std::lock_guard<std::mutex> lock(muWhisper);       
        std::string temp = "WHISPER";
        int res = conn->Send(temp + DEL + player + DEL + msg);

        if ( res == -1 ) return SEND_ERROR;
        flags[WHIS_R] = false;
        while ( !flags[WHIS_R] ) {}

        if ( !flags[WHIS_S] ) return NOT_FOUND;
        return _NOERROR;
    }
    ErrorCode roll ( std::string dice, std::string num) {
        std::lock_guard<std::mutex> lock(muRoll);
        std::string temp = "ROLL";
        if ( conn->Send(temp + DEL + dice + DEL + num) == -1 ) return SEND_ERROR;
        
        flags[ROLL_R] = false;
        while ( !flags[ROLL_R] ) {}

        return (ErrorCode)ErRoll;
    }
    ErrorCode deck ( std::string src, std::string dest, std::string x, std::string y ) {
        std::lock_guard<std::mutex> lock(muDeck);
        std::string temp = "DECK";
        if ( conn->Send(temp + DEL + src + DEL + dest + DEL + x + DEL + y) == -1 ) return SEND_ERROR;
        
        flags[DECK_R] = false;
        while ( !flags[DECK_R] ) {}
        return (ErrorCode)ErDeck;
    }
    ErrorCode move ( std::string src, std::string card_id, std::string dest, std::string x, std::string y ) {
        std::lock_guard<std::mutex> lock(muMove);
        std::string temp = "MOVE";
        if ( conn->Send(temp + DEL + src + DEL + card_id + DEL + dest + DEL + x + DEL + y) == -1 ) return SEND_ERROR;

        flags[MOVE_R] = false;
        while ( !flags[MOVE_R] ) {}
        return (ErrorCode)ErMove;
    }
    ErrorCode rotate ( std::string spatial, std::string card_id, std::string rot) {
        std::lock_guard<std::mutex> lock(muRot);
        std::string temp = "ROTATE";
        if ( conn->Send(temp + DEL + spatial + DEL + card_id + DEL + rot ) == -1 ) return SEND_ERROR;

        flags[ROT_R] = false;
        while ( !flags[ROT_R] ) {}
        return (ErrorCode)ErRot;
    }
    ErrorCode flip ( std::string spatial, std::string card_id ) {
        std::lock_guard<std::mutex> lock(muFlip);
        std::string temp = "FLIP";
        if ( conn->Send(temp + DEL + spatial + DEL + card_id ) == -1 ) return SEND_ERROR;

        flags[FLIP_R] = false;
        while ( !flags[FLIP_R] ) {}
        return (ErrorCode)ErFlip;
    }
    ErrorCode shuffle ( std::string deck ) {
        std::lock_guard<std::mutex> lock(muShuffle);
        std::string temp = "SHUFFLE";
        if ( conn->Send(temp + DEL + deck ) == -1 ) return SEND_ERROR;

        flags[SHUF_R] = false;
        while ( !flags[SHUF_R] ) {}
        return flags[SHUF_S] ? _NOERROR : NOT_FOUND;
    }
    ErrorCode set ( std::string stat, std::string value ) {
        std::lock_guard<std::mutex> lock(muSet);
        std::string temp = "SET";
        if ( conn->Send(temp+DEL+stat+DEL+value ) == -1) return SEND_ERROR;

        flags[SET_R] = false;
        while ( !flags[SET_R] ) {}
        return (ErrorCode)ErSet;
    }
    ErrorCode rename ( std::string name ) {
        std::lock_guard<std::mutex> lock(muRename);
        std::string temp = "RENAME";
        if ( conn->Send(temp+DEL+name ) == -1 ) return SEND_ERROR;

        flags[RNM_R] = false;
        while ( !flags[RNM_R] ) {}
        return flags[RNM_S] ? _NOERROR : RENAME;
    }
    ErrorCode see ( std::string container, std::vector<Card>* res ) {
        std::lock_guard<std::mutex> lock(muSee);
        _see = res;

        std::string temp = "SEE";
        if ( conn->Send(temp+DEL+container ) == -1 ) return SEND_ERROR;

        flags[SEE_R] = false;
        while ( ! flags[SEE_R] ) {}
        return flags[SEE_S] ? _NOERROR : NOT_FOUND;
    }
    ErrorCode cards ( std::string container, std::vector<Card>* res ) {
        std::lock_guard<std::mutex> lock(muCards);
        _cards = res;

        std::string temp = "CARDS";
        if ( conn->Send(temp+DEL+container ) == -1 ) return SEND_ERROR;

        flags[CRDS_R] = false;
        while ( ! flags[CRDS_R] ) {}
        return flags[CRDS_S] ? _NOERROR : NOT_FOUND;
    }
    ErrorCode stat ( std::string player, std::string* res ) {
        std::lock_guard<std::mutex> lock(muStat);
        _stats = res;

        std::string temp = "STAT";
        if ( conn->Send(temp+DEL+player ) == -1 ) return SEND_ERROR;

        flags[STAT_R] = false;
        while ( ! flags[STAT_R] ) {}
        return flags[STAT_S] ? _NOERROR : NOT_FOUND;
    }
    ErrorCode players ( std::vector<std::string>* res ) {
        std::lock_guard<std::mutex> lock(muPlayers);
        _players = res;

        if ( conn->Send("PLAYERS") == -1 ) return SEND_ERROR;

        flags[PLRS_R] = false;
        while ( ! flags[PLRS_R] ) {}
        return _NOERROR;
    }
}
