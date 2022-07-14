#ifndef STRSWITCH_H
#define STRSWITCH_H

#include <QtGlobal>

QT_BEGIN_NAMESPACE
template <typename Key, typename T> class QHash;
class QByteArray;
QT_END_NAMESPACE

#define STRINGS                     \
    STR(invalid,invalid)            \
    STR(stream,stream)              \
    STR(features,features)          \
    STR(starttls,starttls)          \
    STR(mechanisms,mechanisms)      \
    STR(mechanism,mechanism)        \
    STR(required,required)          \
    STR(proceed,proceed)            \
    STR(challenge,challenge)        \
    STR(success,success)            \
    STR(bind,bind)                  \
    STR(iq,iq)                      \
    STR(message,message)            \
    STR(presence,presence)          \
    STR(to,to)                      \
    STR(from,from)                  \
    STR(id,id)                      \
    STR(type,type)                  \
    STR(lang,lang)                  \
    STR(result,result)              \
    STR(error,error)                \
    STR(show,show)                  \
    STR(status,status)              \
    STR(priority,priority)          \
    STR(body,body)                  \
    STR(subject,subject)            \
    STR(thread,thread)              \
    STR(sm,sm)                      \
    STR(a,a)                        \
    STR(r,r)                        \
    STR(enabled,enabled)            \
    STR(delay,delay)                \
    STR(urn:ietf:params:xml:ns:vcard-4.0, ns_xml_vcard40)                           \
    STR(urn:xmpp:pep-vcard-conversion:0, ns_xmpp_pep_vcard_coversion_0)             \
    STR(urn:xmpp:push:0, ns_xmpp_push_0)                                            \
    STR(urn:xmpp:mam:2, ns_xmpp_mam_2)                                              \
    STR(urn:xmpp:sid:0, ns_xmpp_sid_0)

enum class IntFromString{
    #define STR(str,en) en,
    STRINGS
    #undef STR
};

extern const QHash<QByteArray,IntFromString> map_word2int;

extern IntFromString word2int(const QByteArray& word);


#endif // STRSWITCH_H
