#include "cardcontainer.h"
#include <stdexcept>

CardContainer::CardContainer ( QFrame* space )
    : Space{space}
{

}

Card* CardContainer::operator[] ( int i ) {
    if ( i < 0 || i > Cards.size() ) return nullptr;
    return Cards[i];
}

Card* CardContainer::operator[] ( std::string si ) {
    int i;
    try {
        i = std::stoi(si);
    } catch ( std::out_of_range ) {
      return nullptr;
    } catch ( std::invalid_argument ) {
      return nullptr;
    }
    return operator[](i);
}

Card* CardContainer::at ( std::string si ) {
    return operator[](si);
}

Card* CardContainer::at ( int i ) {
    return operator[](i);
}

void CardContainer::push ( Card* card ) {
    Cards.push_back(card);
    card->Container = this;
}

void CardContainer::clear() {
    Cards.erase(Cards.begin(), Cards.begin()+Cards.size());
}

int CardContainer::move ( std::string si, std::string sx, std::string sy, CardContainer* container, std::string num ) {
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
    if ( Cards[i]->parse_card(num) != 0 ) return -1;
    Cards[i]->transform(x, y);
    container->push(Cards[i]);
    Cards.erase(Cards.begin()+i);
    return 0;
}

int CardContainer::rotate ( std::string si, std::string srot ) {
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
    return Cards[i]->rotate(rot);
}

QFrame* CardContainer::space() const {
    return Space;
}

int CardContainer::size() const {
    return Cards.size();
}

bool CardContainer::empty() const {
    return Cards.empty();
}

CardContainer Table(nullptr);
