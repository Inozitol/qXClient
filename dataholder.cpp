#include "dataholder.h"

DataHolder::DataHolder(){
    m_umapIconReference.emplace("online",   QIcon(":usermeta/online.svg"));
    m_umapIconReference.emplace("offline",  QIcon(":usermeta/offline.svg"));
    m_umapIconReference.emplace("expand",   QIcon(":usermeta/expand.svg"));
    m_umapIconReference.emplace("collapse", QIcon(":usermeta/collapse.svg"));
}

const QIcon& DataHolder::getIcon(const QString &name){
    if(m_umapIconReference.count(name)){
        return m_umapIconReference.at(name);
    }
    return m_defIcon;
}

const QImage& DataHolder::getAvatar(const QString& id){
    if(m_umapAvatarReference.count(id)){
        return m_umapAvatarReference.at(id);
    }
    return m_nullImg;
}

void DataHolder::insertAvatar(const QImage &img, const QString& id){
    m_umapAvatarReference.emplace(id, img);
}

DataHolder& DataHolder::instance(){
    static DataHolder instance;
    return instance;
}
