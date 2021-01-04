////  VCSDKCoreInterpolateShannon.hpp
//  VoiceChangerCFramework
//
//  Created on 2020/12/21.
//  
//

#ifndef VCSDKCoreInterpolateShannon_hpp
#define VCSDKCoreInterpolateShannon_hpp


#include "VCSDKCoreRateTransposer.hpp"
#include "VCSDKCoreType.h"

namespace vcsdkcore
{

class InterpolateShannon : public VCSDKCoreTransposerBase
{
protected:
    void resetRegisters();
    int transposeMono(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples);
    int transposeStereo(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples);
    int transposeMulti(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples);

    float fract;

public:
    InterpolateShannon();
};

}







#endif /* VCSDKCoreInterpolateShannon_hpp */
