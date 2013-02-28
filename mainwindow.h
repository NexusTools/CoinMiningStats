#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>

class Graph;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    
protected:
    void changeEvent(QEvent *e);

public slots:
    void requestUpdate();
    void graphDestroyed();
    void showGraph();
    void gotReply();

signals:
    void receivedPoolData(QVariantMap data);
    void receivedAccountData(QVariantMap data);

private:
    QString apiKey;


    QNetworkAccessManager accessMan;
    QNetworkReply* networkReply;
    QTimer updateTimer;

    QSettings settings;
    Graph* graph;
};

#endif // MAINWINDOW_H
