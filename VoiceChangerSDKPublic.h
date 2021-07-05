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
    
    /** 接入方法如下：
     *
     *  1、初始化变声管理类 "VoiceChangerSDKPublic"，全局变量。
     *  2、使用对象方法调用；下面方法;先进行相应的变声种类参数配置。
     *  3、传入原音频文件".wav"格式路径(pcm原始音频数据文件暂未兼容),及存储变声音频全路径需含有".wav"后缀名。
     *  4、此SDK实现以及方法使用，为串行，同步执行，不存在异步等待调用问题。
     *
     */
    
    
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



