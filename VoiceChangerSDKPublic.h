////  VoiceChangerSDKPublic.hpp
//  VoiceChangerCFramework
//
//  Created on 2021/1/4.
//  
//

/** func: achieve Voice Changer SDK public method.
 *
 *  内部封装各个变声功能实现
 *
 *
 */


#ifndef VOICECHANGERSDKPUBLIC_H // VoiceChangerSDKPublic_h
#define VOICECHANGERSDKPUBLIC_H // VoiceChangerSDKPublic_h


#include "VCSDKCore.h"


class VoiceChangerSDKPublic {
    
    
private:
    class  vcsdkcore::VCSDKCore *_vcsdkCore;
    
    

public:
    
    VoiceChangerSDKPublic();
    virtual ~VoiceChangerSDKPublic();
    
    
    // 1.  实时SampleBuffer==>数据处理，
    void putSamples(const short *samples,uint length);
    void receiveSample(short *buffer,uint length);

    
    // 2. 读取文件read file to get SampleBuffer
    void readFileToVoiceChanger(char *originAudioPath,char *outAudioPath);
    
    
// 变声种类选择
    // 1. 萝莉音
    void luoliSound();
    
    // 2.  大叔音
    void uncleSound();
    
    // 3. 搞怪音
    void funnySound();
    
    // 4. 小黄人音
    void littleYellowSound();
    
    
    // 5. 机器人音效 -- 机器人的音效是一个组合效果，，音调比较高，而且有重音.音调延迟
    
    
    // 6. 惊悚音
    
    
};





#endif /* VoiceChangerSDKPublic_hpp */



