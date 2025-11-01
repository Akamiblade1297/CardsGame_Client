#ifndef MAIN_H
#define MAIN_H
#include <vector>
#include <string>
#include <stdexcept>

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
  Card ( int num, int x, int y, int rot )
      : Number(num), X(x), Y(y), Rotation(rot) {}

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
};

/**
 * @brief The CardContainer class
 */
class CardContainer {
protected:
  std::vector<Card> Cards;
public:
  CardContainer () {}

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
   * @return 0 if Success, -1 if Failed
   */
  int move ( std::string si, std::string sx, std::string sy, CardContainer* container ) {
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
   * @return 0 if Success, -1 if Failed
   */
  int pop_and_move ( std::string sx, std::string sy, CardContainer* container ) {
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
  int Level, Power, Gold;
  CardContainer Inventory, Equiped;

  /**
   * @brief Player
   * @param name Username
   * @param lvl Level
   * @param pwr Power
   * @param gld Gold
   */
  Player ( std::string name, int lvl, int pwr, int gld )
      : Name(name), Level(lvl), Power(pwr), Gold(gld) {}
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
    return nullptr;
  }
};

#endif
