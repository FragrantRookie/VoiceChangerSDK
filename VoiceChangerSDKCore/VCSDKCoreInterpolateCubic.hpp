////  VCSDKCoreInterpolateCubic.hpp
//  VoiceChangerCFramework
//
//  Created on 2020/12/21.
//  
//


#ifndef VCSDKCoreInterpolateCubic_hpp
#define VCSDKCoreInterpolateCubic_hpp

#include "VCSDKCoreRateTransposer.hpp"
#include "VCSDKCoreType.h"


namespace vcsdkcore {

class VCSDKCoreInterpolateCubic : public VCSDKCoreTransposerBase {

protected:
    virtual void resetRegisters();
    virtual int transposeMono(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples);
    virtual int transposeStereo(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples);
    virtual int transposeMulti(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples);

    float fract;

public:
    VCSDKCoreInterpolateCubic();
    
};


}








#endif /* VCSDKCoreInterpolateCubic_hpp */



