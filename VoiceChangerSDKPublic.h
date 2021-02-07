////  VoiceChangerSDKPublic.h
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
    
    // 变声种类选择
    // 1. 萝莉音
    void luoliSound();
    
    // 2.  大叔音
    void uncleSound();
    
    // 3. 搞怪音
    void funnySound();
    
    // 4. 小黄人音
    void littleYellowSound();
    
    // 5. 慢吞吞
    void slowlySound();
    
    // 6. 怪兽
    void monsterSound();
    
    // 7. 重机械
    void heavyMachinery();
    
    // 8. 快速说
    void quicklySaySound();

    
    // 读取文件read file to get SampleBuffer
    bool readFileToVoiceChanger(char *originAudioPath,char *outAudioPath);
    
    
    // update version -- 每个变声种类功能
    
    
    
    
};





#endif /* VoiceChangerSDKPublic_hpp */



