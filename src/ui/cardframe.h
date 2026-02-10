#ifndef CARDFRAME_H
#define CARDFRAME_H

#include <QLabel>

class Card;
class CardFrame : public QLabel
{
    Q_OBJECT
public:
    explicit CardFrame( Card* card, QWidget *parent = nullptr );
    bool isDragable () const;
    void toggleDragable ( bool* on );
    void setRotation( int rot );
    int getRotation() const;
    int index;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent ( QPaintEvent* event ) override;

private:
    int rotation;
    bool is_dragging;
    bool dragable;
    QPoint cardPosition;
    QPoint mousePosition;
    Card* card;

private slots:

signals:

};

#endif // CARDFRAME_H
