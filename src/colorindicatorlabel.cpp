#include "colorindicatorlabel.h"
#include "mainwindow.h"

#include <QPainter>
#include <QDebug>

ColorIndicatorLabel::ColorIndicatorLabel(QWidget *parent) :
	QLabel(parent)
{
	if(parent->window())
		connect(parent->window(), SIGNAL(invertChanged(bool)), this, SLOT(setInverted(bool)));

	m = Other;
	dv = 0;
	e = 1;
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

void ColorIndicatorLabel::exchangeRateChanged(float _e, QChar _c) {
	e = _e;
	c = _c;
	if(m == BitCoins) {

		if(e == 1)
			setText(QString::number(v, 'f', 8));
		else
			setText(QString("%1%2").arg(c).arg(QString::number(v * e, 'f', 2)));
	}

}

void ColorIndicatorLabel::setValue(qreal v, BaseSuffix baseSuffix){
	if(v == this->v)
		return;

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
			if(e == 1)
				setText(QString::number(v, 'f', 8));
			else
				setText(QString("%1%2").arg(c).arg(QString::number(v * e, 'f', 2)));
			break;

		case HashRate:
		{
			qreal hr = v;
			char suffix;
			switch(baseSuffix) {
				case Kilohash:
					suffix = 'K';
				break;
				case Megahash:
					suffix = 'M';
				break;
				case Gigahash:
					suffix = 'G';
				break;
				case Terahash:
					suffix = 'T';
				break;
			}

			if(suffix == 'K')
				if(hr > 1000) {
					hr /= 1000;
					suffix = 'M';
				}
			if(suffix == 'M')
				if(hr > 1000) {
					hr /= 1000;
					suffix = 'G';
				}
			if(suffix == 'G')
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
