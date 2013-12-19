#ifndef MINER_H
#define MINER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QNetworkReply>
#include <QFile>
#include <QTextStream>

class Miner : public QObject
{
		Q_OBJECT
	public:
		explicit Miner(QObject *parent = 0);
		QString name;
		QProcess minerProcess;

		bool isRunning();
		void init(QString name, QString applicationPath, QStringList applicationArguments);
		void start();
		void stop();

	private:
		QString applicationPath;
		QStringList applicationArguments;

		QTimer startMinerTimer;
		QTimer stopMinerTimer;

		QFile logFile;
		QTextStream logStream;

	signals:
		void updatedStats();

		void stopped();
		void started();

		void apiDataReceived(QVariantMap data);
	public slots:
		void passStdOut();
		void passStdErr();

		void minerStateChanged(QProcess::ProcessState state);

		void checkIfItHasStopped();
		void checkIfItHasStarted();

};

#endif // MINER_H
