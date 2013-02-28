#include "graph.h"

#include <QPainter>

Graph::Graph(QWidget *parent) :
    QDialog(parent)
{
    setMinimumSize(400, 300);
    show();

    setWindowTitle("Statistics Graph");
    /*
     * I recommend you use a QTimer to make a tick, and draw on the buffer outside of the paint event.
     * Than just call repaint() to flip the buffer onscreen
     * That way you can draw the new line updates from one of the data events
     * And use the timer to push the buffer to the left constantly
     */
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

void Graph::receivedPoolData(QVariantMap data){

}

void Graph::receivedAccountData(QVariantMap data){
}
