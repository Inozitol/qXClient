#ifndef ADDRESSABLE_H
#define ADDRESSABLE_H

#include <QString>
#include <QDataStream>
#include <QDebug>

struct jid_t{
    QString domain;
    QString local;
    QString resource;

    friend QDataStream& operator<<(QDataStream& os, const jid_t& jid){
        QString str;
        str << jid;
        os << str;
        return os;
    }


    friend QString& operator<<(QString& ostr, const jid_t& jid){
        if(!jid.local.isEmpty()){
            ostr.append(jid.local + '@');
        }
        ostr.append(jid.domain);
        if(!jid.resource.isEmpty()){
            ostr.append('/' + jid.resource);
        }
        return ostr;
    }

    friend QDebug& operator<<(QDebug& debug, const jid_t& jid){
        QDebugStateSaver saver(debug);
        QString str;
        str << jid;
        debug.nospace() << str;
        return debug;
    }

    jid_t(const QByteArray& in){
        auto f_split = in.split('@');
        if(f_split.length() == 2){
            local = f_split.at(0);
            auto s_split = f_split.at(1).split('/');
            domain = s_split.at(0);
            if(s_split.length() == 2){
                resource = s_split.at(1);
            }
        }else{
            auto s_split = f_split.at(0).split('/');
            domain = s_split.at(0);
            if(s_split.length() == 2){
                resource = s_split.at(1);
            }
        }
    }

    jid_t(QByteArray&& in){
        auto f_split = in.split('@');
        if(f_split.length() == 2){
            local = f_split.at(0);
            auto s_split = f_split.at(1).split('/');
            domain = s_split.at(0);
            if(s_split.length() == 2){
                resource = s_split.at(1);
            }
        }else{
            auto s_split = f_split.at(0).split('/');
            domain = s_split.at(0);
            if(s_split.length() == 2){
                resource = s_split.at(1);
            }
        }
    }

    jid_t(QString domain_, QString local_, QString resource_){
        domain = domain_;
        local = local_;
        resource = resource_;
    }

    QString str(){
        QString str;
        str << *this;
        return str;
    }
};

class Addressable
{
public:
    Addressable(QString domain, QString local="", QString resource="");
    void setJid(const jid_t& jid);
    jid_t jid();

protected:
    jid_t _jid;
};

#endif // ADDRESSABLE_H
