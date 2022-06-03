#ifndef SASLGENERATOR_H
#define SASLGENERATOR_H

#include <QMessageAuthenticationCode>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDebug>
#include <QPasswordDigestor>

#include "saslmechanisms.h"
#include "../xmpp/creds.h"
#include "../utils.h"

namespace SASL{
    class SASLGenerator
    {
    public:
        SASLGenerator(const SASLSupported& sasl, const Credentials& cred);

        virtual void loadChallenge(const QByteArray& challenge) = 0;
        virtual QByteArray getInitResponse() = 0;
        virtual QByteArray getChallengeResponse() = 0;

        SASLSupported GetSASL() const;

    protected:

        const SASLSupported _sasl;
        const Credentials& _cred;

    };
}

#endif // SASLGENERATOR_H
