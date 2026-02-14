#ifndef CARDCONTAINER_H
#define CARDCONTAINER_H

#include "card.h"
#include <vector>
#include <QFrame>

/**
 * @brief The CardContainer class
 */
class CardContainer {
protected:
  QFrame* Space;
  std::vector<Card*> Cards;
public:
  CardContainer ( QFrame* space );

  Card* operator[] ( int i );
  Card* operator[] ( std::string si );
  Card* at ( std::string si );
  Card* at ( int si );
  /**
   * @brief Push card to container
   * @param card Card
   * @param updated_cards Pointer to array, where to store edited cards 
   */
  virtual void push ( Card* card, Card** updated_cards );

  /**
   * @brief Destroy all cards inside container
   */
  void clear();

  /**
   * @brief Move card to other container
   * @param si Card index
   * @param sx New X coordinate
   * @param sy New Y coordinate
   * @param container Destinition container
   * @param num New card Number
   * @param updated_cards Pointer to array, where to store edited cards 
   * @return 0 if Success, -1 if Failed
   */
  virtual int move ( std::string si, std::string sx, std::string sy, CardContainer* container, std::string num, Card** updated_card );

  /**
   * @brief Rotate card
   * @param si Card index
   * @param srot New Rotation
   * @return 0 if Success, -1 if Failure
   */
  virtual int rotate ( std::string si, std::string srot );

  QFrame* space() const;
  int size() const;
  bool empty() const;
  size_t find ( Card* card ) const;
};

extern CardContainer Table;

#endif // CARDCONTAINER_H
