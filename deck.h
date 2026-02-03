#ifndef DECK_H
#define DECK_H

#include <QFrame>

#define DECK_CARD_OFFSET 2
#define DECK_MAX_CARDS 5

class CardFrame;

class DeckFrame : public QFrame {
    Q_OBJECT
public:
    explicit DeckFrame ( QWidget* parent );

private:
    std::vector<CardFrame*> cards;

private slots:
    void on_PushCard( CardFrame* card );
    void on_PopCard ( CardFrame*& card );

signals:
    void pushCard( CardFrame* card );
    void popCard( CardFrame*& card );
};

#endif
