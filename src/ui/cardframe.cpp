#include "../protocol/card.h"
#include "../protocol/deck.h"
#include "../protocol/functions.h"
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
    draggable{true}
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
    if ( event->button() == Qt::LeftButton && draggable ) {
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

        mousePosition = event->globalPos();
        CardContainer* src = card->Container;
        CardContainer* dest = mainWindow->checkContainer(mousePosition);

        if ( dynamic_cast<Deck*>(src) ) {
            if ( src != dest )
                emit mainWindow->consoleIn( QString("deck %1 %2 %3 %4")
                    .arg(nameByContainer(src)).arg(nameByContainer(dest)).arg(pos().x()).arg(pos().y()),
                    false);
        } else {
            emit mainWindow->consoleIn( QString("move %1 %2 %3 %4 %5")
                    .arg(nameByContainer(src)).arg(src->find(card)).arg(nameByContainer(dest)).arg(pos().x()).arg(pos().y()),
                    false);
        }
        
        move(cardPosition);
        QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
        effect->setOpacity(1);
        setGraphicsEffect(effect);
    }
}

bool CardFrame::isDragable() const {return draggable;}
void CardFrame::setDraggable(bool drag) {
    draggable = drag;
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
