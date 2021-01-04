////  VCSDKCoreFIRFilter.hpp
//  VoiceChanger
//
//  Created on 2020/12/15.
//  
//


#ifndef VCSDKCoreFIRFilter_hpp
#define VCSDKCoreFIRFilter_hpp

#include <stddef.h>
#include "VCSDKCoreType.h"


namespace vcsdkcore {


class VCSDKCoreFIRFilter {
    
    
protected:
    
    //
    uint length;
    
    //
    uint lengthDiv8;
    
    uint resultDivFactor;
    
    SAMPLETYPE resultDivider;
    
    SAMPLETYPE *filterCoeffs;
    
    virtual uint evaluateFilterStereo(SAMPLETYPE *dest,const SAMPLETYPE *src,uint numSamples) const;
    virtual uint evaluateFilterMono(SAMPLETYPE *dest,const SAMPLETYPE *src,uint numSamples) const;
    virtual uint evaluateFilterMulti(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples, uint numChannels) const;
    
    
public:
    VCSDKCoreFIRFilter();
    virtual ~VCSDKCoreFIRFilter();
    
    
    static void *operator new(size_t s);
    
    static VCSDKCoreFIRFilter *newInstance();
    
    uint evaluate(SAMPLETYPE *dest,
                  const SAMPLETYPE *src,
                  uint numSamples,
                  uint numChannels) const;
    
    uint getLength() const;
    
    virtual void setCoefficients(const SAMPLETYPE *coeffs,
                                 uint newLength,
                                 uint uResultDivFactor);
    
};


// Optional subclasses that implement CPU-specific optimizations:
#ifdef SOUNDTOUCH_ALLOW_MMX

class VCSDKCoreFIRFilterMMX: public VCSDKCoreFIRFilter {
protected:
    short *filterCoeffsUnalign;
    short *filterCoeffsAlign;
    
    virtual uint evaluateFilterStereo(short *dest, const short *src, uint numSamples) const;
public:
    VCSDKCoreFIRFilterMMX();
    ~VCSDKCoreFIRFilterMMX();
    
    virtual void setCoefficients(const short *coeffs, uint newLength, uint uResultDivFactor);
};
#endif  // SOUNDTOUCH_ALLOW_MMX



#ifdef SOUNDTOUCH_ALLOW_SSE

/// Class that implements SSE optimized functions exclusive for floating point samples type.
class VCSDKCoreFIRFilterSSE : public VCSDKCoreFIRFilter {
protected:
    float *filterCoeffsUnalign;
    float *filterCoeffsAlign;

    virtual uint evaluateFilterStereo(float *dest, const float *src, uint numSamples) const;
public:
    FIRFilterSSE();
    ~FIRFilterSSE();

    virtual void setCoefficients(const float *coeffs, uint newLength, uint uResultDivFactor);
};

#endif   // SOUNDTOUCH_ALLOW_SSE


}


#endif /* VCSDKCoreFIRFilter_hpp */



