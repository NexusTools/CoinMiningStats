#ifndef SETTINGS_H
#define SETTINGS_H

#include "ui_settings.h"

class Settings : public QDialog, private Ui::Settings
{
	Q_OBJECT

public:
	explicit Settings(QVariantMap settings, QWidget *parent = 0);

protected:
	QVariantMap settingsEntry;
	void changeEvent(QEvent *e);

private slots:
	void updateHost();
	void updateKey();
	void commitSettings();

signals:
	void dataUpdated(QVariantMap);
};

#endif // SETTINGS_H
