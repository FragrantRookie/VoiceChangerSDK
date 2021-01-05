////  VCSDKCore.cpp
//  VoiceChanger
//
//  Created on 2020/12/11.
//  
//

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>


#include "VCSDKCore.h"
#include "VCSDKCoreTDStretch.hpp"
#include "VCSDKCoreRateTransposer.hpp"
#include "VCSDKCoreCpu_detect.h"

using namespace vcsdkcore;
    
/// test if two floating point numbers are equal
#define TEST_FLOAT_EQUAL(a, b)  (fabs(a - b) < 1e-10)


/// Print library version string for autoconf
extern "C" void soundtouch_ac_test()
{
    printf("SoundTouch Version: %s\n",KVOICECHANGERSDK_VERSION);
}


VCSDKCore::VCSDKCore()
{
    // Initialize rate transposer and tempo changer instances

    pRateTransposer = new VCSDKCoreRateTransposer();
    pTDStretch = VCSDKCoreTDStretch::newInstance();

    setOutPipe(pTDStretch);

    rate = tempo = 0;

    virtualPitch =
    virtualRate =
    virtualTempo = 1.0;

    calcEffectiveRateAndTempo();

    channels = 0;
    bSrateSet = FALSE;
}



VCSDKCore::~VCSDKCore()
{
    delete pRateTransposer;
    delete pTDStretch;
}



/// Get SoundTouch library version string
const char *VCSDKCore::getVersionString()
{
    static const char *_version = KVOICECHANGERSDK_VERSION;

    return _version;
}


/// Get SoundTouch library version Id
uint VCSDKCore::getVersionId()
{
    return KVOICECHANGERSDK_VERSION_ID;
}


// Sets the number of channels, 1 = mono, 2 = stereo
void VCSDKCore::setChannels(uint numChannels)
{
    /*if (numChannels != 1 && numChannels != 2)
    {
        //ST_THROW_RT_ERROR("Illegal number of channels");
        return;
    }*/
    channels = numChannels;
    pRateTransposer->setChannels((int)numChannels);
    pTDStretch->setChannels((int)numChannels);
}



// Sets new rate control value. Normal rate = 1.0, smaller values
// represent slower rate, larger faster rates.
void VCSDKCore::setRate(float newRate)
{
    virtualRate = newRate;
    calcEffectiveRateAndTempo();
}



// Sets new rate control value as a difference in percents compared
// to the original rate (-50 .. +100 %)
void VCSDKCore::setRateChange(float newRate)
{
    virtualRate = 1.0f + 0.01f * newRate;
    calcEffectiveRateAndTempo();
}



// Sets new tempo control value. Normal tempo = 1.0, smaller values
// represent slower tempo, larger faster tempo.
void VCSDKCore::setTempo(float newTempo)
{
    virtualTempo = newTempo;
    calcEffectiveRateAndTempo();
}



// Sets new tempo control value as a difference in percents compared
// to the original tempo (-50 .. +100 %)
void VCSDKCore::setTempoChange(float newTempo)
{
    virtualTempo = 1.0f + 0.01f * newTempo;
    calcEffectiveRateAndTempo();
}



// Sets new pitch control value. Original pitch = 1.0, smaller values
// represent lower pitches, larger values higher pitch.
void VCSDKCore::setPitch(float newPitch)
{
    virtualPitch = newPitch;
    calcEffectiveRateAndTempo();
}



// Sets pitch change in octaves compared to the original pitch
// (-1.00 .. +1.00)
void VCSDKCore::setPitchOctaves(float newPitch)
{
    virtualPitch = (float)exp(0.69314718056f * newPitch);
    calcEffectiveRateAndTempo();
}



// Sets pitch change in semi-tones compared to the original pitch
// (-12 .. +12)
void VCSDKCore::setPitchSemiTones(int newPitch)
{
    setPitchOctaves((float)newPitch / 12.0f);
}



void VCSDKCore::setPitchSemiTones(float newPitch)
{
    setPitchOctaves(newPitch / 12.0f);
}


