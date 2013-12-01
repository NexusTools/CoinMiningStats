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
		Coins,
		HashRate
	};

	enum BaseSuffix {
		None,
		Kilohash,
		Megahash,
		Gigahash,
		Terahash
	};

	explicit ColorIndicatorLabel(QWidget *parent);

	inline qreal value() const{return currentValue;}
	inline void setUpColor(Qt::GlobalColor upColor){this->upColor = upColor;}
	inline void setDownColor(Qt::GlobalColor downColor){this->downColor = downColor;}
	inline void setMode(Mode newMode){currentMode=newMode;}

	void paintEvent(QPaintEvent *);

public slots:
	void updateColor();
	void setInverted(bool inverted);
	void exchangeRateChanged(float displayType, QChar displayPrefix);
	void setValue(qreal r, BaseSuffix baseSuffix=None);

private:
	Qt::GlobalColor upColor, downColor;
	qreal r, g, b;
	bool inverted;

	QTimer updateTimer;
	qreal currentValue, dv;
	float displayType;
	QChar displayPrefix;
	Mode currentMode;
};

#endif // COLORINDICATORLABEL_H
