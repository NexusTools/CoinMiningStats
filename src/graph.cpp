#include "graph.h"

#include <QPainter>

Graph::Graph(QWidget *parent) :
    QDialog(parent)
{
    setMinimumSize(400, 300);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Graph");

    green.setWidth(1);
    green.setColor(QColor(0,250,0,100));
    blackpen.setWidth(1);
    blackpen.setColor(QColor(0,0,0));
    show();

    ticks.setInterval(200);
    ticks.start();

    lasth = 150;
    rlasth = 150;
    totalrate = 0;
    offset = 0;

    connect(&ticks, SIGNAL(timeout()), this, SLOT(tick()));
}

void Graph::resizeEvent(QResizeEvent *)
{
    // Create and clear the buffer
    buffer = QPixmap(size());
    buffer.fill(Qt::black);

    offset = 0;
}

void Graph::tick(){
    if(buffer.isNull())
        return; // No Buffer Available;

    QPainter p(&buffer);
    p.setPen(blackpen);
    p.drawLine(offset, 0, offset, height());
    p.setPen(green);
    p.drawLine(offset, rlasth, offset, lasth+1);
    p.drawLine(offset, lasth, offset, height());
    p.end();

    offset++;
    if(offset >= width())
        offset = 0;

    rlasth = lasth;
    repaint();
}

void Graph::paintEvent(QPaintEvent *)
{
    if(buffer.isNull())
        return; // No Buffer Available;

    QPainter p(this);
    p.drawPixmap(width() - (offset+1), 0, buffer);
    p.drawPixmap(-(offset+1), 0, buffer);
    p.end();
}

void Graph::receivedPoolStatsData(QVariantMap){}

void Graph::receivedAccountData(QVariantMap data){
    if(data.contains("workers")){
        totalrate = 0;
        QVariantMap workersMap = data.value("workers").toMap();
        QVariantMap::iterator i;
        for (i = workersMap.begin(); i != workersMap.end(); ++i) {
            QString workerName = i.key();
            QVariantMap workerMap = i.value().toMap();

            totalrate += workerMap.value("hashrate").toFloat();

        }
    }
    lasth = height()-totalrate;
    this->repaint();
}
