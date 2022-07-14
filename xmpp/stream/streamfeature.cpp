#include "streamfeature.h"

Feature::Type operator|(Feature::Type l, Feature::Type r){
    return static_cast<Feature::Type>(
                static_cast<std::underlying_type_t<Feature::Type>>(l) |
                static_cast<std::underlying_type_t<Feature::Type>>(r)
                );
}

Feature::Type operator|=(Feature::Type& l, Feature::Type r){
    l = l | r;
    return l;
}

bool operator&(Feature::Type l, Feature::Type r){
    return static_cast<Feature::Type>(
                static_cast<std::underlying_type_t<Feature::Type>>(l) &
                static_cast<std::underlying_type_t<Feature::Type>>(r)
                ) == r;
}
