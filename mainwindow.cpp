#include "mainwindow.h"
#include "graph.h"

#include <QScriptEngine>
#include <QInputDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    settings()
{
    setupUi(this);
    updateTimer.setSingleShot(true);
    updateTimer.setInterval(5000);
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(requestUpdate()));
    networkReply = 0;

    apiKey = settings.value("slush_api").toString();

    if(apiKey.isNull()) {
        QInputDialog inputDiag(this);
        inputDiag.setInputMode(QInputDialog::TextInput);
        inputDiag.setLabelText("Slush's Pool API Token");
        inputDiag.exec();
        apiKey = inputDiag.textValue();

        if(apiKey.isNull())
            exit(0);

        settings.setValue("slush_api", apiKey);
    }

    connect(graphBtn, SIGNAL(clicked()), this, SLOT(showGraph()));

    workers->resizeColumnsToContents();
    graph = 0;

    requestUpdate();

}

void MainWindow::graphDestroyed()
{
    graph = 0;
}

void MainWindow::showGraph(){
    if(graph)
        return;

    graph = new Graph(this);
    connect(graph, SIGNAL(destroyed()), this, SLOT(graphDestroyed()));
    connect(this, SIGNAL(receivedAccountData(QVariantMap)), graph, SLOT(receivedAccountData(QVariantMap)));
}

void MainWindow::requestUpdate()
{
    qDebug() << "Requesting Update";
    if(!networkReply)
        networkReply->deleteLater();
    networkReply = accessMan.get(QNetworkRequest(QUrl(QString("https://mining.bitcoin.cz/accounts/profile/json/%1").arg(apiKey))));
    connect(networkReply, SIGNAL(finished()), this, SLOT(gotReply()));
}

void MainWindow::gotReply()
{
    updateTimer.start();
    networkReply->deleteLater();
    if(networkReply->error()) {
        qWarning() << "Request Failed" << networkReply->errorString();

        workers->horizontalHeader()->setVisible(false);
        workers->clearContents();
        workers->insertRow(0);
        workers->setItem(0, 0, new QTableWidgetItem(QString("Connection Issue: %1").arg(networkReply->errorString())));
        workers->resizeColumnsToContents();

        networkReply = 0;
        return;
    }

    QVariant data;
    {
        QScriptEngine engine;
        data = engine.evaluate("(" + networkReply->readAll() + ")").toVariant();
    }
    if(data.type() == QVariant::Map){
        QVariantMap map = data.toMap();
        qDebug() << map.keys();
        QStringList knownWorkers;

        emit receivedAccountData(map);

        // Process Workers
        if(map.contains("workers")) {
            QVariantMap workersMap = map.value("workers").toMap();
            QVariantMap::iterator i;
            for (i = workersMap.begin(); i != workersMap.end(); ++i) {
                QString workerName = i.key();
                QVariantMap workerMap = i.value().toMap();
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

                workers->setItem(row, 1, new QTableWidgetItem(QString("%1MH/s").arg(workerMap.value("hashrate").toString())));
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

        workers->horizontalHeader()->setVisible(true);
        // Set Labels
        confirmed->setText(map.value("confirmed_reward").toString());
        unconfirmed->setText(map.value("unconfirmed_reward").toString());
        estimated->setText(map.value("estimated_reward").toString());
        potential->setText(QString("%1").arg(map.value("confirmed_reward").toFloat() + map.value("unconfirmed_reward").toFloat()));
    } else {
        qWarning() << "Bad Reply";
    }
    networkReply = 0;
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
