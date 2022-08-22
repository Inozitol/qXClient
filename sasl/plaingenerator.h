#ifndef PLAINGENERATOR_H
#define PLAINGENERATOR_H

#include "saslgenerator.h"

namespace SASL{

class PLAINGenerator : public SASLGenerator
{
public:
    PLAINGenerator(const SASLSupported& sasl, const Credentials& cred);

    void loadChallenge(const QByteArray& challenge) override;
    QByteArray getInitResponse() override;
    QByteArray getChallengeResponse() override;

    void writeID(const QByteArray& id);

private:
    QByteArray _id;
};

}

#endif // PLAINGENERATOR_H
