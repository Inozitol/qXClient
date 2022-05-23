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
  jid_t jid();

protected:
  jid_t _jid;
};

#endif // ADDRESSABLE_H
