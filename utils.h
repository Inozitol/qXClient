#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QRandomGenerator64>
#include <QChar>
#include <QtXml>

namespace Utils{
    QByteArray getRandomString(int len);
    QByteArray getXOR(const QByteArray& arr1, const QByteArray& arr2);
    void reader2node(QDomDocument& doc, QDomNode& node, QXmlStreamReader& reader);
}


#endif // UTILS_H
