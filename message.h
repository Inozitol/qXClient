#ifndef MESSAGE_H
#define MESSAGE_H

#include "stanza.h"

class Message : public Stanza
{
public:
    Message();
    Message(QXmlStreamReader& reader);
};

#endif // MESSAGE_H
