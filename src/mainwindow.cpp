#include "mainwindow.h"
#include "manageminers.h"
#include "graph.h"
#include "settings.h"
#include "miner.h"
#include "loosejson.h"

#include <stdlib.h>
#include <QCursor>
#include <QInputDialog>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResource>
#include <QDebug>
#include <QThread>

Miner MainWindow::miner;
QNetworkAccessManager MainWindow::accessMan;

void MainWindow::shutdown(){
	if(miner.isRunning()) {
		miner.stop();
	}
	qDebug() << "Exiting...";
	_Exit(0);
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	currencies(0)
#ifdef DBUS_NOTIFICATIONS
	,DBusNotificationInterface("org.freedesktop.Notifications",
							  "/org/freedesktop/Notifications",
							  "org.freedesktop.Notifications")
#endif
{
	qApp->setQuitOnLastWindowClosed(false);
	setupUi(this);

	trayIcon = new QSystemTrayIcon(this);
	minerGroup = new QActionGroup(menuMining);
	confirmed->setMode(ColorIndicatorLabel::Coins);
	unconfirmed->setMode(ColorIndicatorLabel::Coins);
	next_reward->setMode(ColorIndicatorLabel::Coins);
	estimated->setMode(ColorIndicatorLabel::Coins);
	potential->setMode(ColorIndicatorLabel::Coins);
	workers_rate->setMode(ColorIndicatorLabel::HashRate);

	connect(this, SIGNAL(exchangeRateChanged(float,QChar)), confirmed, SLOT(exchangeRateChanged(float,QChar)));
	connect(this, SIGNAL(exchangeRateChanged(float,QChar)), unconfirmed, SLOT(exchangeRateChanged(float,QChar)));
	connect(this, SIGNAL(exchangeRateChanged(float,QChar)), next_reward, SLOT(exchangeRateChanged(float,QChar)));
	connect(this, SIGNAL(exchangeRateChanged(float,QChar)), estimated, SLOT(exchangeRateChanged(float,QChar)));
	connect(this, SIGNAL(exchangeRateChanged(float,QChar)), potential, SLOT(exchangeRateChanged(float,QChar)));



	updateExchangeRate.setSingleShot(true);
	updateExchangeRate.setInterval(60000);
	connect(&updateExchangeRate, SIGNAL(timeout()), this, SLOT(requestCurrencyExchangeRate()));

	exchangeRateRequest = 0;
	blockInfoRequest = 0;
	widgetMode = false;

	idleWatcher.setInterval(500);

	minersUpdated(settings.value("miners").toMap(), false);

	exchangeRate = 1;
	requestBlockInfoUpdate();
	connect(&idleWatcher, SIGNAL(timeout()), this, SLOT(checkIdle()));
	connect(actionIdleControl, SIGNAL(triggered()), this, SLOT(idleControlUpdated()));
	connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(graphBtn, SIGNAL(clicked()), this, SLOT(showGraph()));
	connect(tglWidget, SIGNAL(clicked()), this, SLOT(toggleWidget()));
	connect(actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));
	connect(actionManageMiners, SIGNAL(triggered()), this, SLOT(showMinerManagement()));
	connect(minerGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateSelectedMiner(QAction*)));
	connect(actionMinerControl, SIGNAL(triggered()), this, SLOT(toggleMiner()));


	connect(&miner, SIGNAL(started()), this, SLOT(minerStarted()));
	connect(&miner, SIGNAL(stopped()), this, SLOT(minerStopped()));
	connect(&miner, SIGNAL(apiDataReceived(QVariantMap)), this, SLOT(accountDataReply(QVariantMap)));

	QAction* active = 0;
	activeCurrency = settings.value("display_currency", "BTC").toString();
	foreach(QAction* action, menuCurrency->actions()) {
		if(activeCurrency == action->text())
			active = action;
		currencies.addAction(action);
	}
	if(active)
		active->setChecked(true);
	connect(&currencies, SIGNAL(triggered(QAction*)), this, SLOT(displayCurrencyChanged(QAction*)));
	requestCurrencyExchangeRate();

	workers->resizeColumnsToContents();
	dragPoint = QPoint(-1, -1);
	miners = 0;
	graph = 0;
	mainSettings = 0;

	trayIcon->setIcon(qApp->windowIcon());
	if(!QSystemTrayIcon::isSystemTrayAvailable()) {
		trayHashRate = 0;
		qWarning() << "System Tray Not Available.";
		setAttribute(Qt::WA_DeleteOnClose);
		if(qApp->arguments().contains("-m"))
			QTimer::singleShot(50, this, SLOT(showMinimized()));
	} else {
		QMenu* menu = new QMenu();

		windowVisibilityAction = new QAction("Hide Window", menu);
		connect(windowVisibilityAction, SIGNAL(triggered()), this, SLOT(toggleVisible()));
		menu->addAction(windowVisibilityAction);

		trayHashRate = new QAction("HashRate: N/A", menu);
		trayHashRate->setDisabled(true);
		menu->addAction(trayHashRate);

		menu->addSeparator();
		menu->addMenu(menuMining);
		menu->addSeparator();

		QAction* settingsAction = new QAction("Settings", menu);
		connect(settingsAction, SIGNAL(triggered()), this, SLOT(showSettings()));
		menu->addAction(settingsAction);

		QAction* quitAction = new QAction("Quit", menu);
		connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
		menu->addAction(quitAction);

		trayIcon->setToolTip("No Miner Running");
		trayIcon->setContextMenu(menu);
		trayIcon->show();

		if(qApp->arguments().contains("-m"))
			QTimer::singleShot(50, this, SLOT(hide()));
	}

	if(qApp->arguments().contains("-r"))
		QTimer::singleShot(100, this, SLOT(toggleMiner()));

	if(qApp->arguments().contains("-a") || settings.value("auto").toBool()) {
		actionMinerControl->setDisabled(true);
		actionIdleControl->setChecked(true);
	}
}

