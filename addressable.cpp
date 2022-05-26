#include "addressable.h"

Addressable::Addressable(const QByteArray& domain, const QByteArray& local, const QByteArray& resource)
  : _jid({domain, local, resource}){}

jid_t Addressable::jid(){
  return _jid;
}

void Addressable::setJid(const jid_t& jid){
    _jid = jid;
}
