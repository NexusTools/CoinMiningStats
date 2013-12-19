#ifndef POOLAPI_H
#define POOLAPI_H
#include <QObject>
#include <QTimer>
#include <QNetworkReply>

class PoolAPI : public QObject
{
		Q_OBJECT
	public:
		PoolAPI(QObject *parent = 0);
		void init(int apiHost, QString apiKey);

	private:
		QTimer apiTimer;
		int apiHost;
		QString apiKey;

		QNetworkReply* apiDataRequester;

	private slots:
		void requestAPIData();
		void apiDataReply();

	signals:
		void apiDataReceived(QVariantMap);
};

#endif // POOLAPI_H
