#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <string>
#include "main.h"

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

#endif
