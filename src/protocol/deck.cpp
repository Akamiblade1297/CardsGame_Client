#include "deck.h"
#include <algorithm>
#include <QPoint>

Deck::Deck( QFrame* space, QLabel* frame )
    : CardContainer{space},
    Frame{frame}
{

}

void Deck::push ( Card* card ) {
    int offset = DECK_CARD_OFFSET*(std::min(Cards.size(), size_t(DECK_MAX_VIEW)));
    CardContainer::push(card);

    const QPoint pos = Frame->pos();
    card->transform(pos.x()+offset, pos.y()+offset);

    if ( Cards.size() > 1 ) {
        Cards[Cards.size()-2]->setDraggable(false);
    }

    if ( Cards.size() > DECK_MAX_VIEW ) {
        Cards[Cards.size()-(DECK_MAX_VIEW+1)]->setOpacity(0);
        for ( int i = 1 ; i <= DECK_MAX_VIEW ; i++ ) {
            Card* crd = Cards[Cards.size()-i];
            crd->transform(crd->x()-DECK_CARD_OFFSET, crd->y()-DECK_CARD_OFFSET);
        }
    }
}

int Deck::move ( std::string si, std::string sx, std::string sy, CardContainer* container, std::string num ) {
    return -1;
}
int Deck::rotate ( std::string si, std::string srot ) {
    return -1;
}

int Deck::pop_and_move ( std::string sx, std::string sy, CardContainer* container, std::string num ) {
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
    Card* card = Cards.back();
    if ( card->parse_card(num) != 0 ) return -1;
    card->transform(x, y);
    Cards.pop_back();

    container->push(card);
    card->Container = container;

    if ( Cards.size() >= DECK_MAX_VIEW ) {
        int offset = DECK_CARD_OFFSET*Cards.size();
        Cards[Cards.size()-DECK_MAX_VIEW]->setOpacity(1);
        for ( int i = 0 ; i < DECK_MAX_VIEW ; i++ ) {
            Card* card = Cards[Cards.size()-i];
            card->transform(card->y()+offset, card->y()+offset);
        }
    }
    return 0;
}

QLabel* Deck::frame() const {
    return Frame;
}

Deck Treasures(nullptr, nullptr);
Deck Trapdoors(nullptr, nullptr);
