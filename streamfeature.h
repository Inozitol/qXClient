#ifndef STREAMFEATURE_H
#define STREAMFEATURE_H

#include <QSet>
#include <QString>
#include "saslmechanisms.h"

enum class FeatureType{
    UNKNOWN,
    STARTTLS,
    SASL,
    RESOURCEBIND
};

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

enum class StateBind{
    GENERATE
};

struct FeatureBind : Feature{
    FeatureBind() : Feature(FeatureType::RESOURCEBIND){};
    StateBind state = StateBind::GENERATE;
    QByteArray query_id;
};

#endif // STREAMFEATURE_H
