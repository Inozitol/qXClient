#ifndef STREAMFEATURE_H
#define STREAMFEATURE_H

#include <algorithm>

#include <QSet>
#include <QString>
#include <QCryptographicHash>

#include "../disco.h"
#include "../stanza/infoquery.h"
#include "../../sasl/saslmechanisms.h"


struct Feature{
    enum class Type : quint32{
        UNKNOWN,
        STARTTLS =      1 << 0,
        SASL =          1 << 1,
        RESOURCEBIND =  1 << 2,
        MANAGEMENT =    1 << 3,
        CAPS =          1 << 4,
    };

    friend Type operator|(Type l, Type r);
    friend Type operator|=(Type& l, Type r);
    friend bool operator&(Type l, Type r);

    Feature(Type type_) : type(type_){}
    Type type = Type::UNKNOWN;
    bool required = false;
};


struct FeatureSTARTTLS : Feature{
    enum class State{
        ASK,
        PROCEED,
        FAILURE
    };
    FeatureSTARTTLS() : Feature(Feature::Type::STARTTLS){};
    State state = State::ASK;
};

struct FeatureSASL : Feature{
    enum class State{
        AUTH,
        CHALLENGE,
        SUCCESS
    };
    FeatureSASL() : Feature(Feature::Type::SASL){};
    QSet<QString> srv_mechanisms;
    State state = State::AUTH;
    SASLSupported mechanism;
    QByteArray challenge;
    QByteArray server_sig;
};

struct FeatureBind : Feature{
    FeatureBind() : Feature(Feature::Type::RESOURCEBIND){};
    QByteArray query_id;
    InfoQuery result;
};

struct FeatureManagement : Feature{
    enum class State{
        ASK,
        ENABLED
    };

    void incrInbound(){
        if(inbound == UINT_MAX){
            inbound = 0;
        }else{
            inbound++;
        }
    }

    void incrOutbound(){
        if(outbound == UINT_MAX){
            outbound = 0;
        }else{
            outbound++;
        }
    }

    FeatureManagement() : Feature(Feature::Type::MANAGEMENT){};
    State state = State::ASK;
    quint32 inbound = 0;
    quint32 outbound = 0;
    bool ack_wait = false;
};

struct FeatureCaps : Feature{
    FeatureCaps() : Feature(Feature::Type::CAPS){};
};


#endif // STREAMFEATURE_H
