#ifndef PLAYERS_H
#define PLAYERS_H

#include <string>
#include <vector>
#include <random>
#include <map>

static std::random_device rd;
static std::mt19937 rng(rd());
const std::vector<std::string> playerColors = {
    "#FF6B6B", // Warm coral red
    "#4ECDC4", // Bright turquoise
    "#06D6A0", // Emerald green
    "#118AB2", // Strong ocean blue
    "#9D4EDD", // Deep purple (high contrast)
    "#FF9E6D", // Bright peach/orange
    "#2A9D8F", // Teal (easy on the eyes)
    "#E76F51", // Terracotta orange
    "#6A994E", // Muted forest green
    "#4361EE", // Royal blue
    "#7209B7", // Deep violet
    "#F15BB5", // Magenta pink
    "#0077B6", // Classic medium blue (web standard)
    "#CA6702", // Burnt orange
    "#0A9396", // Dark cyan
    "#9B5DE5", // Soft purple
    "#00BBF9", // Sky blue
    "#F94144", // Alert red (use sparingly)
    "#57CC99", // Fresh mint green
};
static std::uniform_int_distribution<> playerColorsDist(0, playerColors.size()-1);

class CardContainer;
/**
 * @brief The Player class
 */
class Player {
public:
  std::string Level, Power, Gold;
  std::string Color;
  CardContainer* Inventory;
  CardContainer* Equiped;

  /**
   * @brief Player
   * @param name Username
   * @param lvl Level
   * @param pwr Power
   * @param gld Gold
   */
  Player ( std::string lvl = "0", std::string pwr = "0", std::string gld = "0" );
};

extern std::map<std::string, Player*> Players;

#endif // PLAYERS_H
