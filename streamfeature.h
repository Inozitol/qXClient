#ifndef STREAMFEATURE_H
#define STREAMFEATURE_H

#include <QSet>
#include <QString>

#include "saslmechanisms.h"
#include "infoquery.h"

enum class FeatureType : quint32{
    UNKNOWN,
    STARTTLS =      1 << 0,
    SASL =          1 << 1,
    RESOURCEBIND =  1 << 2
};

FeatureType operator|(FeatureType l, FeatureType r);
FeatureType operator|=(FeatureType& l, FeatureType r);
FeatureType operator&(FeatureType l, FeatureType r);

struct Feature{
    Feature(FeatureType type_) : type(type_){}
    FeatureType type = FeatureType::UNKNOWN;
    bool required = false;
};

enum class StateSTARTTLS{
    ASK,
    PROCEED,
    FAILURE
};

struct FeatureSTARTTLS : Feature{
    FeatureSTARTTLS() : Feature(FeatureType::STARTTLS){};
    StateSTARTTLS state = StateSTARTTLS::ASK;
};

enum class StateSASL{
    AUTH,
    CHALLENGE,
    SUCCESS
};

struct FeatureSASL : Feature{
    FeatureSASL() : Feature(FeatureType::SASL){};
    QSet<QString> srv_mechanisms;
    StateSASL state = StateSASL::AUTH;
    SASLSupported mechanism;
    QByteArray challenge;
    QByteArray server_sig;
};

struct FeatureBind : Feature{
    FeatureBind() : Feature(FeatureType::RESOURCEBIND){};
    QByteArray query_id;
    InfoQuery result;
};

#endif // STREAMFEATURE_H
