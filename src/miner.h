#ifndef MINER_H
#define MINER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QNetworkReply>

class Miner : public QObject
{
		Q_OBJECT
	public:
		explicit Miner(QObject *parent = 0);
		QString name;
		QProcess minerProcess;

		bool isRunning();
		void start();
		void start(QString name, QString applicationPath, QStringList applicationArguments, int apiHost, QString apiKey, QString apiSecert);
		void stop();

	private:
		QTimer apiTimer;
		QString applicationPath;
		QStringList applicationArguments;
		int apiHost;
		QString apiKey;
		QString apiSecert;

		QTimer startMinerTimer;
		QTimer stopMinerTimer;

		QNetworkReply* apiDataRequester;

	signals:
		void updatedStats();

		void stopped();
		void started();

		void apiDataReceived(QVariantMap data);
	public slots:
		void requestAPIData();
		void apiDataReply();

		void passStdOut();
		void passStdErr();

		void minerStateChanged(QProcess::ProcessState state);

		void checkIfItHasStopped();
		void checkIfItHasStarted();

};

#endif // MINER_H
