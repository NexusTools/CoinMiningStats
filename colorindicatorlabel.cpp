#include "colorindicatorlabel.h"

#include <QPainter>
#include <QDebug>

ColorIndicatorLabel::ColorIndicatorLabel(QWidget *parent) :
    QLabel(parent)
{
    if(parent->window())
        connect(parent->window(), SIGNAL(invertChanged(bool)), this, SLOT(setInverted(bool)));

    m = Other;
    dv = 0;
    v = 0;

    r = 0;
    g = 0;
    b = 0;

    inverted = false;
    upColor = Qt::darkGreen;
    downColor = Qt::darkRed;

    updateTimer.setInterval(33);
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateColor()));
}

void ColorIndicatorLabel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setFont(font());
    p.setPen(QColor::fromRgbF(r, g, b));
    p.drawText(QRect(QPoint(0, 0), size()), text());
}

void ColorIndicatorLabel::updateColor(){
    qDebug() << "Updating Color" << inverted;

    if(inverted && (r < 1 || g < 1 || b < 1)) {
        r += ((1.0 - r) + 0.014) / 14;
        g += ((1.0 - g) + 0.014) / 14;
        b += ((1.0 - b) + 0.014) / 14;
        if(r > 1)
            r = 1;
        if(g > 1)
            g = 1;
        if(b > 1)
            b = 1;
    } else if(!inverted && (r > 0 || g > 0 || b > 0)) {
        r -= (r + 0.014) / 14;
        g -= (g + 0.014) / 14;
        b -= (b + 0.014) / 14;
        if(r < 0)
            r = 0;
        if(g < 0)
            g = 0;
        if(b < 0)
            b = 0;
    } else
        updateTimer.stop();

    repaint();
}

void ColorIndicatorLabel::setInverted(bool i)
{
    if(inverted == i)
        return;

    inverted = i;

    r = 1.0-r;
    g = 1.0-g;
    b = 1.0-b;

    updateTimer.start();
}

void ColorIndicatorLabel::setValue(qreal v){
    if(v == this->v)
        return;

    qDebug() << this->value() << v;
    QColor n;
    if(this->v == -1 || v == -1)
        n = inverted ? Qt::white : Qt::black;
    else {
        if(v > this->v)
            n = upColor;
        else
            n = downColor;
        if(inverted)
            n = n.lighter();
    }
    r = n.redF();
    g = n.greenF();
    b = n.blueF();
    updateTimer.start();

    this->v = v;

    switch(m){
        case BitCoins:
            setText(QString::number(v, 'f', 8));
            break;

        case HashRate:
        {
            qreal hr = v;
            char suffix = 'M';
            if(hr > 1000) {
                hr /= 1000;
                suffix = 'G';
            }
            if(hr > 1000) {
                hr /= 1000;
                suffix = 'T';
            }
            setText(QString("%1%2H/s").arg(hr).arg(suffix));
            break;
        }

        case Other:
            setText(QString::number(v));
            break;
    }

}
