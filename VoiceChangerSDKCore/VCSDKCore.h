////  VCSDKCore.hpp
//  VoiceChanger
//
//  Created on 2020/12/11.
//  
//

/** func: main class for tempo/pitch/rate adjusting routines.
 *
 */

#ifndef VCSDKCore_h
#define VCSDKCore_h

#include "VCSDKCoreFIFOSamplePipe.h"
#include "VCSDKCoreType.h"


namespace vcsdkcore {

// Voice changer sdk library version string.
#define KVOICECHANGERSDK_VERSION "1.0.0"

#define KVOICECHANGERSDK_VERSION_ID (10000)


//
// Available setting IDs for the 'setSetting' & 'get_setting' functions:

/// Enable/disable anti-alias filter in pitch transposer (0 = disable) -- 是否使用AA滤波器
#define SETTING_USE_AA_FILTER       0

/// Pitch transposer anti-alias filter length (8 .. 128 taps, default = 32) -- 滤波器阶数，默认值是32
#define SETTING_AA_FILTER_LENGTH    1

/// Enable/disable quick seeking algorithm in tempo changer routine
/// (enabling quick seeking lowers CPU utilization but causes a minor sound
///  quality compromising)  --- 是否使用快速查找方法
#define SETTING_USE_QUICKSEEK       2

/// Time-stretch algorithm single processing sequence length in milliseconds. This determines
/// to how long sequences the original sound is chopped in the time-stretch algorithm.
/// See "STTypes.h" or README for more information.
#define SETTING_SEQUENCE_MS         3

/// Time-stretch algorithm seeking window length in milliseconds for algorithm that finds the
/// best possible overlapping location. This determines from how wide window the algorithm
/// may look for an optimal joining location when mixing the sound sequences back together.
/// See "STTypes.h" or README for more information.
#define SETTING_SEEKWINDOW_MS       4

/// Time-stretch algorithm overlap length in milliseconds. When the chopped sound sequences
/// are mixed back together, to form a continuous sound stream, this parameter defines over
/// how long period the two consecutive sequences are let to overlap each other.
/// See "STTypes.h" or README for more information.
#define SETTING_OVERLAP_MS          5


/// Call "getSetting" with this ID to query nominal average processing sequence
/// size in samples. This value tells approcimate value how many input samples
/// SoundTouch needs to gather before it does DSP processing run for the sample batch.
///
/// Notices:
/// - This is read-only parameter, i.e. setSetting ignores this parameter
/// - Returned value is approximate average value, exact processing batch
///   size may wary from time to time
/// - This parameter value is not constant but may change depending on
///   tempo/pitch/rate/samplerate settings.
#define SETTING_NOMINAL_INPUT_SEQUENCE        6


/// Call "getSetting" with this ID to query nominal average processing output
/// size in samples. This value tells approcimate value how many output samples
/// SoundTouch outputs once it does DSP processing run for a batch of input samples.
///
/// Notices:
/// - This is read-only parameter, i.e. setSetting ignores this parameter
/// - Returned value is approximate average value, exact processing batch
///   size may wary from time to time
/// - This parameter value is not constant but may change depending on
///   tempo/pitch/rate/samplerate settings.
#define SETTING_NOMINAL_OUTPUT_SEQUENCE        7


class VCSDKCore: public VCSDKCoreFIFOProcessor {
    
    
private:
    class VCSDKCoreRateTransposer *pRateTransposer;
    

    /// Time-stretch class instance
    class VCSDKCoreTDStretch *pTDStretch;

    /// Virtual pitch parameter. Effective rate & tempo are calculated from these parameters.
    float virtualRate;

    /// Virtual pitch parameter. Effective rate & tempo are calculated from these parameters.
    float virtualTempo;

    /// Virtual pitch parameter. Effective rate & tempo are calculated from these parameters.
    float virtualPitch;

    /// Flag: Has sample rate been set?
    BOOL  bSrateSet;

    /// Calculates effective rate & tempo valuescfrom 'virtualRate', 'virtualTempo' and
    /// 'virtualPitch' parameters.
    void calcEffectiveRateAndTempo();

protected :
    /// Number of channels
    uint  channels;

    /// Effective 'rate' value calculated from 'virtualRate', 'virtualTempo' and 'virtualPitch'
    float rate;

    /// Effective 'tempo' value calculated from 'virtualRate', 'virtualTempo' and 'virtualPitch'
    float tempo;

public:
    VCSDKCore();
    virtual ~VCSDKCore();
    

