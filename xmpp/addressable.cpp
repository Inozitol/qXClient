#include "addressable.h"

jidbare_t::jidbare_t(const jidfull_t& jid){
    domain = jid.domain;
    local = jid.local;
}

jidbare_t::jidbare_t(jidfull_t&& jid){
    domain = std::move(jid.domain);
        local = std::move(jid.local);
}

Addressable::Addressable(const QByteArray& domain, const QByteArray& local, const QByteArray& resource)
    : _jid({domain, local, resource})
{}

Addressable::Addressable(const jidfull_t& jid)
    :_jid(jid)
{}

Addressable::Addressable(jidfull_t&& jid)
    :_jid(jid)
{}

jidfull_t Addressable::jid() const{
    return _jid;
}

void Addressable::setJid(const jidfull_t& jid){
    _jid = jid;
}

void Addressable::setJid(jidfull_t&& jid){
    _jid = jid;
}
