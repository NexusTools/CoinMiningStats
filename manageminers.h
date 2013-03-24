#ifndef MANAGEMINERS_H
#define MANAGEMINERS_H

#include "ui_manageminers.h"

class ManageMiners : public QDialog, private Ui::ManageMiners
{
    Q_OBJECT
    
public:
    explicit ManageMiners(QWidget *parent = 0);
    void setMinerData(QVariant);

public slots:
    void addMinerEntry();
    void removeMinerEntry();
    void updateMinerPage();
    void updateArguments();
    void addArg();
    void removeArg();
    void moveArgUp();
    void moveArgDown();
    void storePage();
    void save();
    void browseProgram();
    
protected:
    void changeEvent(QEvent *e);

signals:
    void dataUpdated(QVariantMap);

private:
    QVariantMap minerData;
};

#endif // MANAGEMINERS_H
