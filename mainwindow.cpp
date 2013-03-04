#include "mainwindow.h"
#include "graph.h"

#include <QScriptEngine>
#include <QInputDialog>
#include <QResource>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    settings()
{
    setupUi(this);

    confirmed->setMode(ColorIndicatorLabel::BitCoins);
    unconfirmed->setMode(ColorIndicatorLabel::BitCoins);
    estimated->setMode(ColorIndicatorLabel::BitCoins);
    potential->setMode(ColorIndicatorLabel::BitCoins);
    workers_rate->setMode(ColorIndicatorLabel::HashRate);

    updateAccountDataTimer.setSingleShot(true);
    updateAccountDataTimer.setInterval(15000);
    connect(&updateAccountDataTimer, SIGNAL(timeout()), this, SLOT(requestAccountDataUpdate()));

    updatePoolStatsTimer.setSingleShot(true);
    updatePoolStatsTimer.setInterval(60000 * 10);
    connect(&updatePoolStatsTimer, SIGNAL(timeout()), this, SLOT(requestPoolStatsUpdate()));

    updateBlockInfoTimer.setSingleShot(true);
    updateBlockInfoTimer.setInterval(60000 * 4);
    connect(&updateBlockInfoTimer, SIGNAL(timeout()), this, SLOT(requestBlockInfoUpdate()));

    accountDataRequest = 0;
    poolStatsRequest = 0;

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

    qDebug() << "Using API Key" << apiKey;
    connect(graphBtn, SIGNAL(clicked()), this, SLOT(showGraph()));
    connect(tglWidget, SIGNAL(toggled(bool)), this, SLOT(toggleWidget(bool)));

    workers->resizeColumnsToContents();
    graph = 0;

    requestAccountDataUpdate();
    requestPoolStatsUpdate();
    requestBlockInfoUpdate();
}

