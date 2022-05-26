#ifndef ADDRESSABLE_H
#define ADDRESSABLE_H

#include <QString>
#include <QDataStream>
#include <QDebug>

struct jid_t{
    QByteArray domain;
    QByteArray local;
    QByteArray resource;

    friend QDataStream& operator<<(QDataStream& os, const jid_t& jid){
        QByteArray arr;
        arr << jid;
        os << arr;
        return os;
    }

    friend QByteArray& operator<<(QByteArray& ostr, const jid_t& jid){
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
        QByteArray arr;
        arr << jid;
        debug.nospace() << arr;
        return debug;
    }

    friend bool operator==(const jid_t& l, const jid_t& r){
        if(l.domain != r.domain)        return false;
        if(l.local != r.local)          return false;
        if(l.resource != r.resource)    return false;
        return true;
    }

    friend bool operator<(const jid_t& l, const jid_t& r){
        QByteArray l_str = l.domain+l.local+l.resource;
        QByteArray r_str = r.domain+r.local+r.resource;
        return l_str < r_str;
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

    jid_t(const QByteArray& domain_, const QByteArray& local_, const QByteArray& resource_){
        domain = domain_;
        local = local_;
        resource = resource_;
    }

    jid_t()=default;

    QByteArray str(){
        QByteArray arr;
        arr << *this;
        return arr;
    }

    jid_t bare() const{
        jid_t bareJid;
        bareJid.domain = domain;
        bareJid.local = local;
        return bareJid;
    }
};

class Addressable
{
public:
    Addressable(const QByteArray& domain,
                const QByteArray& local="",
                const QByteArray& resource="");
    void setJid(const jid_t& jid);
    jid_t jid();

protected:
    jid_t _jid;
};

#endif // ADDRESSABLE_H
