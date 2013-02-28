#include "graph.h"

#include <QPainter>


QPen green; // I might be an idiot
QPen blackpen;


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

    connect(&ticks, SIGNAL(timeout()), this, SLOT(tick()));
}

int w=400, h=300;

void Graph::resizeEvent(QResizeEvent *)
{
    // Create and clear the buffer
    w = size().width();
    h = size().height();
    buffer = QPixmap(size());
    buffer.fill(Qt::black);
}

int lasth = 150;
int rlasth = 150;

void Graph::tick(){
    if(buffer.isNull())
        return; // No Buffer Available;

    QPainter p;
    p.begin(&buffer);
     p.drawPixmap(-1, 0, buffer);
     p.setPen(blackpen);
     p.drawLine(w-1, 0, w-1, h);
     p.setPen(green);
     p.drawLine(w-1, rlasth, w-1, lasth+1);
     p.drawLine(w-1, lasth, w-1, h);
    p.end();

    rlasth = lasth;
    repaint();
}

void Graph::paintEvent(QPaintEvent *)
{
    if(buffer.isNull())
        return; // No Buffer Available;

    QPainter p;
    p.begin(this);
    p.drawPixmap(0, 0, buffer);
    p.end();
}

void Graph::receivedPoolStatsData(QVariantMap data){

}

float totalrate = 0;

float unconfirmed = 0;
float confirmed = 0;

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
    lasth = h-totalrate;
    this->repaint();
}
