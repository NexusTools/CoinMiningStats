#ifndef SETTINGS_H
#define SETTINGS_H

#include "ui_settings.h"

class Settings : public QDialog, private Ui::Settings
{
	Q_OBJECT

public:
	explicit Settings(QWidget *parent = 0);

protected:
	void changeEvent(QEvent *e);
};

#endif // SETTINGS_H
