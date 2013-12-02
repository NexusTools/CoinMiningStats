#include "settings.h"

Settings::Settings(QWidget *parent) :
	QDialog(parent) {
	setupUi(this);
}

void Settings::changeEvent(QEvent *e) {
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		retranslateUi(this);
		break;
	default:
		break;
	}
}
