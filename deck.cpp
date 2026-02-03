#include "deck.h"
#include "card.h"
#include <QGraphicsOpacityEffect>
#include <algorithm>

DeckFrame::DeckFrame ( QWidget* parent )
    : QFrame(parent)
{
    connect(this, &DeckFrame::pushCard, this, &DeckFrame::on_PushCard);
    connect(this, &DeckFrame::popCard, this, &DeckFrame::on_PopCard);
}

void DeckFrame::on_PushCard ( CardFrame* card ) {
    if ( cards.size() > 0 ) {
        bool on = false;
        cards[cards.size()-1]->toggleDragable(&on);
    }
    QPoint position = this->pos();
    int delta = DECK_CARD_OFFSET*std::min(cards.size(), (size_t)DECK_MAX_CARDS);
    card->move(position.x()+delta, position.y()+delta);
    cards.push_back(card);
    card->setCurrentDeck(this);

    if ( cards.size() >= 6 ) {
        QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(cards[cards.size()-6]->graphicsEffect());
        effect->setOpacity(0);
        QPoint delta;
        delta.setX(DECK_CARD_OFFSET);
        delta.setY(DECK_CARD_OFFSET);
        for ( int i = 1 ; i < 6 ; i++ ) {
            CardFrame* lcard = cards[cards.size()-i];
            lcard->move(lcard->pos()-delta);
        }
    }
}

void DeckFrame::on_PopCard ( CardFrame*& card ) {
    if ( cards.size() >= 6 ) {
        QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(cards[cards.size()-6]->graphicsEffect());
        effect->setOpacity(1);
        QPoint delta;
        delta.setX(DECK_CARD_OFFSET);
        delta.setY(DECK_CARD_OFFSET);
        for ( int i = 2 ; i < 6 ; i++ ) {
            CardFrame* lcard = cards[cards.size()-i];
            lcard->move(lcard->pos()+delta);
        }
    }
    card = cards.back();
    cards.pop_back();
    if ( cards.size() > 0 ) {
        bool on = true;
        cards[cards.size()-1]->toggleDragable(&on);
    }
    card->setCurrentDeck(nullptr);
}