// Calculates 'effective' rate and tempo values from the
// nominal control values.
void VCSDKCore::calcEffectiveRateAndTempo()
{
    float oldTempo = tempo;
    float oldRate = rate;

    tempo = virtualTempo / virtualPitch;
    rate = virtualPitch * virtualRate;

    if (!TEST_FLOAT_EQUAL(rate,oldRate)) pRateTransposer->setRate(rate);
    if (!TEST_FLOAT_EQUAL(tempo, oldTempo)) pTDStretch->setTempo(tempo);

#ifndef SOUNDTOUCH_PREVENT_CLICK_AT_RATE_CROSSOVER
    if (rate <= 1.0f)
    {
        if (output != pTDStretch)
        {
            VCSDKCoreFIFOSamplePipe *tempoOut;

            assert(output == pRateTransposer);
            // move samples in the current output buffer to the output of pTDStretch
            tempoOut = pTDStretch->getOutput();
            tempoOut->moveSamples(*output);
            // move samples in pitch transposer's store buffer to tempo changer's input
            // deprecated : pTDStretch->moveSamples(*pRateTransposer->getStore());

            output = pTDStretch;
        }
    }
    else
#endif
    {
        if (output != pRateTransposer)
        {
            VCSDKCoreFIFOSamplePipe *transOut;

            assert(output == pTDStretch);
            // move samples in the current output buffer to the output of pRateTransposer
            transOut = pRateTransposer->getOutput();
            transOut->moveSamples(*output);
            // move samples in tempo changer's input to pitch transposer's input
            pRateTransposer->moveSamples(*pTDStretch->getInput());

            output = pRateTransposer;
        }
    }
}


// Sets sample rate.
void VCSDKCore::setSampleRate(uint srate)
{
    bSrateSet = TRUE;
    // set sample rate, leave other tempo changer parameters as they are.
    pTDStretch->setParameters((int)srate);
}


// Adds 'numSamples' pcs of samples from the 'samples' memory position into
// the input of the object.
void VCSDKCore::putSamples(const SAMPLETYPE *samples, uint nSamples)
{
    if (bSrateSet == FALSE)
    {
        ST_THROW_RT_ERROR("SoundTouch : Sample rate not defined");
    }
    else if (channels == 0)
    {
        ST_THROW_RT_ERROR("SoundTouch : Number of channels not defined");
    }

    // Transpose the rate of the new samples if necessary
    /* Bypass the nominal setting - can introduce a click in sound when tempo/pitch control crosses the nominal value...
    if (rate == 1.0f)
    {
        // The rate value is same as the original, simply evaluate the tempo changer.
        assert(output == pTDStretch);
        if (pRateTransposer->isEmpty() == 0)
        {
            // yet flush the last samples in the pitch transposer buffer
            // (may happen if 'rate' changes from a non-zero value to zero)
            pTDStretch->moveSamples(*pRateTransposer);
        }
        pTDStretch->putSamples(samples, nSamples);
    }
    */
#ifndef SOUNDTOUCH_PREVENT_CLICK_AT_RATE_CROSSOVER
    else if (rate <= 1.0f)
    {
        // transpose the rate down, output the transposed sound to tempo changer buffer
        assert(output == pTDStretch);
        pRateTransposer->putSamples(samples, nSamples);
        pTDStretch->moveSamples(*pRateTransposer);
    }
    else
#endif
    {
        // evaluate the tempo changer, then transpose the rate up,
        assert(output == pRateTransposer);
        pTDStretch->putSamples(samples, nSamples);
        pRateTransposer->moveSamples(*pTDStretch);
    }
}


// Flushes the last samples from the processing pipeline to the output.
// Clears also the internal processing buffers.
//
// Note: This function is meant for extracting the last samples of a sound
// stream. This function may introduce additional blank samples in the end
// of the sound stream, and thus it's not recommended to call this function
// in the middle of a sound stream.
void VCSDKCore::flush()
{
    int i;
    int nUnprocessed;
    int nOut;
    SAMPLETYPE *buff=(SAMPLETYPE*)alloca(64*channels*sizeof(SAMPLETYPE));

    // check how many samples still await processing, and scale
    // that by tempo & rate to get expected output sample count
    nUnprocessed = numUnprocessedSamples();
    nUnprocessed = (int)((double)nUnprocessed / (tempo * rate) + 0.5);

    nOut = numSamples();        // ready samples currently in buffer ...
    nOut += nUnprocessed;       // ... and how many we expect there to be in the end
    
    memset(buff, 0, 64 * channels * sizeof(SAMPLETYPE));
    // "Push" the last active samples out from the processing pipeline by
    // feeding blank samples into the processing pipeline until new,
    // processed samples appear in the output (not however, more than
    // 8ksamples in any case)
    for (i = 0; i < 128; i ++)
    {
        putSamples(buff, 64);
        if ((int)numSamples() >= nOut)
        {
            // Enough new samples have appeared into the output!
            // As samples come from processing with bigger chunks, now truncate it
            // back to maximum "nOut" samples to improve duration accuracy
            adjustAmountOfSamples(nOut);

            // finish
            break;
        }
    }

    // Clear working buffers
    pRateTransposer->clear();
    pTDStretch->clearInput();
    // yet leave the 'tempoChanger' output intouched as that's where the
    // flushed samples are!
}


