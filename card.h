#ifndef CARD_H
#define CARD_H

#include <QFrame>

class DeckFrame;

class CardFrame : public QFrame
{
    Q_OBJECT
public:
    explicit CardFrame(QWidget *parent = nullptr);
    bool isDragable ();
    void toggleDragable ( bool* on );
    void setCurrentDeck( DeckFrame* deck );

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    bool is_dragging;
    bool dragable;
    QPoint cardPosition;
    QPoint mousePosition;
    DeckFrame* currentDeck;

private slots:

signals:

};

#endif // CARD_H
