#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QRandomGenerator64>
#include <QChar>
#include <QtXml>

namespace Utils{
    QByteArray randomString(int len);
    QByteArray getXOR(const QByteArray& arr1, const QByteArray& arr2);
    QByteArray getXOR(const QString& str1, const QString& str2);
    void reader2node(QDomDocument& doc, QDomNode& node, QXmlStreamReader& reader);
    QByteArray simpleEncryption(const QByteArray& data, const QByteArray& key);
}


#endif // UTILS_H
