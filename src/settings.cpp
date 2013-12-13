#include "settings.h"

Settings::Settings(QWidget *parent) :
	QDialog(parent) {
	setupUi(this);
	listWidget->setFocus();
	listWidget->setCurrentRow(0);
	connect(listWidget, SIGNAL(currentRowChanged(int)), stackedWidget, SLOT(setCurrentIndex(int)));
	setAttribute(Qt::WA_DeleteOnClose);
	show();
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
