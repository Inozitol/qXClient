#include "utils.h"

namespace Utils{
QByteArray randomString(int len){
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
        char val = arr1.at(i % len1) ^ arr2.at(i % len2);
        result.append(val);
    }
    return result;
}

QByteArray getXOR(const QString& str1, const QString& str2){
    return getXOR(str1.toUtf8(), str2.toUtf8());
}

void reader2node(QDomDocument& doc, QDomNode& node, QXmlStreamReader& reader){
    if(reader.tokenType() != QXmlStreamReader::StartElement){
        throw std::invalid_argument("reader2node expects reader on element start");
    }

    QXmlStreamReader::TokenType token = reader.tokenType();
    QString name = reader.name().toString();
    QString ending_str = node.nodeName();
    QDomElement currNode = node.toElement();

    while(!(name == ending_str && token == QXmlStreamReader::EndElement) && token != QXmlStreamReader::Invalid){
        token = reader.readNext();
        name = reader.name().toUtf8();
        switch(token){
        case QXmlStreamReader::StartElement:
        {
            QDomElement newNode;
            if(reader.namespaceUri().isEmpty()){
                newNode = doc.createElement(name);
            }else{
                newNode = doc.createElementNS(reader.namespaceUri().toString(), name);
            }
            for(const auto& attr : reader.attributes()){
                newNode.setAttribute(attr.name().toString(), attr.value().toString());
            }
            currNode = currNode.appendChild(newNode).toElement();
        }
            break;
        case QXmlStreamReader::EndElement:
            currNode = currNode.parentNode().toElement();
            break;
        case QXmlStreamReader::Characters:
        {
            QDomText textNode = doc.createTextNode(reader.text().toString());
            currNode = currNode.appendChild(textNode).toElement();
        }
            break;
        default:
            break;
        }
    }
}

QByteArray simpleEncryption(const QByteArray& data, const QByteArray& key){
    QByteArray output;
    qsizetype keyLen = key.length();
    for(int i=0; i<data.length(); i++){
        output.append(data.at(i) ^ key.at(i % keyLen));
    }
    return output;
}
}
