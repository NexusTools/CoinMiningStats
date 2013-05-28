#include "loosejson.h"
#include <QBuffer>
#include <QDebug>

QList<QByteArray> errorList;
const QRegExp validString("^[\\d\\w\\-_!@#\\$%\\^&\\*\\(\\)\\.]+$", Qt::CaseInsensitive, QRegExp::RegExp);
const QRegExp validNumber("^\\d+(\\.\\d+)?$", Qt::CaseInsensitive, QRegExp::RegExp2);

QVariant _parse(QIODevice* device, bool startup=true)
{
    if(startup) {
        errorList.clear();
        QVariant data;
        try {
            data = _parse(device, false);
        } catch(const char* error) {
            errorList << error;
            data = QByteArray("Parse Error: ") + error;
        } catch(QByteArray error) {
            errorList << error;
            data = QByteArray("Parse Error: Unexpected ") + error;
        }
        if(errorList.size())
            qWarning() << errorList;
        return data;
    }

    if(device->atEnd())
        throw "End of Stream";

    bool quoted = false;
    char dat;
    QByteArray varDat;
    while(device->read(&dat, 1)) {
        QChar c(dat);
        if(c.isSpace()) {
            if(varDat.isEmpty())
                continue;
            break;
        }

        bool breakLoop = false;
        switch(dat) {
        case '[':
        {
            if(!varDat.isEmpty()) {
                device->seek(device->pos()-1);
                breakLoop = true;
                break;
            }
            QVariantList list;
            while(true)
                try {
                    list.append(_parse(device, false));
                } catch(QByteArray) {} // Skip

            return list;
        }
            break;

        case '{':
        {
            if(!varDat.isEmpty()) {
                device->seek(device->pos()-1);
                breakLoop = true;
                break;
            }
            QVariantMap map;
            try {
                while(true) {
                    try {
                        QString key = _parse(device, false).toString();
                        QVariant val;
                        try {
                            val = _parse(device, false);
                        } catch(QByteArray error) {
                            val = error;
                        } catch(const char* error) {
                            map.insert(key, QByteArray("Unexpected ") + error);
                            throw error;
                        } catch(...) {
                            val = QByteArray("Error");
                        }

                        map.insert(key, val);
                    } catch(QByteArray) {}
                }
            } catch(const char* error) {}
            return map;
        }
            break;

        case '}':
        case ']':
            if(!varDat.isEmpty()) {
                device->seek(device->pos()-1);
                breakLoop = true;
                break;
            }
            throw "End of Loop";

        case ',':
        case '=':
        case ':':
        case '(':
        case ')':
            if(!varDat.isEmpty())
                breakLoop = true;
            break;

        case '"':
        case '\'':
        {
            quoted = true;
            char nDat;
            while(device->read(&nDat, 1)) {
                if(nDat == c)
                    break;
                if(nDat == '\\') { //TODO: Better Escape Parsing
                    if(device->read(&nDat, 1))
                        varDat.append(nDat);

                    continue;
                } else
                    varDat.append(nDat);
            }
            breakLoop = true;
        }
            break;

        default:
            varDat.append(dat);
            break;
        }
        if(breakLoop)
            break;
    }

    if(varDat.isEmpty())
        return QVariant();

    bool ok;
    double number = varDat.toDouble(&ok);
    if(ok)
        return QVariant(number);

    if(validString.exactMatch(varDat)) {
        if(!quoted) {
            if(varDat == "true")
                return QVariant(true);
            else if(varDat == "false")
                return QVariant(false);
        }

        return QVariant(QString(varDat));
    }

    return QVariant(varDat);
}

QVariant LooseJSON::parse(QIODevice *io)
{
    return _parse(io);
}

QVariant LooseJSON::parse(QByteArray data)
{
    QBuffer buffer;
    buffer.setBuffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QVariant dat = _parse(&buffer);
    return dat;
}
