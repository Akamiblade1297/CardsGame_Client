#include "card.h"
#include "cardcontainer.h"
#include "../ui/cardframe.h"
#include <QGraphicsOpacityEffect>
#include <QString>
#include <QPixmap>

Card::Card ( int num, CardFrame* frame, int x, int y, int rot )
    : Number{num},
    Frame{frame},
    Rot{rot},
    X{x},
    Y{y},
    Opacity{1}
{
    if ( Frame != nullptr ) {
        QMetaObject::invokeMethod(Frame, [this]() {
            updateFrame();
        }, Qt::QueuedConnection);
    }
}

Card::~Card() {
    Frame->deleteLater();
    delete Frame;
}

void Card::transform( int x, int y ) {
    X = x;
    Y = y;
}

int Card::rotate( int rot ) {
    if ( rot < 0 || rot > 360 ) {
        return -1;
    }
    Rot = rot;
    return 0;
}

int Card::parse_card( std::string card ) {
    try {
        Number = std::stoi(card);
        if ( Number < 0 || Number > CARD_COUNT )
            return -1;
    } catch ( std::invalid_argument ) {;} catch ( std::out_of_range ) { return -1; }
    if ( card == "TRAP" ) Number = TRAP;
    else if ( card == "TRES" ) Number = TRES;
    else return -1;
    return 0;
}

std::string Card::unparse_card() const {
    if ( Number == TRAP ) return "TRAP";
    else if ( Number == TRES ) return "TRES";
    else if ( Number >= 0 && Number < CARD_COUNT ) return std::to_string(Number);
    else return "";
}

void Card::setOpacity( qreal opacity ) {
    Opacity = opacity;
}

int Card::x() const {
    return X;
}
int Card::y() const {
    return Y;
}
int Card::rot() const {
    return Rot;
}
int Card::num() const {
    return Number;
}
qreal Card::opacity() const {
    return Opacity;
}

void Card::updateFrame() {
    if ( Frame == nullptr || Frame->parent() != Container->space() ) {
        delete Frame;
        Frame = new CardFrame(this, Container->space());
        Frame->show();
    }

    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(Frame->graphicsEffect());
    if ( effect == nullptr ) {
        effect = new QGraphicsOpacityEffect(0);
        Frame->setGraphicsEffect(effect);
    }
    QString snum = QString::fromStdString(unparse_card());
    if ( snum == "" ) throw std::runtime_error("Invalid card number");
    QPixmap img(":/Cards/"+snum+".png");
    QPoint pos;
    pos.setX(X);
    pos.setY(Y);
    Frame->setRotation(Rot);
    Frame->setPixmap(img);
    Frame->move(pos);

    effect->setOpacity(Opacity);
}
