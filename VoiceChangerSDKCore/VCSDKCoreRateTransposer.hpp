////  VCSDKCoreRateTransposer.hpp
//  VoiceChangerCFramework
//
//  Created on 2020/12/21.
//  
//



#ifndef VCSDKCoreRateTransposer_hpp
#define VCSDKCoreRateTransposer_hpp

#include <stddef.h>
#include "VCSDKCoreAAFilter.hpp"
#include "VCSDKCoreFIFOSamplePipe.h"
#include "VCSDKCoreFIFOSampleBuffer.hpp"

#include "VCSDKCoreType.h"


namespace  vcsdkcore {


class VCSDKCoreTransposerBase {
    
    
public:
        enum ALGORITHM {
        LINEAR = 0,
        CUBIC,
        SHANNON
    };

protected:
    virtual void resetRegisters() = 0;

    virtual int transposeMono(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples)  = 0;
    virtual int transposeStereo(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples) = 0;
    virtual int transposeMulti(SAMPLETYPE *dest,
                        const SAMPLETYPE *src,
                        int &srcSamples) = 0;

    static ALGORITHM algorithm;

public:
    float rate;
    int numChannels;

    VCSDKCoreTransposerBase();
    virtual ~VCSDKCoreTransposerBase();

    virtual int transpose(VCSDKCoreFIFOSampleBuffer &dest, VCSDKCoreFIFOSampleBuffer &src);
    virtual void setRate(float newRate);
    virtual void setChannels(int channels);

    // static factory function
    static VCSDKCoreTransposerBase *newInstance();

    // static function to set interpolation algorithm
    static void setAlgorithm(ALGORITHM a);
};


/// A common linear samplerate transposer class.
///
class VCSDKCoreRateTransposer : public VCSDKCoreFIFOProcessor
{
protected:
    /// Anti-alias filter object
    VCSDKCoreAAFilter *pAAFilter;
    VCSDKCoreTransposerBase *pTransposer;

    /// Buffer for collecting samples to feed the anti-alias filter between
    /// two batches
    VCSDKCoreFIFOSampleBuffer inputBuffer;

    /// Buffer for keeping samples between transposing & anti-alias filter
    VCSDKCoreFIFOSampleBuffer midBuffer;

    /// Output sample buffer
    VCSDKCoreFIFOSampleBuffer outputBuffer;

    BOOL bUseAAFilter;


    /// Transposes sample rate by applying anti-alias filter to prevent folding.
    /// Returns amount of samples returned in the "dest" buffer.
    /// The maximum amount of samples that can be returned at a time is set by
    /// the 'set_returnBuffer_size' function.
    void processSamples(const SAMPLETYPE *src,
                        uint numSamples);

public:
    VCSDKCoreRateTransposer();
    virtual ~VCSDKCoreRateTransposer();

    /// Operator 'new' is overloaded so that it automatically creates a suitable instance
    /// depending on if we're to use integer or floating point arithmetics.
//    static void *operator new(size_t s);

    /// Use this function instead of "new" operator to create a new instance of this class.
    /// This function automatically chooses a correct implementation, depending on if
    /// integer ot floating point arithmetics are to be used.
//    static RateTransposer *newInstance();

    /// Returns the output buffer object
    VCSDKCoreFIFOSamplePipe *getOutput() { return &outputBuffer; };

    /// Returns the store buffer object
//    FIFOSamplePipe *getStore() { return &storeBuffer; };

    /// Return anti-alias filter object
    VCSDKCoreAAFilter *getAAFilter();

    /// Enables/disables the anti-alias filter. Zero to disable, nonzero to enable
    void enableAAFilter(BOOL newMode);

    /// Returns nonzero if anti-alias filter is enabled.
    BOOL isAAFilterEnabled() const;

    /// Sets new target rate. Normal rate = 1.0, smaller values represent slower
    /// rate, larger faster rates.
    virtual void setRate(float newRate);

    /// Sets the number of channels, 1 = mono, 2 = stereo
    void setChannels(int channels);

    /// Adds 'numSamples' pcs of samples from the 'samples' memory position into
    /// the input of the object.
    void putSamples(const SAMPLETYPE *samples, uint numSamples);

    /// Clears all the samples in the object
    void clear();

    /// Returns nonzero if there aren't any samples available for outputting.
    int isEmpty() const;
};

}




#endif /* VCSDKCoreRateTransposer_hpp */
