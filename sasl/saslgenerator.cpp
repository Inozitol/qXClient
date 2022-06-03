#include "saslgenerator.h"

namespace SASL{
SASLGenerator::SASLGenerator(const SASLSupported& sasl, const Credentials& cred)
    : _sasl(sasl),
      _cred(cred)
{}

SASLSupported SASLGenerator::GetSASL() const
{
    return _sasl;
}

}
