#include "graph.h"

#include <QPainter>

Graph::Graph(QWidget *parent) :
    QDialog(parent)
{
    setMinimumSize(400, 300);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Graph");
    show();
}

void Graph::resizeEvent(QResizeEvent *)
{
    // Create and clear the buffer
    buffer = QPixmap(size());
    buffer.fill(Qt::black);
}

void Graph::paintEvent(QPaintEvent *)
{
    if(buffer.isNull())
        return; // No Buffer Available;

    QPainter p;
    //p.begin(&buffer);
    // Draw Buffer Information
    //p.end();

    p.begin(this);
    p.drawPixmap(0, 0, buffer);
    p.end();
}

void Graph::receivedPoolStatsData(QVariantMap data){

}

void Graph::receivedAccountData(QVariantMap data){
}
