////  VCSDKCoreBPMDetect.h
//  VoiceChangerCFramework
//
//  Created on 2020/12/15.
//  
//

#ifndef VCSDKCoreBPMDetect_h
#define VCSDKCoreBPMDetect_h

#include "VCSDKCoreType.h"
#include "VCSDKCoreFIFOSampleBuffer.hpp"


namespace vcsdkcore
{

/// Minimum allowed BPM rate. Used to restrict accepted result above a reasonable limit.
#define MIN_BPM 29

/// Maximum allowed BPM rate. Used to restrict accepted result below a reasonable limit.
#define MAX_BPM 200


/// Class for calculating BPM rate for audio data.
class VCSDKCoreBPMDetect
{
protected:
    /// Auto-correlation accumulator bins.
    float *xcorr;
    
    /// Amplitude envelope sliding average approximation level accumulator
    double envelopeAccu;

    /// RMS volume sliding average approximation level accumulator
    double RMSVolumeAccu;

    /// Sample average counter.
    int decimateCount;

    /// Sample average accumulator for FIFO-like decimation.
    vcsdkcore::LONG_SAMPLETYPE decimateSum;

    /// Decimate sound by this coefficient to reach approx. 500 Hz.
    int decimateBy;

    /// Auto-correlation window length
    int windowLen;

    /// Number of channels (1 = mono, 2 = stereo)
    int channels;

    /// sample rate
    int sampleRate;

    /// Beginning of auto-correlation window: Autocorrelation isn't being updated for
    /// the first these many correlation bins.
    int windowStart;
 
    /// FIFO-buffer for decimated processing samples.
    vcsdkcore::VCSDKCoreFIFOSampleBuffer *buffer;
    

    /// Updates auto-correlation function for given number of decimated samples that
    /// are read from the internal 'buffer' pipe (samples aren't removed from the pipe
    /// though).
    void updateXCorr(int process_samples      /// How many samples are processed.
                     );

    /// Decimates samples to approx. 500 Hz.
    ///
    /// \return Number of output samples.
    int decimate(SAMPLETYPE *dest,      ///< Destination buffer
                 const SAMPLETYPE *src, ///< Source sample buffer
                 int numsamples                     ///< Number of source samples.
                 );

    /// Calculates amplitude envelope for the buffer of samples.
    /// Result is output to 'samples'.
    void calcEnvelope(SAMPLETYPE *samples,  ///< Pointer to input/output data buffer
                      int numsamples                    ///< Number of samples in buffer
                      );

    /// remove constant bias from xcorr data
    void removeBias();

public:
    /// Constructor.
    VCSDKCoreBPMDetect(int numChannels,  ///< Number of channels in sample data.
              int sampleRate    ///< Sample rate in Hz.
              );

    /// Destructor.
    virtual ~VCSDKCoreBPMDetect();

    /// Inputs a block of samples for analyzing: Envelopes the samples and then
    /// updates the autocorrelation estimation. When whole song data has been input
    /// in smaller blocks using this function, read the resulting bpm with 'getBpm'
    /// function.
    ///
    /// Notice that data in 'samples' array can be disrupted in processing.
    void inputSamples(const SAMPLETYPE *samples,    ///< Pointer to input/working data buffer
                      int numSamples                            ///< Number of samples in buffer
                      );


    /// Analyzes the results and returns the BPM rate. Use this function to read result
    /// after whole song data has been input to the class by consecutive calls of
    /// 'inputSamples' function.
    ///
    /// \return Beats-per-minute rate, or zero if detection failed.
    float getBpm();
};

}







#endif /* VCSDKCoreBPMDetect_h */


