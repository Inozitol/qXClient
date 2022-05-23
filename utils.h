#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QRandomGenerator64>
#include <QChar>

namespace Utils{
    QByteArray getRandomString(int len);
    QByteArray getXOR(const QByteArray& arr1, const QByteArray& arr2);
}


#endif // UTILS_H
