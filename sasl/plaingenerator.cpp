#include "plaingenerator.h"

namespace SASL{
    PLAINGenerator::PLAINGenerator(const SASLSupported& sasl,
                                   const Credentials& cred)
        :SASLGenerator(sasl,cred){}

    void PLAINGenerator::loadChallenge(const QByteArray&){}

    QByteArray PLAINGenerator::getChallengeResponse(){
        return "";
    }

    QByteArray PLAINGenerator::getInitResponse(){
        QByteArray response;
        response.append('\0');
        response.append(_id);
        response.append('\0');
        response.append(_cred.getPass());
        return response;
    }

    void PLAINGenerator::writeID(const QByteArray& id){
        _id = id;
    }
};
