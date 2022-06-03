#include "scramgenerator.h"

namespace SASL{
SCRAMGenerator::SCRAMGenerator(const SASLSupported& sasl,
                               const Credentials& cred)
    :SASLGenerator(sasl, cred)
{
    switch(sasl){
    case SASLSupported::SCRAM_SHA_1:
        _hash_algo = QCryptographicHash::Sha1;
        _hash = std::make_shared<QCryptographicHash>(_hash_algo);
        _hmac = std::make_shared<QMessageAuthenticationCode>(_hash_algo);
        KEY_LEN = 20;
        break;
    default:
        throw std::invalid_argument("Unsupported SASL");
    }
}

void SCRAMGenerator::writeAttribute(const Attributes& attr, const QByteArray& data){
    _attrs[attr] = data;
}

void SCRAMGenerator::setSCRAMFlag(const CHANNEL_FLAG& flag){
    switch(flag){
    case CHANNEL_FLAG::Y:
        _attrs[Attributes::SCRAM_FLAG] = "y";
        break;
    case CHANNEL_FLAG::N:
        _attrs[Attributes::SCRAM_FLAG] = "n";
        break;
    case CHANNEL_FLAG::P:
        _attrs[Attributes::SCRAM_FLAG] = "p";
        break;
    }
}

QByteArray SCRAMGenerator::getInitResponse(){
    switch(_sasl){
    case SASLSupported::SCRAM_SHA_1:
    {
        QByteArray result;
        if(_attrs.contains(Attributes::SCRAM_FLAG))
            result.append(_attrs.value(Attributes::SCRAM_FLAG));
        result.append(",,n=");
        if(_attrs.contains(Attributes::NAME))
            result.append(_attrs.value(Attributes::NAME));
        result.append(",r=");
        if(_attrs.contains(Attributes::NONCE_CLIENT))
            result.append(_attrs.value(Attributes::NONCE_CLIENT));
        return result;
    }
    default:
        return "";
    }
}

QByteArray SCRAMGenerator::getChallengeResponse(){
    QByteArray result;
    result.append("c=");
    result.append(GS2Header().toBase64());
    result.append(",r=");
    if(_attrs.contains(Attributes::NONCE_FULL))
        result.append(_attrs.value(Attributes::NONCE_FULL));
    result.append(",p=");
    result.append(getClientProof().toBase64());

    return result;
}

QByteArray SCRAMGenerator::getSaltedPass() const{
    return QPasswordDigestor::deriveKeyPbkdf2(_hash_algo,
                                              _cred.getPass(),
                                              _attrs.value(Attributes::SALT),
                                              _attrs.value(Attributes::ITER_COUNT).toInt(),
                                              KEY_LEN);

}

QByteArray SCRAMGenerator::getAuthMessage() const{
    QByteArray cli_frmb;
    QByteArray srv_frm;
    QByteArray cli_fimwp;

    cli_frmb =  "n=" + _attrs.value(Attributes::NAME) + "," +
            "r=" + _attrs.value(Attributes::NONCE_CLIENT);

    srv_frm =   "r=" + _attrs.value(Attributes::NONCE_FULL) + "," +
            "s=" + _attrs.value(Attributes::SALT).toBase64() + "," +
            "i=" + _attrs.value(Attributes::ITER_COUNT);

    cli_fimwp = "c=" + GS2Header().toBase64() + "," +
            "r=" + _attrs.value(Attributes::NONCE_FULL);

    return cli_frmb + "," + srv_frm + "," + cli_fimwp;

}

QByteArray SCRAMGenerator::getClientProof() const{

    QByteArray salt_pass = getSaltedPass();
    _hmac->setKey(salt_pass);
    _hmac->addData("Client Key");
    QByteArray client_key = _hmac->result();
    _hmac->reset();

    _hash->addData(client_key);
    QByteArray stored_key = _hash->result();
    _hash->reset();

    _hmac->setKey(stored_key);
    _hmac->addData(getAuthMessage());
    QByteArray client_signature = _hmac->result();
    _hmac->reset();

    QByteArray client_proof = Utils::getXOR(client_key, client_signature);

    return client_proof;
}

QByteArray SCRAMGenerator::getAttribute(const Attributes& attr) const{
    if(_attrs.contains(attr)){
        return _attrs.value(attr);
    }
    return "";
}

void SCRAMGenerator::loadChallenge(const QByteArray& challenge){
    QList<QByteArray> tokens = challenge.split(',');
    for(const auto& token : tokens){
        const char atype = token[0];
        const char* data = &token.data()[2];
        switch(atype){
        case 'r':   // NONCE
            // TODO Check that first part is same as on client
            writeAttribute(Attributes::NONCE_FULL, data);
            break;
        case 's':   // SALT
            writeAttribute(Attributes::SALT, QByteArray::fromBase64(data));
            break;
        case 'i':   // ITERS
            writeAttribute(Attributes::ITER_COUNT, data);
            break;
        }
    }
}

QByteArray SCRAMGenerator::GS2Header() const{
    QByteArray flag = _attrs.value(Attributes::SCRAM_FLAG);
    return flag.append(",,");
}

bool SCRAMGenerator::isServerSigValid(QByteArray sig) const{
    const char* signature = &sig.data()[2];
    QByteArray salt_pass = getSaltedPass();
    _hmac->setKey(salt_pass);
    _hmac->addData("Server Key");
    QByteArray server_key = _hmac->result();
    _hmac->setKey(server_key);
    _hmac->addData(getAuthMessage());
    QByteArray server_sig = _hmac->result();
    _hmac->reset();
    return signature == server_sig.toBase64();
}

}
