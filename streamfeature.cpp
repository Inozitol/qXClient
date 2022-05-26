#include "streamfeature.h"

FeatureType operator|(FeatureType l, FeatureType r){
    return static_cast<FeatureType>(
                static_cast<std::underlying_type_t<FeatureType>>(l) |
                static_cast<std::underlying_type_t<FeatureType>>(r)
                );
}

FeatureType operator|=(FeatureType& l, FeatureType r){
    l = l | r;
    return l;
}

FeatureType operator&(FeatureType l, FeatureType r){
    return static_cast<FeatureType>(
                static_cast<std::underlying_type_t<FeatureType>>(l) &
                static_cast<std::underlying_type_t<FeatureType>>(r)
                );
}
