#ifndef SCRAMGENERATOR_H
#define SCRAMGENERATOR_H

#include "saslgenerator.h"

namespace SASL{
class SCRAMGenerator : public SASLGenerator
{
public:
    SCRAMGenerator(const SASLSupported& sasl, const Credentials& cred);

    enum class Attributes{
        NAME,
        NONCE_FULL,
        NONCE_CLIENT,
        SALT,
        ITER_COUNT,
        CLIENT_PROOF,
        ERROR,
        SCRAM_FLAG,
        SALTED_PASS
    };

    enum class CHANNEL_FLAG{
        Y,
        N,
        P
    };

    void loadChallenge(const QByteArray& challenge) override;
    QByteArray getInitResponse() override;
    QByteArray getChallengeResponse() override;

    void writeAttribute(const Attributes& attr, const QByteArray& data);
    QByteArray getAttribute(const Attributes& attr) const;
    void setSCRAMFlag(const CHANNEL_FLAG& flag);
    QByteArray GS2Header() const;
    bool isServerSigValid(QByteArray sig) const;
private:
    QByteArray getClientProof() const;
    QByteArray getSaltedPass() const;
    QByteArray getAuthMessage() const;

    QMap<Attributes, QByteArray> _attrs;
    QCryptographicHash::Algorithm _hash_algo;
    std::shared_ptr<QCryptographicHash> _hash;
    std::shared_ptr<QMessageAuthenticationCode> _hmac;

    unsigned int KEY_LEN;
};


}
#endif // SCRAMGENERATOR_H
