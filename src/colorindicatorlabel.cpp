#include "colorindicatorlabel.h"
#include "mainwindow.h"

#include <QPainter>
#include <QDebug>

ColorIndicatorLabel::ColorIndicatorLabel(QWidget *parent) :
	QLabel(parent) {
	if(parent->window())
		connect(parent->window(), SIGNAL(invertChanged(bool)), this, SLOT(setInverted(bool)));

	currentMode = Other;
	dv = 0;
	displayType = 1;
	currentValue = 0;

	r = 0;
	g = 0;
	b = 0;

	inverted = false;
	upColor = Qt::darkGreen;
	downColor = Qt::darkRed;

	updateTimer.setInterval(33);
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateColor()));
}

void ColorIndicatorLabel::paintEvent(QPaintEvent *) {
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

void ColorIndicatorLabel::setInverted(bool inverted) {
	if(this->inverted == inverted)
		return;

	this->inverted = inverted;

	r = 1.0-r;
	g = 1.0-g;
	b = 1.0-b;

	updateTimer.start();
}

void ColorIndicatorLabel::exchangeRateChanged(float displayValue, QChar displayPrefix) {
	this->displayType = displayValue;
	this->displayPrefix = displayPrefix;
	if(currentMode == Coins) {
		setText(QString("%1%2").arg(displayPrefix).arg(QString::number(currentValue * displayValue, 'f', displayValue == 1 ? 8 : 2)));
	}
}

void ColorIndicatorLabel::setValue(qreal newValue, BaseSuffix baseSuffix) {
	if(newValue == this->currentValue)
		return;

	QColor newColor;
	if(this->currentValue == -1 || newValue == -1)
		newColor = inverted ? Qt::white : Qt::black;
	else {
		if(newValue > this->currentValue)
			newColor = upColor;
		else
			newColor = downColor;
		if(inverted)
			newColor = newColor.lighter();
	}
	r = newColor.redF();
	g = newColor.greenF();
	b = newColor.blueF();
	updateTimer.start();

	this->currentValue = newValue;

	switch(currentMode) {
		case Coins:
			setText(QString("%1%2").arg(displayPrefix).arg(QString::number(newValue * displayType, 'f', displayType == 1 ? 8 : 2)));
		break;

		case HashRate:
		{
			qreal finalNewValue = newValue;
			char suffix = QChar::Null;
			switch(baseSuffix) {
				case None:
				break;
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
				if(finalNewValue > 1000) {
					finalNewValue /= 1000;
					suffix = 'M';
				}
			if(suffix == 'M')
				if(finalNewValue > 1000) {
					finalNewValue /= 1000;
					suffix = 'G';
				}
			if(suffix == 'G')
				if(finalNewValue > 1000) {
					finalNewValue /= 1000;
					suffix = 'T';
				}
			setText(QString("%1%2H/s").arg(finalNewValue).arg(suffix));
		break;
		}

		case Other:
			setText(QString::number(newValue));
		break;
	}

}
