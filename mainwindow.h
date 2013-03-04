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
    void requestBlockInfoUpdate();
    void requestPoolStatsUpdate();
    void requestAccountDataUpdate();

    void accountDataReply();
    void poolStatsReply();
    void blockInfoReply();

    void graphDestroyed();
    void toggleWidget(bool);
    void showGraph();

signals:
    void receivedPoolStatsData(QVariantMap data);
    void receivedAccountData(QVariantMap data);
    void receivedBlockInfoData(QVariantMap data);
    void invertChanged(bool);

private:
    QString apiKey;


    QNetworkAccessManager accessMan;
    QNetworkReply* accountDataRequest;
    QNetworkReply* poolStatsRequest;
    QNetworkReply* blockInfoRequest;
    QTimer updateAccountDataTimer;
    QTimer updatePoolStatsTimer;
    QTimer updateBlockInfoTimer;

    QSettings settings;
    Graph* graph;
};

#endif // MAINWINDOW_H
