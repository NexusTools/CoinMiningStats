#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#ifdef DBUS_NOTIFICATIONS
#include <QDBusInterface>
#endif

#include <QActionGroup>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QProcess>
#include <QTimer>
#include <QTime>

class ManageMiners;
class Graph;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);

    static void shutdown();
    
protected:
    void changeEvent(QEvent *e);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

public slots:
    void requestBlockInfoUpdate();
    void requestPoolStatsUpdate();
    void requestAccountDataUpdate();
    void requestCurrencyExchangeRate();
    void displayCurrencyChanged(QAction*);

    void exchangeRateReply();
    void accountDataReply();
    void poolStatsReply();
    void blockInfoReply();

    void minersUpdated(QVariantMap, bool store=true);
    void graphDestroyed();
    void minerManagementDestroyed();
    void toggleWidget();
    void setWidget(bool);
    void showGraph();
    void showMinerManagement();
    void toggleVisible();
    void changeApiToken();
    void updateSelectedMiner(QAction* =0);
    void minerStateChanged(QProcess::ProcessState);

    bool isMinerBusy();
    void checkIdle();
    void idleControlUpdated();
    void passStdOut();
    void passStdErr();

    void stopMiner();
    void toggleMiner();
    void startMiner(QString name =QString());

private slots:
    void finishTransform();
    void showMessage(QString title, QString message);

signals:
    void receivedPoolStatsData(QVariantMap data);
    void receivedAccountData(QVariantMap data);
    void receivedBlockInfoData(QVariantMap data);
    void exchangeRateChanged(float, char);
    void invertChanged(bool);

private:
    friend class ColorIndicatorLabel;

    QString apiKey;
    QString activeCurrency;
    float exchangeRate;

    QTime lastMouseMove;
    QPoint lastMousePos;
    QTimer idleWatcher;
    QTimer killMiner;
    static QProcess miner;
    QActionGroup* minerGroup;
    QAction* windowVisibilityAction;
    QAction* trayHashRate;
    QSystemTrayIcon* trayIcon;
    QNetworkAccessManager accessMan;
    QNetworkReply* exchangeRateRequest;
    QNetworkReply* accountDataRequest;
    QNetworkReply* poolStatsRequest;
    QNetworkReply* blockInfoRequest;
    QTimer updateAccountDataTimer;
    QTimer updateBlockInfoTimer;
    QTimer updateExchangeRate;

    QPoint dragPoint;
    QSettings settings;
    bool widgetMode;

    QActionGroup currencies;
    ManageMiners* miners;
    Graph* graph;

    // Cached Values
    qreal cw;
    qreal uw;
    qreal ew;

#ifdef DBUS_NOTIFICATIONS
    QDBusInterface DBusNotificationInterface;
#endif
};

#endif // MAINWINDOW_H
