#include "addressable.h"

Addressable::Addressable(QString domain, QString local, QString resource)
  : _jid({domain, local, resource}){}

jid_t Addressable::jid(){
  return _jid;
}