    /// Get SoundTouch library version string
    static const char *getVersionString();

    /// Get SoundTouch library version Id
    static uint getVersionId();

    /// Sets new rate control value. Normal rate = 1.0, smaller values
    /// represent slower rate, larger faster rates. -- 指定播放速率，原始值为1.0，大快小慢
    void setRate(float newRate);

    /// Sets new tempo control value. Normal tempo = 1.0, smaller values
    /// represent slower tempo, larger faster tempo. -- 指定节拍，原始值为1.0，大快小慢
    void setTempo(float newTempo);

    
    // 在原速1.0基础上，按百分比做增量，取值(-50 .. +100 %)
    /// Sets new rate control value as a difference in percents compared
    /// to the original rate (-50 .. +100 %) -- 设置声音的速率
    void setRateChange(float newRate);

    /// Sets new tempo control value as a difference in percents compared
    /// to the original tempo (-50 .. +100 %) -- 变速不变调
    void setTempoChange(float newTempo);

    /// Sets new pitch control value. Original pitch = 1.0, smaller values
    /// represent lower pitches, larger values higher pitch. -- 指定音调值，原始值为1.0
    void setPitch(float newPitch);

    /// Sets pitch change in octaves（八度） compared to the original pitch
    /// (-1.00 .. +1.00) -- 在原音调基础上以八度音为单位进行调整，取值为[-1.00,+1.00]
    void setPitchOctaves(float newPitch);

    /// Sets pitch change in semi-tones（半音音高） compared to the original pitch
    /// (-12 .. +12) -- 设置声音的pitch --  在原音调基础上以半音为单位进行调整，取值为[-12,+12]
    void setPitchSemiTones(int newPitch);
    void setPitchSemiTones(float newPitch);

    /// Sets the number of channels, 1 = mono, 2 = stereo -- 设置声音的声道
    void setChannels(uint numChannels);

    /// Sets sample rate. -- 设置声音的采样频率
    void setSampleRate(uint srate);

    /// Flushes the last samples from the processing pipeline to the output.
    /// Clears also the internal processing buffers.
    //
    /// Note: This function is meant for extracting the last samples of a sound
    /// stream. This function may introduce additional blank samples in the end
    /// of the sound stream, and thus it's not recommended to call this function
    /// in the middle of a sound stream.
    // 冲出处理管道中的最后一组“残留”的数据，应在最后执行
    void flush();

    // 输入采样数据
    /// Adds 'numSamples' pcs of samples from the 'samples' memory position into
    /// the input of the object. Notice that sample rate _has_to_ be set before
    /// calling this function, otherwise throws a runtime_error exception.
    virtual void putSamples(
            const SAMPLETYPE *samples,  ///< Pointer to sample buffer.
            uint numSamples                         ///< Number of samples in buffer. Notice
                                                    ///< that in case of stereo-sound a single sample
                                                    ///< contains data for both channels.
            );

    /// Clears all the samples in the object's output and internal processing
    /// buffers.
    virtual void clear();

    /// Changes a setting controlling the processing system behaviour. See the
    /// 'SETTING_...' defines for available setting ID's.
    ///  >>> mSoundTouch.setSetting(SETTING_USE_QUICKSEEK, quick);
    /// \return 'TRUE' if the setting was succesfully changed
    BOOL setSetting(int settingId,   ///< Setting ID number. see SETTING_... defines.
                    int value        ///< New setting value.
                    );

    /// Reads a setting controlling the processing system behaviour. See the
    /// 'SETTING_...' defines for available setting ID's.
    ///
    /// \return the setting value.
    int getSetting(int settingId    ///< Setting ID number, see SETTING_... defines.
                   ) const;

    /// Returns number of samples currently unprocessed.
    virtual uint numUnprocessedSamples() const;


    /// Other handy functions that are implemented in the ancestor classes (see
    /// classes 'FIFOProcessor' and 'FIFOSamplePipe')
    ///
    /// receiveSamples 输出处理后的数据，需要循环执行
    /// - receiveSamples() : Use this function to receive 'ready' processed samples from SoundTouch.
    /// - numSamples()     : Get number of 'ready' samples that can be received with
    ///                      function 'receiveSamples()'
    /// - isEmpty()        : Returns nonzero if there aren't any 'ready' samples.
    /// - clear()          : Clears all samples from ready/processing buffers.
};

}
#endif
/* VCSDKCore_hpp */



