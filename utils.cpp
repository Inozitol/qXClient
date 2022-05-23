#include "utils.h"

namespace Utils{
    QByteArray getRandomString(int len){
        QByteArray result;
        for(; len>0; --len){
            char rando_char;
            do{
                rando_char = QRandomGenerator64::global()->bounded(33,126);
            }while(rando_char == ',');
            result.append(rando_char);
        }
        return result;
    }

    QByteArray getXOR(const QByteArray& arr1, const QByteArray& arr2){
        qsizetype len1 = arr1.length();
        qsizetype len2 = arr2.length();
        qsizetype iter = std::max({len1, len2});
        QByteArray result;
        for(int i = 0; i < iter; i++){
            char val = arr1.at(i%len1) ^ arr2.at(i%len2);
            result.append(val);
        }
        return result;
    }

}
