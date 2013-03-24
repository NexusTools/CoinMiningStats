#ifndef LOOSEJSON_H
#define LOOSEJSON_H

#include <QIODevice>
#include <QVariant>
#include <QRegExp>

class LooseJSON
{
public:
    enum EncodeStrictness {
        VeryLoose,
        VeryStrict
    };

    static QVariant parse(QIODevice* iodevice);
    static QVariant parse(QByteArray data); // Parse UTF8 JSON String
    static QByteArray encode(QVariant data, EncodeStrictness =VeryLoose);

};

#endif // LOOSEJSON_H
