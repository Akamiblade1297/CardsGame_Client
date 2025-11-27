#include "main.h"
#include <string>

bool isVisible( std::string containerName ) {
    if ( containerName == "TABLE" || containerName == "EQUIPPED" ) { return true; }
    return false;
}

std::string* StatByName ( std::string name, Player* player ) {
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
