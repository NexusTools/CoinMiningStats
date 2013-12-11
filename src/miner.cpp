#include "miner.h"
#include <QDebug>
#include <QDir>
#include "mainwindow.h"
#include "loosejson.h"

Miner::Miner(QObject *parent) :
	QObject(parent) {
	apiTimer.setInterval(30000);
	connect(&minerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(minerStateChanged(QProcess::ProcessState)));

	startMinerTimer.setInterval(100);
	stopMinerTimer.setInterval(100);
	connect(&minerProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(passStdOut()));
	connect(&minerProcess, SIGNAL(readyReadStandardError()), this, SLOT(passStdErr()));

	connect(&startMinerTimer, SIGNAL(timeout()), this, SLOT(checkIfItHasStarted()));
	connect(&stopMinerTimer, SIGNAL(timeout()), this, SLOT(checkIfItHasStopped()));

	connect(&apiTimer, SIGNAL(timeout()), this, SLOT(requestAPIData()));

}

void Miner::requestAPIData() {
	if(apiKey.trimmed().isEmpty())
		return;
	qDebug() << name << "Requesting API Data";
	QString hostURL;
	switch(apiHost) {
		case 0:
			hostURL = QString("https://www.wemineltc.com/api?api_key=%1").arg(apiKey);
		break;

		case 1:
			hostURL = QString("https://mining.bitcoin.cz/accounts/profile/json/%1").arg(apiKey);
		break;

		default:
		return;
	}

	apiDataRequester = MainWindow::accessMan.get(QNetworkRequest(QUrl(hostURL)));
	connect(apiDataRequester, SIGNAL(finished()), this, SLOT(apiDataReply()));
}

void Miner::apiDataReply() {
	if(apiDataRequester->error()) {//TODO: Notify user of request failure.
		qWarning() << "Request Failed" << apiDataRequester->errorString();
		return;
	}
	QVariantMap returnableValues;

	qreal totalRate = 0;
	QVariantMap map = LooseJSON::parse(apiDataRequester->readAll()).toMap();
	if(map.isEmpty()) {
		qWarning() << "Bad API Data Reply";
	}

	if(apiHost == 0 || apiHost == 1) {
		qDebug() << map.keys();

		if(map.contains("workers")) {
			QVariantMap workersMap = map.value("workers").toMap();
			QVariantMap::iterator workerMapEntry;
			returnableValues.insert("totalWorkers", workersMap.count());
			int workerIndex = 0;
			for (workerMapEntry = workersMap.begin(); workerMapEntry != workersMap.end(); ++workerMapEntry) {
				QString workerName = workerMapEntry.key();
				QVariantMap workerEntry = workerMapEntry.value().toMap();

				QVariantMap worker;
				worker.insert("name", workerName);


				worker.insert("name", workerName);
				if(apiHost == 1)
					totalRate += workerEntry.value("hashrate").toFloat();

				worker.insert("alive", workerEntry.value("alive").toBool());
				worker.insert("hashrate", QString("%1%2").arg(workerEntry.value("hashrate").toString()).arg(apiHost == 0 ? "KH/s" : "MH/s"));
				if(apiHost == 1) {
					worker.insert("shares", workerEntry.value("shares").toString());
					worker.insert("score", workerEntry.value("score").toString());
				}
				returnableValues.insert(QString("worker%1").arg(workerIndex), worker);
				workerIndex++;
			}
		}
		if(apiHost == 0)
			totalRate = map.value("total_hashrate").toFloat();
		returnableValues.insert("hashType", apiHost);
		returnableValues.insert("totalRate", totalRate);
		returnableValues.insert("estimatedReward", map.value(apiHost == 1 ? "estimated_reward" : "round_estimate").toReal());
		returnableValues.insert("confirmedReward", map.value(apiHost == 1 ? "confirmed_reward" : "confirmed_rewards").toReal());
		if(apiHost == 1)
			returnableValues.insert("unconfirmedReward", map.value("unconfirmed_reward").toReal());
	}
	emit apiDataReceived(returnableValues);
	apiDataRequester->deleteLater();
}

void Miner::passStdOut() {
	QString stdData = minerProcess.readAllStandardOutput().data();
	qDebug() << stdData;
	*this->logStream << stdData;

}

void Miner::passStdErr() {
	QString errData = minerProcess.readAllStandardError().data();
	qDebug() << errData;
	*this->logStream << errData << endl;
}

void Miner::init(QString name, QString applicationPath, QStringList applicationArguments, int apiHost, QString apiKey, QString apiSecert) {
	apiTimer.stop();
	stop();
	this->name = name;
	this->applicationPath = applicationPath;
	this->applicationArguments = applicationArguments;
	this->apiHost = apiHost;
	this->apiKey = apiKey;
	this->apiSecert = apiSecert;
	logFile = new QFile(QDir::homePath().append(QDir::separator()).append("CoinMiningStats - ").append(name).append(".log"));

	requestAPIData();
	apiTimer.start();
}

void Miner::start() {
	stop();

	startMinerTimer.start();
	minerProcess.start(applicationPath, applicationArguments);
	if(!logFile->open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::Append)) {
		qCritical() << "Could not open logging file!";
		logStream = 0;
	} else {
		logStream = new QTextStream(logFile);
		*this->logStream << endl << endl << "Started at " << QDateTime::currentDateTime().toString() << endl;
	}
}

void Miner::stop() {
	if(!minerProcess.isOpen())
		return;
	stopMinerTimer.start();
	minerProcess.close();
	minerProcess.waitForFinished(3000);
	if(minerProcess.state() != QProcess::NotRunning) {
		qWarning() << "Force killing" << name;
		if(logStream)
			*this->logStream << "Force killing!" << endl;
		minerProcess.kill();
	}
	if(logStream)
		*this->logStream << "Stopped." << endl;
	if(logFile)
		logFile->close();
}

bool Miner::isRunning() {
	return minerProcess.isOpen() || startMinerTimer.isActive();
}

void Miner::checkIfItHasStarted() {
	if(minerProcess.isOpen()) {
		startMinerTimer.stop();
		emit started();
	}
}

void Miner::checkIfItHasStopped() {
	if(!minerProcess.isOpen()) {
		stopMinerTimer.stop();
		emit stopped();
	}
}

void Miner::minerStateChanged(QProcess::ProcessState state) {
	qDebug() << "Miner" << name << " State Changed" << state;
	switch(state) {
		default:
		return;

		case QProcess::NotRunning:
			emit stopped();
		break;

		case QProcess::Running:
			emit started();
		break;
	}
}
