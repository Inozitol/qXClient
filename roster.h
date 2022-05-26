#ifndef ROSTER_H
#define ROSTER_H

#include <QByteArray>
#include <QtXml>

#include "addressable.h"

struct rosteritem_t{
    jid_t jid;
    bool approved;
    QByteArray ask;
    QByteArray name;
    QByteArray subscription;
    QVector<QByteArray> groups;

    rosteritem_t(QDomElement roster_root){
        if(roster_root.nodeName() != "item"){
            throw std::invalid_argument("rosteritem_it expects DOM element of item");
        }
        approved =  roster_root.attribute("approved") == "true";
        ask =       roster_root.attribute("ask").toUtf8();
        name =      roster_root.attribute("name").toUtf8();
        jid =       roster_root.attribute("jid").toUtf8();
        if(roster_root.hasAttribute("subscription")){
            subscription = roster_root.attribute("subscription").toUtf8();
        }else{
            subscription = "none";
        }
        if(roster_root.hasChildNodes()){
            QDomNodeList groups_nodes = roster_root.childNodes();
            for(int i=0; i<groups_nodes.length(); i++){
                QDomNode group = groups_nodes.at(i);
                groups.append(group.toElement().text().toUtf8());
            }
        }
    }
    rosteritem_t() = default;
};

#endif // ROSTER_H