// Changes a setting controlling the processing system behaviour. See the
// 'SETTING_...' defines for available setting ID's.
BOOL VCSDKCore::setSetting(int settingId, int value)
{
    int sampleRate, sequenceMs, seekWindowMs, overlapMs;

    // read current tdstretch routine parameters
    pTDStretch->getParameters(&sampleRate, &sequenceMs, &seekWindowMs, &overlapMs);

    switch (settingId)
    {
        case SETTING_USE_AA_FILTER :
            // enables / disabless anti-alias filter -- 抗混叠滤波器
            pRateTransposer->enableAAFilter((value != 0) ? TRUE : FALSE);
            return TRUE;

        case SETTING_AA_FILTER_LENGTH :
            // sets anti-alias filter length -- 抗混叠滤波器长度
            pRateTransposer->getAAFilter()->setLength(value);
            return TRUE;

        case SETTING_USE_QUICKSEEK :
            // enables / disables tempo routine quick seeking algorithm -- 节奏常规快速搜索算法
            pTDStretch->enableQuickSeek((value != 0) ? TRUE : FALSE);
            return TRUE;

        case SETTING_SEQUENCE_MS:
            // change time-stretch sequence duration parameter -- 更改时间拉伸序列持续时间参数
            pTDStretch->setParameters(sampleRate, value, seekWindowMs, overlapMs);
            return TRUE;

        case SETTING_SEEKWINDOW_MS:
            // change time-stretch seek window length parameter -- 更改时间拉伸搜索窗口长度参数
            pTDStretch->setParameters(sampleRate, sequenceMs, value, overlapMs);
            return TRUE;

        case SETTING_OVERLAP_MS:
            // change time-stretch overlap length parameter -- 更改时间拉伸重叠长度参数
            pTDStretch->setParameters(sampleRate, sequenceMs, seekWindowMs, value);
            return TRUE;

        default :
            return FALSE;
    }
}


// Reads a setting controlling the processing system behaviour. See the
// 'SETTING_...' defines for available setting ID's.
//
// Returns the setting value.
int VCSDKCore::getSetting(int settingId) const
{
    int temp;

    switch (settingId)
    {
        case SETTING_USE_AA_FILTER :
            return (uint)pRateTransposer->isAAFilterEnabled();

        case SETTING_AA_FILTER_LENGTH :
            return pRateTransposer->getAAFilter()->getLength();

        case SETTING_USE_QUICKSEEK :
            return (uint)   pTDStretch->isQuickSeekEnabled();

        case SETTING_SEQUENCE_MS:
            pTDStretch->getParameters(NULL, &temp, NULL, NULL);
            return temp;

        case SETTING_SEEKWINDOW_MS:
            pTDStretch->getParameters(NULL, NULL, &temp, NULL);
            return temp;

        case SETTING_OVERLAP_MS:
            pTDStretch->getParameters(NULL, NULL, NULL, &temp);
            return temp;

        case SETTING_NOMINAL_INPUT_SEQUENCE :
            return pTDStretch->getInputSampleReq();

        case SETTING_NOMINAL_OUTPUT_SEQUENCE :
            return pTDStretch->getOutputBatchSize();

        default :
            return 0;
    }
}


// Clears all the samples in the object's output and internal processing
// buffers.
void VCSDKCore::clear()
{
    pRateTransposer->clear();
    pTDStretch->clear();
}



/// Returns number of samples currently unprocessed.
uint VCSDKCore::numUnprocessedSamples() const
{
    VCSDKCoreFIFOSamplePipe * psp;
    if (pTDStretch)
    {
        psp = pTDStretch->getInput();
        if (psp)
        {
            return psp->numSamples();
        }
    }
    return 0;
}











