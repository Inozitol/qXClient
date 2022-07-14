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

    bool operator==(const jidbare_t& other) const{
        if(this->domain != other.domain) return false;
        if(this->local  != other.local)  return false;
        return true;
    }

    inline bool operator<(const jidbare_t& other) const{
        QByteArray t_str = this->domain + this->local;
        QByteArray o_str = other.domain + other.local;
        return t_str < o_str;
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

    jidbare_t(const QString& in) : jidbare_t(in.toUtf8()){}

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

    QByteArray toByteArray(){
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

    inline bool operator==(const jidfull_t& other) const{
        if(this->domain   != other.domain)   return false;
        if(this->local    != other.local)    return false;
        if(this->resource != other.resource) return false;
        return true;
    }

    inline bool operator<(const jidfull_t& other) const{
        QByteArray t_str = this->domain + this->local + this->resource;
        QByteArray o_str = other.domain + other.local + other.resource;
        return t_str < o_str;
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

    jidfull_t(const QString& in) : jidfull_t(in.toUtf8()){}

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

    QByteArray toByteArray(){
        QByteArray arr;
        arr << *this;
        return arr;
    }

    jidbare_t bare() const{
        jidbare_t bareJid(*this);
        return bareJid;
    }
};

template<>
struct std::hash<jidbare_t>{
    std::size_t operator()(const jidbare_t& jid) const noexcept{
        QByteArray join = jid.domain + jid.local;
        return std::hash<std::string>{}(join.toStdString());
    }
};

template<>
struct std::hash<jidfull_t>{
    std::size_t operator()(const jidfull_t& jid) const noexcept{
        QByteArray join = jid.domain + jid.local + jid.resource;
        return std::hash<std::string>{}(join.toStdString());
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
    Addressable() = default;

    void setJid(const jidfull_t& jid);
    void setJid(jidfull_t&& jid);
    jidfull_t jid() const;

protected:
    jidfull_t _jid;
};

#endif // ADDRESSABLE_H
