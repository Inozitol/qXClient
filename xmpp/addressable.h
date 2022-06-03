#ifndef ADDRESSABLE_H
#define ADDRESSABLE_H

#include <QString>
#include <QDataStream>
#include <QDebug>

struct jidfull_t;

struct jidbare_t{
    QByteArray domain;
    QByteArray local;

    friend QByteArray& operator<<(QByteArray& ostr, const jidbare_t& jid){
        if(!jid.local.isEmpty()){
            ostr.append(jid.local + '@');
        }
        ostr.append(jid.domain);
        return ostr;
    }

    friend QDataStream& operator<<(QDataStream& os, const jidbare_t& jid){
        QByteArray arr;
        arr << jid;
        os << arr;
        return os;
    }

    friend QDebug& operator<<(QDebug& debug, const jidbare_t& jid){
        QDebugStateSaver saver(debug);
        QByteArray arr;
        arr << jid;
        debug.nospace() << arr;
        return debug;
    }

    friend bool operator==(const jidbare_t& l, const jidbare_t& r){
        if(l.domain != r.domain)        return false;
        if(l.local != r.local)          return false;
        return true;
    }

    friend bool operator<(const jidbare_t& l, const jidbare_t& r){
        QByteArray l_str = l.domain+l.local;
        QByteArray r_str = r.domain+r.local;
        return l_str < r_str;
    }

    jidbare_t& operator=(const jidbare_t&)=default;
    jidbare_t& operator=(jidbare_t&&)=default;

    jidbare_t(const QByteArray& in){
        auto f_split = in.split('@');
        if(f_split.length() == 2){
            local = f_split.at(0);
            auto s_split = f_split.at(1).split('/');
            domain = s_split.at(0);
        }else{
            auto s_split = f_split.at(0).split('/');
            domain = s_split.at(0);
        }
    }

    jidbare_t(QByteArray&& in){
        auto f_split = in.split('@');
        if(f_split.length() == 2){
            local = f_split.at(0);
            auto s_split = f_split.at(1).split('/');
            domain = s_split.at(0);
        }else{
            auto s_split = f_split.at(0).split('/');
            domain = s_split.at(0);
        }
    }

    jidbare_t(const QByteArray& domain_, const QByteArray& local_){
        domain = domain_;
        local = local_;
    }

    jidbare_t(QByteArray&& domain_, QByteArray&& local_){
        domain = std::move(domain_);
        local = std::move(local_);
    }

    jidbare_t(const jidbare_t& jid){
        domain = jid.domain;
        local = jid.local;
    }

    jidbare_t(jidbare_t&& jid){
        domain = std::move(jid.domain);
        local = std::move(jid.local);
    }

    jidbare_t(const jidfull_t& jid);

    jidbare_t(jidfull_t&& jid);

    jidbare_t()=default;

    QByteArray str(){
        QByteArray arr;
        arr << *this;
        return arr;
    }

};

struct jidfull_t : jidbare_t{
    QByteArray resource;

    friend QDataStream& operator<<(QDataStream& os, const jidfull_t& jid){
        QByteArray arr;
        arr << jid;
        os << arr;
        return os;
    }

    friend QByteArray& operator<<(QByteArray& ostr, const jidfull_t& jid){
        if(!jid.local.isEmpty()){
            ostr.append(jid.local + '@');
        }
        ostr.append(jid.domain);
        if(!jid.resource.isEmpty()){
            ostr.append('/' + jid.resource);
        }
        return ostr;
    }

    friend QDebug& operator<<(QDebug& debug, const jidfull_t& jid){
        QDebugStateSaver saver(debug);
        QByteArray arr;
        arr << jid;
        debug.nospace() << arr;
        return debug;
    }

    friend bool operator==(const jidfull_t& l, const jidfull_t& r){
        if(l.domain != r.domain)        return false;
        if(l.local != r.local)          return false;
        if(l.resource != r.resource)    return false;
        return true;
    }

    friend bool operator<(const jidfull_t& l, const jidfull_t& r){
        QByteArray l_str = l.domain+l.local+l.resource;
        QByteArray r_str = r.domain+r.local+r.resource;
        return l_str < r_str;
    }

    jidfull_t& operator=(const jidfull_t&)=default;
    jidfull_t& operator=(jidfull_t&&)=default;

    jidfull_t(const QByteArray& in){
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

    jidfull_t(QByteArray&& in){
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

    jidfull_t(const QByteArray& domain_, const QByteArray& local_, const QByteArray& resource_)
        : jidbare_t(domain_, local_)
    {
        resource = resource_;
    }

    jidfull_t(const jidfull_t& jid)
        : jidbare_t(jid.domain, jid.local)
    {
        resource = jid.resource;
    }

    jidfull_t(jidfull_t&& jid)
        : jidbare_t(std::move(jid.domain), std::move(jid.local))
    {
        resource = std::move(jid.resource);
    }

    jidfull_t(const jidbare_t& jid)
        : jidbare_t(jid.domain, jid.local)
    {}

    jidfull_t(jidbare_t&& jid)
        : jidbare_t(std::move(jid.domain), std::move(jid.local))
    {}


    jidfull_t()=default;

    QByteArray str(){
        QByteArray arr;
        arr << *this;
        return arr;
    }

    jidbare_t bare() const{
        jidbare_t bareJid(*this);
        return bareJid;
    }
};

Q_DECLARE_METATYPE(jidbare_t);
Q_DECLARE_METATYPE(jidfull_t)

class Addressable
{
public:
    Addressable(const QByteArray& domain,
                const QByteArray& local="",
                const QByteArray& resource="");

    Addressable(const jidfull_t& jid);
    Addressable(jidfull_t&& jid);

    void setJid(const jidfull_t& jid);
    void setJid(jidfull_t&& jid);
    jidfull_t jid();

protected:
    jidfull_t _jid;
};

#endif // ADDRESSABLE_H
