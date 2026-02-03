#include "card.h"
#include "deck.h"
#include "mainwindow.h"
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

CardFrame::CardFrame(QWidget *parent)
    : QFrame{parent},
    is_dragging{false},
    dragable{true},
    currentDeck{nullptr}
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    effect->setOpacity(1);
    setMouseTracking(false);
}

void CardFrame::mousePressEvent(QMouseEvent *event) {
    if ( event->button() == Qt::LeftButton && dragable ) {
        is_dragging = true;
        cardPosition = pos();
        mousePosition = event->globalPos();

        setCursor(Qt::ClosedHandCursor);

        raise();

        QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
        effect->setOpacity(0.8);
    }
    QFrame::mousePressEvent(event);
}

void CardFrame::mouseMoveEvent(QMouseEvent *event) {
    if ( is_dragging ) {
        QPoint delta = event->globalPos() - mousePosition;
        mousePosition = event->globalPos();

        cardPosition += delta;
        move(cardPosition);
    }
    QFrame::mouseMoveEvent(event);
}

void CardFrame::mouseReleaseEvent(QMouseEvent *event) {
    if ( event->button() == Qt::LeftButton && is_dragging ) {
        is_dragging = false;

        QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
        effect->setOpacity(1);
        setGraphicsEffect(effect);

        DeckFrame* deck;
        emit mainWindow->checkForDeck(mousePosition, deck);
        if ( deck != nullptr && deck != currentDeck ) {
            if ( currentDeck != nullptr) {
                CardFrame* card;
                emit currentDeck->popCard(card);
                if ( card != this )
                    throw std::runtime_error("Cards are insane");
            }
            emit deck->pushCard(this);
        } else if ( currentDeck != nullptr ) {
            CardFrame* card;
            emit currentDeck->popCard(card);
            if ( card != this )
                throw std::runtime_error("Cards are insane");
        }
    }
}

bool CardFrame::isDragable() {return dragable;}
void CardFrame::toggleDragable(bool *on) {
    if ( on == nullptr ) {
        dragable = !dragable;
    } else {
        dragable = *on;
    }
}

void CardFrame::setCurrentDeck(DeckFrame* deck) {currentDeck = deck;}
