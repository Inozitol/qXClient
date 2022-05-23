#ifndef SASLMECHANISMS_H
#define SASLMECHANISMS_H

#include <QString>
#include <QMap>

#define FOREACH_MECHANISM(MECH) \
    MECH(SCRAM_SHA_1) \
    MECH(PLAIN)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_E2S(ENUM) {SASLSupported::ENUM, QString(#ENUM).replace('_','-')},

enum class SASLSupported{
    FOREACH_MECHANISM(GENERATE_ENUM)
};

const QMap<SASLSupported, QString> SASL2StrMapper = {
    FOREACH_MECHANISM(GENERATE_E2S)
};

#endif // SASLMECHANISMS_H
