#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "players.h"
#include "deck.h"
#include <string>

/**
 * Get Player's Stat by Name
 *
 * @param name Stat name
 * @param player Player
 * @return Pointer to a Stat if Success, nullptr if Failed (Not Found)
 */
std::string* StatByName ( std::string name, Player* player );
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
 * Get name of container
 *
 * @param container
 * @return name
 */
std::string nameByContainer ( CardContainer* container );
/**
 * Get a Visibility of Card Container
 *
 * @param containerName Container Name
 * @return True if Container if visible, False otherwise
 */
bool isVisible ( std::string containerName );

#endif
