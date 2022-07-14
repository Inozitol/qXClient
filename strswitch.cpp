#include "strswitch.h"

#include <QHash>
#include <QByteArray>

const QHash<QByteArray,IntFromString> map_word2int = {
        #define STR(str,en) {R"(#str)", IntFromString::en},
        STRINGS
        #undef STR
};

IntFromString word2int(const QByteArray& word){
    static const auto end = map_word2int.end();
    auto it = map_word2int.constFind(word);
    if(it != end){
        return it.value();
    }else{
        return IntFromString::invalid;
    }
}
