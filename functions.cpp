#ifndef FUNC_CPP
#define FUNC_CPP
#include "main.h"
#include <string>

// FUNCTION PROTOTYPES //

/**
 * Get Player's string Stat by Name
 *
 * @param name Stat name
 * @param player Player
 * @return Pointer to a Stat if Success, nullptr if Failed (Not Found)
 */
std::string* sStatByName ( std::string name, Player* player );
/**
 * Get Player's int Stat by Name
 *
 * @param name Stat name
 * @param player Player
 * @return Pointer to a Stat if Success, nullptr if Failed (Not Found)
 */
int* iStatByName ( std::string name, Player* player );
/**
 * Get CardContainer by Name
 *
 * @param name Container Name
 * @param player Player, that might own the Container
 * @return Pointer to a Container if Success, nullptr if Failed (Not Found)
 */
CardContainer* containerByName ( std::string name, Player* player );
/**
 * Get Spatial CardContainer by Name
 *
 * @param name Container Name
 * @param player Player, that might own the Container
 * @return Pointer to a Container if Success, nullptr if Failed (Not Found)
 */
CardContainer* spatialByName   ( std::string name, Player* player );
/**
 * Get Deck CardContainer by Name
 *
 * @param name Container Name
 * @param player Player, that might own the Container
 * @return Pointer to a Container if Success, nullptr if Failed (Not Found)
 */
Deck* deckByName ( std::string name );
/**
 * Get a Visibility of Card Container
 *
 * @param containerName Container Name
 * @return True if Container if visible, False otherwise
 */
bool isVisible ( std::string containerName );


Table table;
PlayerManager playerMgr;

bool isVisible( std::string containerName ) {
    if ( containerName == "TABLE" || containerName == "EQUIPPED" ) { return true; }
    return false;
}

std::string* sStatByName ( std::string name, Player* player ) {
    return nullptr;
}

int* iStatByName ( std::string name, Player* player ) {
    if ( name == "LEVEL" ) {
        return &player->Level;
    } else if ( name == "POWER" ) {
        return &player->Power;
    } else if ( name == "GOLD" ) {
        return &player->Gold;
    } else {
        return nullptr;
    }
}

CardContainer* containerByName ( std::string name, Player* player ) {
    if ( name == "TABLE" ) {
        return &table;
    } else if ( name == "TREASURES" ) {
        return &table.Treasures;
    } else if ( name == "TRAPDOORS" ) {
        return &table.TrapDoors;
    } else if ( name == "INVENTORY" ) {
        return &player->Inventory;
    } else if ( name == "EQUIPPED" ) {
        return &player->Equiped;
    } else {
        return nullptr;
    }
}

Deck* deckByName ( std::string name ) {
    if ( name == "TREASURES" ) {
        return &table.Treasures;
    } else if ( name == "TRAPDOORS" ) {
        return &table.TrapDoors;
    } else {
        return nullptr;
    }
}

CardContainer* spatialByName ( std::string name, Player* player ) {
    if ( name == "TABLE" ) {
        return &table;
    } else if ( name == "INVENTORY" ) {
        return &player->Inventory;
    } else if ( name == "EQUIPPED" ) {
        return &player->Equiped;
    } else {
        return nullptr;
    }
}

#endif
