#ifndef MESSAGE_H
#define MESSAGE_H

#include "stanza.h"

class Message : public Stanza
{
public:
    enum Flag{
        DELAYED = 1 << 0
    };
    Q_DECLARE_FLAGS(Flags, Flag);

    Message();
    Message(const Message& message);
    Message(Message&& message);
    Message(QXmlStreamReader& reader);

    Message& operator=(const Message&)=default;

    QString getBody() const;
    QString getBody(const QLocale& locale) const;
    void setBody(const QString& str, const QLocale& locale);
    void setBody(const QString& str);
    QString getSubject() const;
    QString getSubject(const QLocale& locale) const;
    void setThread(const QByteArray& thread);
    void setThread(QByteArray&& thread);
    void setThread();
    void setThreadParent(const QByteArray& thread);
    void setThreadParent(QByteArray&& thread);
    void setThreadParent();

    void setFlag(Flag flgs);
    bool isDelayed();
    QString payloadNS() const;

    static const unsigned int THREAD_LEN = 10;
private:
    QHash<QLocale, QString> _body_map;
    QHash<QLocale, QString> _subj_map;
    QByteArray _threadParent;
    QByteArray _thread;
    QDomElement _threadElem;
    Flags m_flgFlags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Message::Flags);
Q_DECLARE_METATYPE(Message);

#endif // MESSAGE_H
