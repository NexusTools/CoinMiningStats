#include "loosejson.h"
#include <QBuffer>
#include <QDebug>

QList<QByteArray> errorList;
const QRegExp validString("^[\d\w\-_!@#\$%\^&\*\(\)\.]+$", Qt::CaseInsensitive, QRegExp::RegExp);
const QRegExp validNumber("^\d+(\.\d+)?$", Qt::CaseInsensitive, QRegExp::RegExp2);

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
            data = QByteArray("Parse Error: ") + error;
        }
        if(errorList.size())
            qDebug() << errorList;
        return data;
    }

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
            try {
                while(true)
                    list.append(_parse(device, false));
            } catch(const char*) {
                // Skip These
            } catch(QByteArray error) {
                errorList << error;
            }

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
                    QString key = _parse(device, false).toString();
                    map.insert(key, _parse(device, false));
                }
            } catch(const char*) {
                // Skip These
            } catch(QByteArray error) {
                errorList << error;
            }

            return map;
        }
            break;

        case '}':
        case ']':
            throw "Unexpected End Bracket";

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
                    else
                        throw QByteArray("Unexpected End of Stream during Escaped Character");

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
        throw (QByteArray("Unexpected ") + dat + ". Expected value, array or map.");


    //if(validNumber.exactMatch(varDat)) {
        bool ok;
        double number = varDat.toDouble(&ok);
        if(ok)
            return QVariant(number);
    //}

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

QVariant LooseJSON::parse(QByteArray data)
{

    QBuffer buffer;
    buffer.setBuffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QVariant retData = _parse(&buffer);
    //();
    return retData;
}
