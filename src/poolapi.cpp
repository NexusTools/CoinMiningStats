#include "poolapi.h"
#include "mainwindow.h"
#include "loosejson.h"
#include <QDebug>

PoolAPI::PoolAPI(QObject *parent) :
		QObject(parent) {
	apiTimer.setInterval(30000);

	connect(&apiTimer, SIGNAL(timeout()), this, SLOT(requestAPIData()));
}

void PoolAPI::init(int apiHost, QString apiKey) {
	this->apiHost = apiHost;
	this->apiKey = apiKey;
	apiTimer.start();
	requestAPIData();
}

void PoolAPI::requestAPIData() {
	if(apiKey.trimmed().isEmpty())
		return;
	qDebug() << "Requesting API Data";
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

void PoolAPI::apiDataReply() {
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

	disconnect(apiDataRequester, SIGNAL(finished()), this, SLOT(apiDataReply())); //< Redundancy?
	apiDataRequester->deleteLater();
}
