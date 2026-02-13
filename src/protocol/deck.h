#ifndef DECK_H
#define DECK_H

#include "cardcontainer.h"
#include <QLabel>

#define DECK_MAX_VIEW 5
#define DECK_CARD_OFFSET 1

/**
 * @brief The Deck class
 */
class Deck : public CardContainer {
protected:
    QLabel* Frame;
public:
  Deck ( QFrame* table, QLabel* frame );

  void push ( Card* card ) override;
  int move ( std::string si, std::string sx, std::string sy, CardContainer* container, std::string num ) override;
  int rotate ( std::string si, std::string srot ) override;

  /**
   * @brief Pop Card and move to other container
   * @param sx New X coordinate
   * @param sy New Y coordinate
   * @param container Destinition container
   * @param num New card Number
   * @return 0 if Success, -1 if Failed
   */
  int pop_and_move ( std::string sx, std::string sy, CardContainer* container, std::string num );

  QLabel* frame() const;
};

extern Deck Treasures;
extern Deck Trapdoors;

#endif // DECK_H
