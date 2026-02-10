#include "../protocol/card.h"
#include "cardframe.h"
#include "mainwindow.h"
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include <QPixmap>
#include <QPalette>
#include <QPainter>

CardFrame::CardFrame( class Card* card, QWidget *parent)
    : QLabel{parent},
    card(card),
    is_dragging{false},
    dragable{true}
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    effect->setOpacity(1);
    setMouseTracking(false);
    setScaledContents(true);
    setFixedSize(80,122);
    // show();
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

        move(cardPosition+delta);
    }
    QFrame::mouseMoveEvent(event);
}

void CardFrame::mouseReleaseEvent(QMouseEvent *event) {
    if ( event->button() == Qt::LeftButton && is_dragging ) {
        is_dragging = false;

        move(cardPosition);
        QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
        effect->setOpacity(1);
        setGraphicsEffect(effect);

        std::string deck;
        mainWindow->checkForDeck(mousePosition, deck);

    }
}

bool CardFrame::isDragable() const {return dragable;}
void CardFrame::toggleDragable(bool *on) {
    if ( on == nullptr ) {
        dragable = !dragable;
    } else {
        dragable = *on;
    }
}

void CardFrame::setRotation(int rot) {
    rotation = rot;
}
int CardFrame::getRotation() const { return rotation; }

void CardFrame::paintEvent(QPaintEvent* event) {
    if ( rotation != 0 ) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPoint center = this->rect().center();
        painter.translate(center);
        painter.rotate(rotation);
        painter.translate(-center);
    }
    QLabel::paintEvent(event);
}
