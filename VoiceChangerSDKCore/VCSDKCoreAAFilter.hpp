////  VCSDKCoreAAFilter.hpp
//  VoiceChanger
//
//  Created on 2020/12/15.
//  
//

#ifndef VCSDKCoreAAFilter_hpp
#define VCSDKCoreAAFilter_hpp

#include "VCSDKCoreType.h"
#include "VCSDKCoreFIFOSampleBuffer.hpp"


// remain add
#include "VCSDKCoreFIRFilter.hpp"


namespace vcsdkcore {


class VCSDKCoreAAFilter {

protected:
    
    class VCSDKCoreFIRFilter *pFIR;
    
    /// Low-pass filter cut-off frequency, negative = invalid
    double cutoffFreq;

    /// num of filter taps
    uint length;

    /// Calculate the FIR coefficients realizing the given cutoff-frequency
    void calculateCoeffs();
public:
    VCSDKCoreAAFilter(uint length);

    ~VCSDKCoreAAFilter();

    /// Sets new anti-alias filter cut-off edge frequency, scaled to sampling
    /// frequency (nyquist frequency = 0.5). The filter will cut off the
    /// frequencies than that.
    void setCutoffFreq(double newCutoffFreq);

    /// Sets number of FIR filter taps, i.e. ~filter complexity
    void setLength(uint newLength);

    uint getLength() const;

    /// Applies the filter to the given sequence of samples.
    /// Note : The amount of outputted samples is by value of 'filter length'
    /// smaller than the amount of input samples.
    uint evaluate(SAMPLETYPE *dest,
                  const SAMPLETYPE *src,
                  uint numSamples,
                  uint numChannels) const;

    /// Applies the filter to the given src & dest pipes, so that processed amount of
    /// samples get removed from src, and produced amount added to dest
    /// Note : The amount of outputted samples is by value of 'filter length'
    /// smaller than the amount of input samples.
    uint evaluate(VCSDKCoreFIFOSampleBuffer &dest,
                  VCSDKCoreFIFOSampleBuffer &src) const;

    
};



}



#endif /* VCSDKCoreAAFilter_hpp */



