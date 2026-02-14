#ifndef DECK_H
#define DECK_H

#include "cardcontainer.h"
#include <QLabel>

#define DECK_MAX_VIEW 5
#define DECK_CARD_OFFSET 5

constexpr size_t UPDATED_CARDS_LENGTH = 2*DECK_MAX_VIEW + 2;

/**
 * @brief The Deck class
 */
class Deck : public CardContainer {
protected:
    QLabel* Frame;
public:
  Deck ( QFrame* table, QLabel* frame );

  void push ( Card* card, Card** updated_cards ) override;
  int move ( std::string si, std::string sx, std::string sy, CardContainer* container, std::string num, Card** updated_cards ) override;
  int rotate ( std::string si, std::string srot ) override;

  /**
   * @brief Pop Card and move to other container
   * @param sx New X coordinate
   * @param sy New Y coordinate
   * @param container Destinition container
   * @param num New card Number
   * @param updated_cards Pointer to array with size of DECK_MAX_VIEW+1, where to store edited cards 
   * @return 0 if Success, -1 if Empty, -2 if Failed to parse card num
   */
  int pop_and_move ( std::string sx, std::string sy, CardContainer* container, std::string num, Card** updated_cards = nullptr );

  QLabel* frame() const;
};

extern Deck Treasures;
extern Deck Trapdoors;

#endif // DECK_H
