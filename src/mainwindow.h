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
class Miner;
class Settings;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);

	static QNetworkAccessManager accessMan;
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
	void requestCurrencyExchangeRate();
	void displayCurrencyChanged(QAction*);

	void exchangeRateReply();
	void accountDataReply(QVariantMap data);
	void blockInfoReply();

	void minersUpdated(QVariantMap, bool store=true);
	void graphDestroyed();
	void minerManagementDestroyed();
	void mainSettingsDestroyed();
	void toggleWidget();
	void setWidget(bool);
	void showGraph();
	void showMinerManagement();
	void showSettings();
	void toggleVisible();
	void updateSelectedMiner(QAction* =0);

	void aboutNexusTools();
	void supportNexusTools();
	void aboutQt();

	void checkIdle();
	void idleControlUpdated();

	void stopMiner();
	void toggleMiner();
	void startMiner();

private slots:
	void finishTransform();
	void showMessage(QString title, QString message);
	void minerStarted();
	void minerStopped();

signals:
	void plotRateGraph(qreal hashrate);
	void receivedBlockInfoData(QVariantMap data);
	void exchangeRateChanged(float, QChar);
	void invertChanged(bool);

private:
	friend class ColorIndicatorLabel;

	QString activeCurrency;
	float exchangeRate;

	QTime lastMouseMove;
	QPoint lastMousePos;
	QTimer idleWatcher;
	QActionGroup* minerGroup;
	QAction* windowVisibilityAction;
	QAction* trayHashRate;
	QSystemTrayIcon* trayIcon;
	QNetworkReply* exchangeRateRequest;
	QNetworkReply* blockInfoRequest;
	QTimer updateExchangeRate;

	QPoint dragPoint;
	QSettings settings;
	bool widgetMode;

	static Miner miner;

	QActionGroup currencies;
	ManageMiners* miners;
	Graph* graph;
	Settings* mainSettings;

	// Cached Values
	qreal cw;
	qreal uw;
	qreal ew;

#ifdef DBUS_NOTIFICATIONS
	QDBusInterface DBusNotificationInterface;
#endif
};

#endif // MAINWINDOW_H
