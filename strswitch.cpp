#include "strswitch.h"

#include <QHash>
#include <QByteArray>

const QHash<QByteArray,std::function<XMLWord()>> map_word2int = {
    FOREACH_XMLWORD(GENERATE_WORDMAP)
};

XMLWord word2int(const QByteArray& word){
    static const auto end = map_word2int.end();
    auto it = map_word2int.constFind(word);
    if(it != end){
        return it.value()();
    }else{
        return XMLWord::invalid;
    }
}
