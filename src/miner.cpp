#include "miner.h"
#include <QDebug>
#include <QDir>
#include "mainwindow.h"
#include "loosejson.h"

Miner::Miner(QObject *parent) :
			 QObject(parent),
			 logStream(&logFile) {

	connect(&minerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(minerStateChanged(QProcess::ProcessState)));

	startMinerTimer.setInterval(100);
	stopMinerTimer.setInterval(100);
	connect(&minerProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(passStdOut()));
	connect(&minerProcess, SIGNAL(readyReadStandardError()), this, SLOT(passStdErr()));

	connect(&startMinerTimer, SIGNAL(timeout()), this, SLOT(checkIfItHasStarted()));
	connect(&stopMinerTimer, SIGNAL(timeout()), this, SLOT(checkIfItHasStopped()));

}

void Miner::passStdOut() {
	QString stdData = minerProcess.readAllStandardOutput().data();
	qDebug() << stdData;
	logStream << stdData << endl;

}

void Miner::passStdErr() {
	QString errData = minerProcess.readAllStandardError().data();
	qDebug() << errData;
	logStream << errData << endl;
}

void Miner::init(QString name, QString applicationPath, QStringList applicationArguments) {
	stop();
	this->name = name;
	this->applicationPath = applicationPath;
	this->applicationArguments = applicationArguments;

	if(logFile.isOpen()) {
		logStream.flush();
		logFile.close();
	}
	logFile.setFileName(QDir::homePath().append(QDir::separator()).append("CoinMiningStats - ").append(name).append(".log"));
}

void Miner::start() {
	stop();
	if(applicationPath.isEmpty()) {
		emit stopped();
		return;
	}

	logFile.open(QFile::WriteOnly | QFile::Text | QFile::Append);

	startMinerTimer.start();
	minerProcess.start(applicationPath, applicationArguments);

	logStream << endl << endl << "Started at " << QDateTime::currentDateTime().toString() << endl;
}

void Miner::stop() {
	if(!minerProcess.isOpen())
		return;
	stopMinerTimer.start();
	minerProcess.close();
	minerProcess.waitForFinished(3000);
	if(minerProcess.state() != QProcess::NotRunning) {
		qWarning() << "Force killing" << name;
		logStream << "Force killing!" << endl;
		minerProcess.kill();
	}
	logStream << "Stopped." << endl;
	if(logFile.isOpen()) {
		logStream.flush();
		logFile.close();
	}
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
	logStream << "State Changed to " << state << endl;
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
