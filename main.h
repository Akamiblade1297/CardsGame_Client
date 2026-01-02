#ifndef MAIN_H
#define MAIN_H
#include <vector>
#include <string>
#include <stdexcept>
#include <random>

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

enum CardType {
  TRAP = -1,
  TRES = -2,
};

/**
 * @brief The Card class
 */
struct Card {
  int Number;
  int X, Y, Rotation;
  /**
   * @brief Card
   * @param num Card Number
   * @param x X coordinate
   * @param y Y coordinate
   * @param rot Rotation
   */
  Card ( int num, int x = 0, int y = 0, int rot = 0 )
      : Number(num), X(x), Y(y), Rotation(rot) {}
  Card () : Card(0) {}

  /**
   * @brief Change card position
   * @param x New X coordinate
   * @param y New Y coordinate
   */
  void transform ( int x, int y ) {
    X = x;
    Y = y;
  }

  /**
   * @brief Rotate card
   * @param rot New Rotation
   * @return 0 if Success, -1 if Failed
   */
  int rotate ( int rot ) {
    if ( rot < 0 || rot > 360 ) return -1;
    Rotation = rot;
    return 0;
  }
  /**
   * @brief Parse and set card number from string
   * @param card String to parse
   * @return 0 if Success, -1 if Failed
   */
  int parse_card ( std::string card ) {
    try {
        Number = std::stoi(card);
        return 0;
    } catch ( std::invalid_argument ) {;} catch ( std::out_of_range ) { return -1; }
    if ( card == "TRAP" ) Number = TRAP;
    else if ( card == "TRES" ) Number = TRES;
    else return -1;

    return 0;
  } 

  /**
   * @brief Unparse card number to string
   * @return String reference of card number
   */
  std::string unparse_card () {
      if ( Number == TRAP ) return "TRAP";
      else if ( Number == TRES ) return "TRES";
      else return std::to_string(Number);
  }
};

/**
 * @brief The CardContainer class
 */
class CardContainer {
public:
  std::vector<Card> Cards;
  CardContainer () {}

  Card* operator[] ( std::string si ) {
    int i;
    try {
        i = std::stoi(si);
    } catch ( std::out_of_range ) {
      return nullptr;
    } catch ( std::invalid_argument ) {
      return nullptr;
    }
    if ( i < 0 || i > Cards.size() ) return nullptr;
    return &Cards[i];
  }

  Card* at ( std::string si ) {
      return operator[](si);
  }
  /**
   * @brief Push card to container
   * @param card Card
   */
  void push ( Card card ) {
    Cards.push_back(card);
  }

  /**
   * @brief Move card to other container
   * @param si Card index
   * @param sx New X coordinate
   * @param sy New Y coordinate
   * @param container Destinition container
   * @param num New card Number
   * @return 0 if Success, -1 if Failed
   */
  int move ( std::string si, std::string sx, std::string sy, CardContainer* container, std::string num ) {
    int i, x, y;
    try {
        i = std::stoi(si);
        x = std::stoi(sx);
        y = std::stoi(sy);
    } catch ( std::out_of_range ) {
      return -1;
    } catch ( std::invalid_argument ) {
      return -1;
    }
    if ( i < 0 || i > Cards.size() ) return -1;
    if ( Cards[i].parse_card(num) != 0 ) return -1;
    Cards[i].transform(x, y);
    container->push(Cards[i]);
    Cards.erase(Cards.begin()+i);
    return 0;
  }

  /**
   * @brief Rotate card
   * @param si Card index
   * @param srot New Rotation
   * @return 0 if Success, -1 if Failure
   */
  int rotate ( std::string si, std::string srot ) {
    int i, rot;
    try {
        i = std::stoi(si);
        rot = std::stoi(srot);
    } catch ( std::out_of_range ) {
      return -1;
    } catch ( std::invalid_argument ) {
      return -1;
    }
    if ( i < 0 || i > Cards.size() ) return -1;
    return Cards[i].rotate(rot);
  }
};

/**
 * @brief The Deck class
 */
class Deck : public CardContainer {
public:
  Deck () : CardContainer () {}
  int move ( std::string si, std::string sx, std::string sy, CardContainer* container ) { return -1; }
  int rotate ( std::string si, std::string srot ) { return -1; }

  /**
   * @brief Pop Card and move to other container
   * @param sx New X coordinate
   * @param sy New Y coordinate
   * @param container Destinition container
   * @param num New card Number
   * @return 0 if Success, -1 if Failed
   */
  int pop_and_move ( std::string sx, std::string sy, CardContainer* container, std::string num ) {
    if ( Cards.empty() ) return -1;
    int x, y;
    try {
        x = std::stoi(sx);
        y = std::stoi(sy);
    } catch ( std::out_of_range ) {
      return -1;
    } catch ( std::invalid_argument ) {
      return -1;
    }
    Card card = Cards.back();
    if ( card.parse_card(num) != 0 ) return -1;
    card.transform(x, y);
    Cards.pop_back();

    container->push(card);
    return 0;
  }
};

/**
 * @brief The Table class
 */
class Table : public CardContainer {
public:
  Deck TrapDoors;
  Deck Treasures;

  Table () : CardContainer(), TrapDoors(), Treasures() {}
};

/**
 * @brief The Player class
 */
class Player {
public:
  std::string Name;
  std::string Level, Power, Gold;
  std::string Color;
  CardContainer Inventory, Equiped;

  /**
   * @brief Player
   * @param name Username
   * @param lvl Level
   * @param pwr Power
   * @param gld Gold
   */
  Player ( std::string name, std::string lvl = "0", std::string pwr = "0", std::string gld = "0" )
      : Name(name), Level(lvl), Power(pwr), Gold(gld), Color(playerColors[playerColorsDist(rng)]) {}
};

class PlayerManager {
public:
  std::vector<Player> Players;
  /**
   * @brief Get player by Username
   * @param name Username
   * @return Pointer to a player if Success, nullptr if Failed
   */
  Player* playerByName ( std::string name ) {
    for ( Player &player : Players ) {
      if ( player.Name == name ) return &player;
    }
    Players.push_back(Player(name));
    return &Players.back();
  }
};

extern Table table;
extern PlayerManager playerMgr;

#endif
