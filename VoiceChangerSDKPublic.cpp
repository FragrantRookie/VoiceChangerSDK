////  VoiceChangerSDKPublic.cpp
//  VoiceChangerCFramework
//
//  Created on 2021/1/4.
//  
//

#include "VoiceChangerSDKPublic.h"


#define DR_WAV_IMPLEMENTATION // 必须加上下面这个宏定义，否则会报错

#include "dr_wav.h"



VoiceChangerSDKPublic::VoiceChangerSDKPublic() {
    
    _vcsdkCore = new vcsdkcore::VCSDKCore();

    //
    //_vcsdkCore->setSampleRate(8000); //
    _vcsdkCore->setSampleRate(44100);
    _vcsdkCore->setChannels(1);



    // set default
    _vcsdkCore->setSetting(SETTING_SEQUENCE_MS, 40); // 帧长
    _vcsdkCore->setSetting(SETTING_SEEKWINDOW_MS, 15); // 叠加的时候寻找窗的范围长度（ms）
    _vcsdkCore->setSetting(SETTING_OVERLAP_MS, 8);  //  叠加范围（ms），样例中用的是8

    
}

VoiceChangerSDKPublic::~VoiceChangerSDKPublic() {
    
    
}


/** 1.  萝莉音
 *
 * 提高8个音调(女声)
 *
 */
void VoiceChangerSDKPublic::luoliSound() {
    _vcsdkCore->setPitchSemiTones(8);
    _vcsdkCore->setTempo(1);
    _vcsdkCore->setRate(1);
}


/** 2. 大叔音
 * 降低音调到0.8
 *
 */
void VoiceChangerSDKPublic::uncleSound() {
    _vcsdkCore->setPitch(0.8);
    _vcsdkCore->setTempo(1);
    _vcsdkCore->setRate(1);
}

/** 3. 搞怪音
 *  提高语速x2
 *
 */
void VoiceChangerSDKPublic::funnySound() {
    _vcsdkCore->setPitchSemiTones(0);
    _vcsdkCore->setTempo(1);
    _vcsdkCore->setRate(2);
}

    
/** 4. 小黄人音
 *
 */
void VoiceChangerSDKPublic::littleYellowSound() {
    _vcsdkCore->setPitchSemiTones(8);
    _vcsdkCore->setTempo(1);
    _vcsdkCore->setRateChange(120);
}


// --- --- ---
/** 1. 实时输入CSSampleBuffer数据处理
 * putSamples输入,receiveSamples输出处理后的sampleBuffer。
 */
// 往ST中输入buffer
void VoiceChangerSDKPublic::putSamples(const short *samples,uint length) {
    _vcsdkCore->putSamples(samples, length);
}
// 处理完成的buffers从ST中输出
void VoiceChangerSDKPublic::receiveSample(short *buffer,uint length) {
    _vcsdkCore->receiveSamples(buffer,length);
}


/** 2. 读取文件get SampleBuffer
 *
 */

