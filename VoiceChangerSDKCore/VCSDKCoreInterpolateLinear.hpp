////  VCSDKCoreInterpolateLinear.hpp
//  VoiceChangerCFramework
//
//  Created on 2020/12/21.
//  
//

#ifndef VCSDKCoreInterpolateLinear_hpp
#define VCSDKCoreInterpolateLinear_hpp

#include "VCSDKCoreRateTransposer.hpp"
#include "VCSDKCoreType.h"

namespace vcsdkcore
{

/// Linear transposer class that uses integer arithmetics
class InterpolateLinearInteger : public vcsdkcore::VCSDKCoreTransposerBase
{
protected:
    int iFract;
    int iRate;

    virtual void resetRegisters();

    virtual int transposeMono(SAMPLETYPE *dest,
                       const SAMPLETYPE *src,
                       int &srcSamples);
    virtual int transposeStereo(SAMPLETYPE *dest,
                         const SAMPLETYPE *src,
                         int &srcSamples);
    virtual int transposeMulti(SAMPLETYPE *dest, const SAMPLETYPE *src, int &srcSamples);
public:
    InterpolateLinearInteger();

    /// Sets new target rate. Normal rate = 1.0, smaller values represent slower
    /// rate, larger faster rates.
    virtual void setRate(float newRate);
};


/// Linear transposer class that uses floating point arithmetics
class InterpolateLinearFloat : public VCSDKCoreTransposerBase
{
protected:
    float fract;

    virtual void resetRegisters();

    virtual int transposeMono(SAMPLETYPE *dest,
                       const SAMPLETYPE *src,
                       int &srcSamples);
    virtual int transposeStereo(SAMPLETYPE *dest,
                         const SAMPLETYPE *src,
                         int &srcSamples);
    virtual int transposeMulti(SAMPLETYPE *dest, const SAMPLETYPE *src, int &srcSamples);

public:
    InterpolateLinearFloat();
};

}








#endif /* VCSDKCoreInterpolateLinear_hpp */


