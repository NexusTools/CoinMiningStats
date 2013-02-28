#ifndef GRAPH_H
#define GRAPH_H

#include <QVariantMap>
#include <QDialog>
#include <QPixmap>

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

private:
    QPixmap buffer;
};

#endif // GRAPH_H