void MainWindow::showSettings() {
	if(mainSettings) {
		mainSettings->setFocus();
		return;
	}

	mainSettings = new Settings(this);
	//mainSettings->setMinerData(settings.value("mainSettings"));
	//connect(mainSettings, SIGNAL(dataUpdated(QVariantMap)), this, SLOT(minersUpdated(QVariantMap)));
	connect(mainSettings, SIGNAL(destroyed()), this, SLOT(mainSettingsDestroyed()));
}

void MainWindow::minerStarted() {
	showMessage("Started Miner", "The mining software is now running.");
	actionMinerControl->setEnabled(!actionIdleControl->isChecked());
	trayIcon->setToolTip("Miner Started");
}

void MainWindow::minerStopped() {
	showMessage("Stopped Miner", "The mining software has stopped running.");
	updateSelectedMiner(minerGroup->checkedAction());
	trayIcon->setToolTip("Miner Stopped");
}

void MainWindow::focusInEvent(QFocusEvent *)
{
	if(widgetMode)
		setWindowOpacity(0.8);
}

void MainWindow::focusOutEvent(QFocusEvent *){
	if(widgetMode)
		setWindowOpacity(0.4);
}

void MainWindow::closeEvent(QCloseEvent *){
	if(!trayIcon->isVisible())
		qApp->quit();
	else
		toggleVisible();
}


void MainWindow::keyPressEvent(QKeyEvent * k){
	if(widgetMode) {
		k->accept();
		if(k->key() == Qt::Key_Escape)
			setWidget(false);
	}
}

void MainWindow::mouseMoveEvent(QMouseEvent * m){
	if(widgetMode) {
		m->accept();
		if(dragPoint.x() > -1 && dragPoint.y() > -1)
			move(m->globalPos() - dragPoint);
	}
}

