#ifndef CARD_H
#define CARD_H

#define CARD_COUNT 170
#define CARD_TRESH 95

#include <string>
#include <QtNumeric>

enum CardType {
  TRAP = -1,
  TRES = -2,
};

class CardFrame;
class CardContainer;
/**
 * @brief The Card class
 */
class Card {
private:
  int Number;
  CardFrame* Frame;
  int X;
  int Y;
  int Rot;
  qreal Opacity;
public:
  CardContainer* Container;
  /**
   * @brief Card
   * @param num Card Number
   * @param x X coordinate
   * @param y Y coordinate
   * @param rot Rotation
   */
  Card ( int num = TRAP, CardFrame* frame = nullptr, int x = 0, int y = 0, int rot = 0 );
  ~Card();

  /**
   * @brief Change card position
   * @param x New X coordinate
   * @param y New Y coordinate
   */
  void transform ( int x, int y );

  /**
   * @brief Rotate card
   * @param rot New Rotation
   * @return 0 if Success, -1 if Failed
   */
  int rotate ( int rot );
  /**
   * @brief Parse and set card number from string
   * @param card String to parse
   * @return 0 if Success, -1 if Failed
   */
  int parse_card ( std::string card );
  /**
   * @brief Unparse card number to string
   * @return String reference of card number
   */
  std::string unparse_card () const;

  int x() const;
  int y() const;
  int rot() const;
  int num() const;
  qreal opacity() const;

  /**
   * @brief Set card opacity
   * @param opacity
   */
  void setOpacity( qreal opacity );

  // NEXT METHODS SHOULD ONLY BE CALLED FROM GUI THREAD
  /**
   * @brief Update CardFrame gui element SHOULD ONLY BE CALLED FROM GUI THREAD
   */
  void updateFrame();
};

#endif