void wavWrite_s16(char *filename, int16_t *buffer, int sampleRate, uint32_t totalSampleCount) {
    drwav_data_format format;
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = 1;
    format.sampleRate = (drwav_uint32) sampleRate;
    format.bitsPerSample = 16;
    drwav *pWav = drwav_open_file_write(filename, &format);
    if (pWav) {
        drwav_uint64 samplesWritten = drwav_write(pWav, totalSampleCount, buffer);
        drwav_uninit(pWav);
        if (samplesWritten != totalSampleCount) {
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
    }
}

int16_t *wavRead_s16(char *filename, uint32_t *sampleRate, uint64_t *totalSampleCount, uint32_t *channels) {
    int16_t *buffer = drwav_open_and_read_file_s16(filename, channels, sampleRate, totalSampleCount);
    if (buffer == NULL) {
        printf("ERROR.");
    }
    if (*channels == 2 && buffer != NULL) {
        int16_t *bufferSave = buffer;
        for (uint64_t i = 0; i < *totalSampleCount; i += 2) {
            *bufferSave++ = ((buffer[i] + buffer[i + 1]) >> 1);
        }
        *totalSampleCount = *totalSampleCount >> 1;
    } else if (*channels != 1) {
        drwav_free(buffer);
        buffer = NULL;
        *sampleRate = 0;
        *totalSampleCount = 0;
    }
    return buffer;
}


void VoiceChangerSDKPublic::readFileToVoiceChanger(char *originAudioPath,char *outAudioPath) {
    
    // ** 需要判断outAudioPath是否已经存在，存在则删除 ** 
    FILE *file = fopen(outAudioPath, "r");
    if (file != NULL) {
        // 文件已经存在
        // 移除文件
        remove(outAudioPath);
    }
    
    uint32_t sampleRate = 0;
    uint64_t totalSampleCount = 0;
    uint32_t channels = 0;
    short *data_in = wavRead_s16(originAudioPath, &sampleRate, &totalSampleCount, &channels);
    
    _vcsdkCore->putSamples(data_in, totalSampleCount);
    
    
    short *samples = new short[totalSampleCount];
    
    // 此处是一个cycle，不需要等待其他回调
    // 在变声文件完成后，会自动走出循环
    // 若不想阻塞主线程，可在外部自己开设其他线程处理即可。
    int numSamples = 0;
    do {
        //
        numSamples = _vcsdkCore->receiveSamples(samples,totalSampleCount);
        

    } while (numSamples>0);
    
    wavWrite_s16(outAudioPath, samples, sampleRate, totalSampleCount);
    
    
    delete [] samples;
    
}


void splitpath(const char *path, char *drv, char *dir, char *name, char *ext) {
    const char *end;
    const char *p;
    const char *s;
    if (path[0] && path[1] == ':') {
        if (drv) {
            *drv++ = *path++;
            *drv++ = *path++;
            *drv = '\0';
        }
    } else if (drv)
        *drv = '\0';
    for (end = path; *end && *end != ':';)
        end++;
    for (p = end; p > path && *--p != '\\' && *p != '/';)
        if (*p == '.') {
            end = p;
            break;
        }
    if (ext)
        for (s = end; (*ext = *s++);)
            ext++;
    for (p = end; p > path;)
        if (*--p == '\\' || *p == '/') {
            p++;
            break;
        }
    if (name) {
        for (s = p; s < end;)
            *name++ = *s++;
        *name = '\0';
    }
    if (dir) {
        for (s = path; s < p;)
            *dir++ = *s++;
        *dir = '\0';
    }
}

/*
void PitchShiftMain(char *filepath) {
    
    uint32_t sampleRate = 0;
    uint64_t totalSampleCount = 0;
    uint32_t channels = 0;
    short *data_in = wavRead_s16(filepath, &sampleRate, &totalSampleCount, &channels);
    if (data_in != NULL) {
        float pitchShift = 0.9f;
        size_t ms = 50;
        size_t frameSize = sampleRate * ms / 1000; // 44100 * 50 /1000 = 2205
        frameSize += frameSize % 2;
        planData pitchPlanData = {0};
        double startTime = now();
        makePlanData(frameSize, sampleRate, &pitchPlanData);
        pitchshift(pitchShift, data_in, data_in, totalSampleCount, &pitchPlanData);
        // turn to minion pitch
        {
            totalSampleCount /= 2;
            short *samples = data_in;
            for (int i = 0; i < totalSampleCount; i++) {
                data_in[i] = samples[0];
                samples += 2;
            }
        }
        double time_interval = calcElapsed(startTime, now());
        freePlanData(&pitchPlanData);
        printf("time interval: %f ms\n ", (time_interval * 1000));
    }
    char drive[3];
    char dir[256];
    char fname[256];
    char ext[256];
    char out_file[1024];
    splitpath(in_file, drive, dir, fname, ext);
    sprintf(out_file, "%s%s%s_out%s", drive, dir, fname, ext);
    wavWrite_s16(out_file, data_in, sampleRate, totalSampleCount);
    if (data_in) {
        free(data_in);
    }
    printf("press any key to exit.\n");
    getchar();
     
}
*/

















