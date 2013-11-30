#ifndef GRAPH_H
#define GRAPH_H

#include <QVariantMap>
#include <QDialog>
#include <QPixmap>
#include <QTimer>
#include <QPen>

class Graph : public QDialog
{
    Q_OBJECT
public:
    explicit Graph(QWidget *parent = 0);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    
public slots:
    void receivedPoolStatsData(QVariantMap data);
    void receivedAccountData(QVariantMap data);

private slots:
    void tick();

private:
    QPixmap buffer;
    QTimer ticks;

    QPen green; // I might be an idiot
    QPen blackpen;

    int offset;

    int lasth;
    int rlasth;
    float totalrate;
};

#endif // GRAPH_H