void MainWindow::mousePressEvent(QMouseEvent * m){
	if(widgetMode) {
		m->accept();
		grabMouse();
		dragPoint = m->globalPos() - pos();
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent * m){
	if(widgetMode) {
		m->accept();
		if(dragPoint.x() > -1 && dragPoint.y() > -1) {
			dragPoint = QPoint(-1, -1);
			releaseMouse();
		}
	}
}

void MainWindow::toggleVisible()
{
	setVisible(!isVisible());
	if(isVisible())
		windowVisibilityAction->setText("Hide Window");
	else
		windowVisibilityAction->setText("Show Window");
}

void MainWindow::toggleWidget()
{
	setWidget(!widgetMode);
}

void MainWindow::setWidget(bool checked)
{
	if(widgetMode == checked)
		return;

	widgetMode = checked;
	advanced->setVisible(!widgetMode);
	fileMenu->setVisible(!widgetMode);

	emit invertChanged(widgetMode);
	QTimer::singleShot(0, this, SLOT(hide()));
	QTimer::singleShot(50, this, SLOT(finishTransform()));
}

void MainWindow::finishTransform(){
	static Qt::WindowFlags oldFlags;
	static QRect oldGeometry;
	if (widgetMode)
	{
		oldFlags = windowFlags();
		QResource styleRes(":/widget.css");
		setStyleSheet(QString((const char*)styleRes.data()));
		oldGeometry = geometry();
		setWindowOpacity(0.7);
		setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
		resize(minimumSize());
		move(oldGeometry.topLeft());
	}
	else
	{
		setStyleSheet("");
		setWindowOpacity(1);
		setWindowFlags(oldFlags);
		setGeometry(oldGeometry);
	}
	show();
}

void MainWindow::minerManagementDestroyed()
{
	miners->deleteLater();
	miners = 0;
}

void MainWindow::graphDestroyed()
{
	graph->deleteLater();
	graph = 0;
}

void MainWindow::mainSettingsDestroyed()
{
	mainSettings->deleteLater();
	mainSettings = 0;
}

void MainWindow::minersUpdated(QVariantMap data, bool store){
	foreach(QAction* action, minerGroup->actions())
		menuMining->removeAction(action);

	QAction* selMiner = 0;
	QStringList miners = data.keys();
	if(miners.isEmpty())
		menuMining->addAction(actionNoMinersConfigured);
	else {
		menuMining->removeAction(actionNoMinersConfigured);
		foreach(QString miner, miners) {
			QAction* action = new QAction(miner, menuMining);
			action->setCheckable(true);
			if(settings.value("miner").toString() == miner)
				selMiner = action;
			minerGroup->addAction(action);
			menuMining->addAction(action);
		}
	}
	if(selMiner)
		selMiner->setChecked(true);
	updateSelectedMiner(selMiner);
	if(store) {
		settings.setValue("miners", data);
		settings.sync();
	}
}

void MainWindow::updateSelectedMiner(QAction* action)
{
	QString minerText = action ? action->text() : settings.value("miner").toString();
	settings.setValue("miner", minerText);

	if(!miner.isRunning()) {
		QVariantMap minerEntry = settings.value("miners").toMap().value(minerText).toMap();
		int hostType = minerEntry.value("host").toInt();
		switch(hostType) {
			case 0:
				if(actionBTC->isChecked() || actionBTC_USD->isChecked() || actionBTC_EUR->isChecked()) {
					actionLTC->setChecked(true);
					displayCurrencyChanged(actionLTC);
				}

				actionBTC->setEnabled(false);
				actionBTC->setChecked(false);
				actionBTC_USD->setEnabled(false);
				actionBTC_USD->setChecked(false);
				actionBTC_EUR->setEnabled(false);
				actionBTC_EUR->setChecked(false);

				actionLTC->setEnabled(true);
				actionLTC_USD->setEnabled(true);
				actionLTC_EUR->setEnabled(true);
			break;
			case 1:
				if(actionLTC->isChecked() || actionLTC_USD->isChecked() || actionLTC_EUR->isChecked()) {
					actionBTC->setChecked(true);
					displayCurrencyChanged(actionBTC);
				}

				actionBTC->setEnabled(true);
				actionBTC_USD->setEnabled(true);
				actionBTC_EUR->setEnabled(true);

				actionLTC->setEnabled(false);
				actionLTC->setChecked(false);
				actionLTC_USD->setEnabled(false);
				actionLTC_USD->setChecked(false);
				actionLTC_EUR->setEnabled(false);
				actionLTC_EUR->setChecked(false);
			break;
			case 2:
			break;
		}

		QVariantMap allMiners = settings.value("miners").toMap();
		QList<QAction*> actions = menuMining->actions();
		for(int i = 0; i < actions.length(); i++) {
			QVariantMap::iterator it;
			for (it = allMiners.begin(); it != allMiners.end(); ++it) {
				if(actions.at(i)->text() == it.key())
					actions.at(i)->setEnabled(true);
			}
		}

		actionMinerControl->setEnabled(!miner.isRunning() && !actionIdleControl->isChecked());
		actionMinerControl->setText(action ? QString("Start `%1`").arg(action->text()) : "Select a Miner");
	}

	actionMinerControl->setEnabled(action);
	actionIdleControl->setEnabled(action);
}

void MainWindow::checkIdle()
{
	QPoint mPos = QCursor::pos();
	if(miner.isRunning() && mPos != lastMousePos) {
		lastMouseMove.start();
		miner.stop();
	} else if(!miner.isRunning() && lastMouseMove.elapsed() > settings.value("idle_timeout", 30).toInt() * 1000) {
		lastMouseMove.start();
		miner.start();
	}
	lastMousePos = mPos;
}

void MainWindow::idleControlUpdated()
{
	settings.setValue("auto", actionIdleControl->isChecked());
	if(actionIdleControl->isChecked()) {
		idleWatcher.start();
		actionMinerControl->setEnabled(false);
	} else {
		idleWatcher.stop();
		actionMinerControl->setEnabled(!miner.isRunning());
	}
}

void MainWindow::showMessage(QString title, QString message)
{
	qDebug() << "Showing Notification" << title << message;
#ifdef DBUS_NOTIFICATIONS
	if(DBusNotificationInterface.isValid()) {
		qDebug() << "Using DBus Notifications";
		QList<QVariant> argumentList;
		argumentList << qApp->applicationName(); //app_name
		argumentList << (uint)0;  // replace_id
		argumentList << "";  // app_icon
		argumentList << title; // summary
		argumentList << message; // body
		argumentList << QStringList();  // actions
		argumentList << QVariantMap();  // hints
		argumentList << (int)2500; // timeout in ms
		QDBusMessage reply = DBusNotificationInterface.callWithArgumentList(QDBus::AutoDetect, "Notify", argumentList);
		if(reply.type() != QDBusMessage::ErrorMessage)
			return;
	}
#endif
	if(QSystemTrayIcon::supportsMessages()) {
		qDebug() << "Using Tray Icon Notifications";
		trayIcon->showMessage(title, message);
	} else
		qWarning() << "No Qt Notification Fallback";
}

void MainWindow::stopMiner(){
	showMessage("Stopping Miner", "The mining software is being stopped...");
	miner.stop();
	actionMinerControl->setEnabled(false);
}

void MainWindow::startMiner(){
	QAction* active = minerGroup->checkedAction();
	if(!active)
		return;
	QString name = active->text();


	QVariantMap minerEntry = settings.value("miners").toMap().value(name).toMap();
	if(minerEntry.isEmpty()) {
			qWarning() << "Attempted to Start Invalid Miner" << name;
			return;
	}

	QVariantMap allMiners = settings.value("miners").toMap();
	QList<QAction*> actions = menuMining->actions();
	for(int i = 0; i < actions.length(); i++) {
		QVariantMap::iterator it;
		for (it = allMiners.begin(); it != allMiners.end(); ++it) {
			if(actions.at(i)->text() == it.key())
				actions.at(i)->setEnabled(false);
		}
	}
	actionMinerControl->setEnabled(false);
	actionMinerControl->setText(QString("Stop `%1`").arg(name));

	showMessage("Starting Miner", "The mining software is being started...");
	miner.start(name, minerEntry.value("program").toString(), minerEntry.value("arguments").toStringList(), minerEntry.value("host").toInt(), minerEntry.value("hostKey").toString(), minerEntry.value("hostSecert").toString());
}

void MainWindow::showMinerManagement(){
	if(miners) {
		miners->setFocus();
		return;
	}

	miners = new ManageMiners(this);
	miners->setMinerData(settings.value("miners"));
	connect(miners, SIGNAL(dataUpdated(QVariantMap)), this, SLOT(minersUpdated(QVariantMap)));
	connect(miners, SIGNAL(destroyed()), this, SLOT(minerManagementDestroyed()));
}

void MainWindow::showGraph(){
	if(graph) {
		graph->setFocus();
		return;
	}

	graph = new Graph(this);
	connect(graph, SIGNAL(destroyed()), this, SLOT(graphDestroyed()));
	connect(this, SIGNAL(plotRateGraph(qreal)), graph, SLOT(plotRate(qreal)));
}

void MainWindow::displayCurrencyChanged(QAction* act) {
	activeCurrency = act->text();
	settings.setValue("display_currency", activeCurrency);
	qDebug() << "Currency changed to" << activeCurrency;
	requestCurrencyExchangeRate();
}

void MainWindow::requestCurrencyExchangeRate()
{
	updateExchangeRate.stop();

	if(exchangeRateRequest)
		exchangeRateRequest->deleteLater();
	QString currencyFrom;
	QString currencyTo;

	if(activeCurrency.indexOf("->") != -1) {
		QStringList tmp = activeCurrency.split("->");
		currencyFrom = tmp.at(0).toLower();
		currencyTo = tmp.at(1).toLower();
	} else {
		currencyFrom = activeCurrency.toLower();
	}

	if(currencyFrom == "btc" && currencyTo.isEmpty()) {
		emit exchangeRateChanged(1, L'฿');
		return;
	}

	if(currencyFrom == "ltc" && currencyTo.isEmpty()) {
		emit exchangeRateChanged(1, L'Ł');
		return;
	}

	exchangeRate = -1;
	exchangeRateRequest = accessMan.get(QNetworkRequest(QUrl(QString("https://btc-e.com/api/2/%1_%2/ticker").arg(currencyFrom).arg(currencyTo))));
	connect(exchangeRateRequest, SIGNAL(finished()), this, SLOT(exchangeRateReply()));
}

void MainWindow::requestBlockInfoUpdate()
{
	updateBlockInfoTimer.stop();
	qDebug() << "Requesting Block Info Update";
	if(blockInfoRequest)
		blockInfoRequest->deleteLater();
	blockInfoRequest = accessMan.get(QNetworkRequest(QUrl("http://blockchain.info/latestblock")));
	connect(blockInfoRequest, SIGNAL(finished()), this, SLOT(blockInfoReply()));
}

void MainWindow::exchangeRateReply() {
	QTimer::singleShot(0, exchangeRateRequest, SLOT(deleteLater()));

	updateExchangeRate.start();
	if(exchangeRateRequest->error()) {
		qWarning() << "Request Failed" << exchangeRateRequest->errorString();
		exchangeRateRequest = 0;
		return;

	}

	bool okay;
	QVariantMap map = LooseJSON::parse(exchangeRateRequest->readAll()).toMap();
	exchangeRate = map.value("ticker").toMap().value("buy").toFloat(&okay);

	if(!okay) {
		qCritical() << "Failed to retreive exchange rate for requested currency...";
		exchangeRate = -1;
	} else {
		qDebug() << "Exchange rate for BTC to" << activeCurrency << "is" << exchangeRate;
		QChar s = '$';

		if(activeCurrency.indexOf("->") != -1) {
			QStringList tmp = activeCurrency.split("->");
			QString currencyTo = tmp.at(1).toLower();
			if(currencyTo == "eur") {
				s = L'£';
			}
		}

		emit exchangeRateChanged(exchangeRate, s);
	}

	exchangeRateRequest = 0;
}

void MainWindow::accountDataReply(QVariantMap map)
{
	if(!map.isEmpty()) {
		QStringList knownWorkers;

		qreal totalRate = map.value("totalRate").toReal();
		emit plotRateGraph(totalRate);
		// Process Workers
		if(map.contains("totalWorkers")) {
			int numWorkers = map.value("totalWorkers").toInt();
			for(int i = 0; i < numWorkers; i++) {
				QVariantMap workerMap = map.value(QString("worker%1").arg(i)).toMap();
				QString workerName = workerMap.value("name").toString();
				knownWorkers.append(workerName);

				int row = -1;
				for(int a=0; a<workers->rowCount(); a++) {
					if(workers->item(a, 0) &&
							workers->item(a, 0)->text() == workerName) {
						row = a;
						break;
					}
				}
				if(row == -1) {
					row = workers->rowCount();
					workers->insertRow(row);
					workers->setItem(row, 0, new QTableWidgetItem(workerName));
				}

				workers->item(row, 0)->setIcon(style()->standardIcon(workerMap.value("alive").toBool()
													? QStyle::SP_MediaPlay : QStyle::SP_MediaStop));

				workers->setItem(row, 1, new QTableWidgetItem(workerMap.value("hashrate").toString()));
				workers->setItem(row, 2, new QTableWidgetItem(workerMap.value("shares").toString()));
				workers->setItem(row, 3, new QTableWidgetItem(workerMap.value("score").toString()));
			}
		}

		for(int a=workers->rowCount()-1; a>=0; a--) {
			if(!workers->item(a, 0) ||
					!knownWorkers.contains(workers->item(a, 0)->text()))
				workers->removeRow(a);
		}

		workers->resizeColumnsToContents();

		ew = map.value("estimatedReward").toReal();
		cw = map.value("confirmedReward").toReal();
		uw = map.value("unconfirmedReward").toReal();
		workers->horizontalHeader()->setVisible(true);
		// Set Labels
		workers_rate->setValue(totalRate, map.value("hashType").toInt() == 1 ? ColorIndicatorLabel::Megahash : ColorIndicatorLabel::Kilohash);
		if(trayHashRate)
			trayHashRate->setText(QString("HashRate: %1").arg(workers_rate->text()));

		confirmed->setValue(cw);
		unconfirmed->setValue(uw);
		estimated->setValue(ew);
		potential->setValue(cw + uw);
	} else
		qWarning() << "Bad Account Data Reply";
}

void MainWindow::blockInfoReply()
{
	QTimer::singleShot(0, blockInfoRequest, SLOT(deleteLater()));
	updateBlockInfoTimer.start();
	if(blockInfoRequest->error()) {
		qWarning() << "Pool Statistics Request Failed" << blockInfoRequest->errorString();

		blockInfoRequest = 0;
		return;
	}

	QVariantMap map = LooseJSON::parse("(" + blockInfoRequest->readAll() + ")").toMap();
	if(!map.isEmpty()) {
		emit receivedBlockInfoData(map);

		if(map.contains("height")) {
			uint height = map.value("height").toUInt();
			if(height > blockchain_height->value()) {
				blockchain_height->setValue(height);
			}
		}
	} else
		qWarning() << "Bad Block Info Reply";


	blockInfoRequest = 0;
}

void MainWindow::toggleMiner() {
	if(miner.isRunning())
		stopMiner();
	else
		startMiner();
}

void MainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		retranslateUi(this);
		break;
	default:
		break;
	}
}
