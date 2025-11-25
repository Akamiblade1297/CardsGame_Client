#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <vector>
#include <string>
#include "network.h"
#include "main.h"

namespace protocol {
    class CriticalError;

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
    enum ErrorCode {
        INVALID_SOCK =-11,
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
        RENAME       =  1,
    };

    ErrorCode connect ( const char* u );
    /**
     * @brief Join
     * @param username Player Username
     * @param result Pointer to a result response (must be size of 8 bytes);
     * @return NOERROR, RENAME, SEND_ERROR, PROTOCOL_ERR
     */
    ErrorCode join ( const char* username, char* result );
    /**
     * @brief Rejoin
     * @param pass Player pass
     * @return NOERROR, NOT_FOUND, SEND_ERROR, PROTOCOL_ERR
     */
    ErrorCode rejoin ( const char* pass );
    /**
     * @brief Ping
     * @param timeout Connection timeout in secconds
     * @return NOERROR, TIMEOUT, SEND_ERROR
     */
    ErrorCode ping ( int timeout = 10 );
    /**
     * @brief Send message to Chat
     * @param msg Message to send
     * @return NOERROR, SEND_ERROR
     */
    ErrorCode chat ( std::string msg );
    /**
     * @brief Send action to Chat
     * @param act Action to send
     * @return NOERROR, SEND_ERROR
     */
    ErrorCode action ( std::string act );
    /**
     * @brief Whisper message to playert
     * @param player Receiver
     * @param msg Message to whisper
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode whisper ( std::string player, std::string msg );
    /**
     * @brief Roll a dice
     * @param rolls Pointer to result array of rolled numbers (must be size of num bytes)
     * @param dice Number of sides of each dice
     * @param num Number of dices to roll (1-10)
     * @return NOERROR, OUT_OF_RNG, NOT_A_NUM, SEND_ERROR, BUFFER_ERROR
     */
    ErrorCode roll ( uint8_t* rolls, std::string dice = "6", std::string num = "1" );
    /**
     * @brief Pop card from Deck
     * @param src Source deck
     * @param dest Destinition container
     * @param x New X coordinate
     * @param y New Y coordinate
     * @return NOERROR, EMPTY, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode deck ( std::string src, std::string dest, std::string x, std::string y );
    /**
     * @brief Move card from Spatial
     * @param src Source spatial
     * @param card_id Card id inside Source container
     * @param dest Destinition container
     * @param x New X coordinate
     * @param y New Y coordinate
     * @return NOERROR, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode move ( std::string src, std::string card_id, std::string dest, std::string x, std::string y );
    /**
     * @brief Rotate card in Spatial
     * @param spatial Container
     * @param card_id Card id inside Container
     * @param rot New rotation angle
     * @return NOERROR, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode rotate ( std::string spatial, std::string card_id, std::string rot);
    /**
     * @brief Flip card in Spatial
     * @param spatial Container
     * @param card_id Card id insite Container
     * @return NOERROR, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode flip ( std::string spatial, std::string card_id );
    /**
     * @brief Shuffle deck
     * @param deck Deck to shuffle
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode shuffle ( std::string deck );
    /**
     * @brief Set localplayer stat
     * @param stat Stat to set
     * @param value Value to set
     * @return NOERROR, NOT_FOUND, NOT_A_NUM, OUT_OF_RNG, SEND_ERROR
     */
    ErrorCode set ( std::string stat, std::string value );
    /**
     * @brief Rename localplayer
     * @param name New name
     * @return NOERROR, SEND_ERROR
     */
    ErrorCode rename ( std::string name );
    /**
     * @brief Get all cards in visible containers
     * @param container Container
     * @param res Cards array to store gotten cards
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode see ( std::string container, std::vector<Card>* res );
    /**
     * @brief Get all cards in non-visible containers
     * @param container Container
     * @param res Cards array to store gotten cards
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode cards ( std::string container, std::vector<Card>* res );
    /**
     * @brief Get all stats of a player
     * @param player Player
     * @param res Array of stats values
     * @return NOERROR, NOT_FOUND, SEND_ERROR
     */
    ErrorCode stat ( std::string player, std::vector<std::string>* res );
    /**
     * @brief Get all players
     * @param res Array of player names
     * @return NOERROR, SEND_ERROR
     */
    ErrorCode players ( std::vector<std::string>* res );
};

#endif // PROTOCOL_H
