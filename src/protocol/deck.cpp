#include "deck.h"
#include <algorithm>
#include <QPoint>

Deck::Deck( QFrame* space, QLabel* frame )
    : CardContainer{space},
    Frame{frame}
{

}

void Deck::push ( Card* card, Card** updated_cards ) {
    int offset = DECK_CARD_OFFSET*(std::min(Cards.size(), size_t(DECK_MAX_VIEW)));
    CardContainer::push(card, updated_cards);

    const QPoint pos = Frame->pos();
    card->transform(pos.x()+offset, pos.y()+offset);

    if ( Cards.size() > 1 ) {
        Cards[Cards.size()-2]->setDraggable(false);
    }

    if ( Cards.size() > DECK_MAX_VIEW ) {
        Card* card = Cards[Cards.size()-(DECK_MAX_VIEW+1)];
        card->setOpacity(0);
        if ( updated_cards != nullptr )
            updated_cards[1] = card;
        for ( int i = 1 ; i <= DECK_MAX_VIEW ; i++ ) {
            card = Cards[Cards.size()-i];
            card->transform(card->x()-DECK_CARD_OFFSET, card->y()-DECK_CARD_OFFSET);
            if ( updated_cards != nullptr )
                updated_cards[DECK_MAX_VIEW+2-i] = card;
        }
        if ( updated_cards != nullptr )
            updated_cards[DECK_MAX_VIEW+2] = nullptr;
    }
}

int Deck::move ( std::string si, std::string sx, std::string sy, CardContainer* container, std::string num, Card** updated_cards ) {
    return -1;
}
int Deck::rotate ( std::string si, std::string srot ) {
    return -1;
}

int Deck::pop_and_move ( std::string sx, std::string sy, CardContainer* container, std::string num, Card** updated_cards ) {
    int x, y;
    try {
        x = std::stoi(sx);
        y = std::stoi(sy);
    } catch ( std::out_of_range ) {
        return -3;
    } catch ( std::invalid_argument ) {
        return -3;
    }
    if ( Cards.empty() ) return -1;
    Card* card = Cards.back();
    if ( card->parse_card(num) != 0 ) return -2;
    card->transform(x, y);
    Cards.pop_back();
    Cards[Cards.size()-1]->setDraggable(true);

    container->push(card, updated_cards);
    card->Container = container;

    if ( Cards.size() >= DECK_MAX_VIEW ) {
        card = Cards[Cards.size()-DECK_MAX_VIEW];
        card->setOpacity(1);
        size_t upc_idx = 1;
        if ( updated_cards != nullptr ) { 
            if ( updated_cards[1] != 0 )
                upc_idx = DECK_MAX_VIEW+2;
            else
                updated_cards[DECK_MAX_VIEW+1] = 0;
            updated_cards[upc_idx] = card;
        }
        for ( int i = 1 ; i < DECK_MAX_VIEW ; i++ ) {
            card = Cards[Cards.size()-i];
            card->transform(card->x()+DECK_CARD_OFFSET, card->y()+DECK_CARD_OFFSET);
            if ( updated_cards != nullptr )
                updated_cards[DECK_MAX_VIEW+upc_idx-i] = card;
        }
    }
    return 0;
}

QLabel* Deck::frame() const {
    return Frame;
}

Deck Treasures(nullptr, nullptr);
Deck Trapdoors(nullptr, nullptr);
