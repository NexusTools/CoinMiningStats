#ifndef COLORINDICATORLABEL_H
#define COLORINDICATORLABEL_H

#include <QLabel>
#include <QTimer>

class ColorIndicatorLabel : public QLabel
{
    Q_OBJECT
public:
    enum Mode {
        Other,
        BitCoins,
        HashRate
    };

    explicit ColorIndicatorLabel(QWidget *parent);

    inline qreal value() const{return v;}
    inline void setUpColor(Qt::GlobalColor c){upColor = c;}
    inline void setDownColor(Qt::GlobalColor c){downColor = c;}
    inline void setMode(Mode n){m=n;}

    void paintEvent(QPaintEvent *);

public slots:
    void updateColor();
    void setInverted(bool i);
    void exchangeRateChanged(float e, char c);
    void setValue(qreal r);

private:
    Qt::GlobalColor upColor, downColor;
    qreal r, g, b;
    bool inverted;

    QTimer updateTimer;
    qreal v, dv;
    float e;
    char c;
    Mode m;
};

#endif // COLORINDICATORLABEL_H
