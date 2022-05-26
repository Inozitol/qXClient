#ifndef STRSWITCH_H
#define STRSWITCH_H

#include <QtGlobal>

QT_BEGIN_NAMESPACE
template <typename Key, typename T> class QHash;
class QByteArray;
QT_END_NAMESPACE

#define FOREACH_XMLWORD(WORD) \
    WORD(invalid)       \
    WORD(stream)        \
    WORD(features)      \
    WORD(starttls)      \
    WORD(mechanisms)    \
    WORD(mechanism)     \
    WORD(required)      \
    WORD(proceed)       \
    WORD(challenge)     \
    WORD(success)       \
    WORD(bind)          \
    WORD(iq)            \
    WORD(message)       \
    WORD(presence)      \
    WORD(to)            \
    WORD(from)          \
    WORD(id)            \
    WORD(type)          \
    WORD(lang)          \
    WORD(result)        \
    WORD(error)         \
    WORD(show)          \
    WORD(status)

#define LAMBDA_ENUM(ENUM)       [](){ return XMLWord::ENUM; }

#define GENERATE_ENUM(ENUM)     ENUM,
#define GENERATE_WORDMAP(ENUM)  {#ENUM, LAMBDA_ENUM(ENUM)},

enum class XMLWord{
    FOREACH_XMLWORD(GENERATE_ENUM)
};

extern const QHash<QByteArray,std::function<XMLWord()>> map_word2int;

extern XMLWord word2int(const QByteArray& word);


#endif // STRSWITCH_H