void MainWindow::toggleWidget(bool checked)
{
    static QRect oldGeometry;

    hide();
    Qt::WindowFlags flags = this->windowFlags();
    if (checked)
    {
        oldGeometry = geometry();
        setStyleSheet(QString((const char*)QResource(":/widget.css").data()));
        setWindowFlags(flags | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        setWindowOpacity(0.8);
        workers->setVisible(false);

        layout()->setSizeConstraint(QLayout::SetFixedSize);
        resize(width(), layout()->minimumHeightForWidth(width()));
    }
    else
    {
        setStyleSheet("");
        setWindowFlags(flags ^ (Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint));
        setWindowOpacity(1);
        workers->setVisible(true);
        layout()->setSizeConstraint(QLayout::SetDefaultConstraint);
        setGeometry(oldGeometry);
    }

    emit invertChanged(checked);
    //confirmations_left->setInverted();

    QTimer::singleShot(50, this, SLOT(show()));
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

void MainWindow::requestAccountDataUpdate()
{
    updateAccountDataTimer.stop();
    qDebug() << "Requesting Account Data Update";
    if(!accountDataRequest)
        accountDataRequest->deleteLater();
    accountDataRequest = accessMan.get(QNetworkRequest(QUrl(QString("https://mining.bitcoin.cz/accounts/profile/json/%1").arg(apiKey))));
    connect(accountDataRequest, SIGNAL(finished()), this, SLOT(accountDataReply()));
}

void MainWindow::requestBlockInfoUpdate()
{
    qDebug() << "Requesting Pool Statistics Update";
    if(!blockInfoRequest)
        blockInfoRequest->deleteLater();
    blockInfoRequest = accessMan.get(QNetworkRequest(QUrl("http://blockchain.info/latestblock")));
    connect(blockInfoRequest, SIGNAL(finished()), this, SLOT(blockInfoReply()));
}

void MainWindow::requestPoolStatsUpdate()
{
    qDebug() << "Requesting Pool Statistics Update";
    if(!poolStatsRequest)
        poolStatsRequest->deleteLater();
    poolStatsRequest = accessMan.get(QNetworkRequest(QUrl(QString("https://mining.bitcoin.cz/stats/json/%1").arg(apiKey))));
    connect(poolStatsRequest, SIGNAL(finished()), this, SLOT(poolStatsReply()));
}

void MainWindow::accountDataReply()
{
    updateAccountDataTimer.start();
    accountDataRequest->deleteLater();
    if(accountDataRequest->error()) {
        qWarning() << "Request Failed" << accountDataRequest->errorString();

        workers->horizontalHeader()->setVisible(false);
        workers->clearContents();
        workers->insertRow(0);
        workers->setItem(0, 0, new QTableWidgetItem(QString("Connection Issue: %1").arg(accountDataRequest->errorString())));
        workers->resizeColumnsToContents();

        accountDataRequest = 0;
        return;
    }

    qreal totalRate = 0;
    QVariant data;
    {
        QScriptEngine engine;
        data = engine.evaluate("(" + accountDataRequest->readAll() + ")").toVariant();
    }

    QVariantMap map = data.toMap();
    if(!map.isEmpty()) {
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

                totalRate += workerMap.value("hashrate").toFloat();

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

        qreal cw = map.value("confirmed_reward").toReal();
        qreal uw = map.value("unconfirmed_reward").toReal();
        workers->horizontalHeader()->setVisible(true);
        // Set Labels
        workers_rate->setValue(totalRate);
        confirmed->setValue(cw);
        unconfirmed->setValue(uw);
        estimated->setValue(map.value("estimated_reward").toReal());
        potential->setValue(cw + uw);
    } else {
        qWarning() << "Bad Reply";
    }
    accountDataRequest = 0;
}

void MainWindow::poolStatsReply()
{
    updatePoolStatsTimer.start();
    poolStatsRequest->deleteLater();
    if(poolStatsRequest->error()) {
        qWarning() << "Pool Statistics Request Failed" << poolStatsRequest->errorString();

        poolStatsRequest = 0;
        return;
    }

    QVariant data;
    {
        QScriptEngine engine;
        data = engine.evaluate("(" + poolStatsRequest->readAll() + ")").toVariant();
    }

    QVariantMap map = data.toMap();
    if(!map.isEmpty()) {
        qreal reward = 0;
        emit receivedPoolStatsData(map);
        int leastConfirmations = -1;
        int unconfirmedblocks = 0;
        if(map.contains("blocks")) {
            QVariantMap blocksMap = map.value("blocks").toMap();
            QVariantMap::iterator i;
            for (i = blocksMap.begin(); i != blocksMap.end(); ++i) {
                QVariantMap blockMap = i.value().toMap();
                if(blockMap.value("reward").toReal() > 0) {
                    int confirmationsLeft = 100 - blockMap.value("confirmations").toInt();
                    if(confirmationsLeft > 0) {
                        if((leastConfirmations == -1 || confirmationsLeft < leastConfirmations)) {
                            leastConfirmations = confirmationsLeft;
                            reward = blockMap.value("reward").toReal();
                        }
                        unconfirmedblocks++;
                    }
                }
            }
        }

        unconfirmed_blocks->setValue(unconfirmedblocks);
        next_reward->setValue(reward);
        confirmations_left->setValue(leastConfirmations);
    } else {
        qWarning() << "Bad Reply";
    }
    poolStatsRequest = 0;
}

void MainWindow::blockInfoReply()
{
    updateBlockInfoTimer.start();
    blockInfoRequest->deleteLater();
    if(blockInfoRequest->error()) {
        qWarning() << "Pool Statistics Request Failed" << poolStatsRequest->errorString();

        blockInfoRequest = 0;
        return;
    }

    QVariant data;
    {
        QScriptEngine engine;
        data = engine.evaluate("(" + blockInfoRequest->readAll() + ")").toVariant();
    }

    QVariantMap map = data.toMap();
    if(!map.isEmpty()) {
        qreal reward = 0;
        emit receivedBlockInfoData(map);

        if(map.contains("height"))
            blockchain_height->setValue(map.value("height").toULongLong());
    } else {
        qWarning() << "Bad Reply";
    }
    blockInfoRequest = 0;
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
