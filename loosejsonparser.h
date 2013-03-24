#ifndef LOOSEJSONPARSER_H
#define LOOSEJSONPARSER_H

#include <QObject>

class LooseJSONParser : public QObject
{
    Q_OBJECT
public:
    explicit LooseJSONParser(QObject *parent = 0);
    
public slots:
    QVariant parse(QByteArray data);
    inline QVariant parse(QString data) const{return parse(data.toUtf8());}
    
};

#endif // LOOSEJSONPARSER_H
