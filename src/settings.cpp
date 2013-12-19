#include "settings.h"
#include <QDebug>

Settings::Settings(QVariantMap settings, QWidget *parent) :
	QDialog(parent) {
	setupUi(this);
	settingsEntry = settings;

	host->setCurrentText(settingsEntry.value("apiHost").toString());
	key->setText(settingsEntry.value("apiHostKey").toString());

	listWidget->setFocus();
	listWidget->setCurrentRow(0);
	connect(listWidget, SIGNAL(currentRowChanged(int)), stackedWidget, SLOT(setCurrentIndex(int)));
	connect(host, SIGNAL(currentTextChanged(QString)), this, SLOT(updateHost()));
	connect(key, SIGNAL(textChanged(QString)), this, SLOT(updateKey()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(commitSettings()));
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

void Settings::updateHost() {
	settingsEntry.insert("apiHost", host->currentText());
}

void Settings::updateKey() {
	settingsEntry.insert("apiHostKey", key->text());
}

void Settings::commitSettings() {
	qDebug() << "Commiting Settings " << settingsEntry;
	emit dataUpdated(settingsEntry);
}
