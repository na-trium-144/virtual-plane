#include <portaudio.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "audio.h"

// 44100Hz, 16bitにすること
#define SAMPLE_RATE 44100
#define BGM_NUM 7
#define SE_NUM 5
#define WAV_NUM (BGM_NUM + SE_NUM)
const char *wav_name[WAV_NUM] = {
    "./wav/bgm_0.wav", // 25
    "./wav/bgm_1.wav", // 12.5
    "./wav/bgm_2.wav", // 50
    "./wav/bgm_3.wav", // bass
    "./wav/bgm_4.wav",
    "./wav/bgm_5.wav",
    "./wav/bgm_6.wav",
    "./wav/vf_ready.wav",
    "./wav/vf_coin.wav",
    "./wav/vf_shoot.wav",
    "./wav/vf_hit.wav",
    "./wav/vf_yakan.wav",
};
int16_t *wav_bgm[WAV_NUM];
int16_t *wav_start[WAV_NUM], *wav_end[WAV_NUM], *wav_current[WAV_NUM];

int wav_flag[WAV_NUM];
int bgm_fade = 0;
#define BGM_FADE_T 1
double bgm_vol_current = 1;
double bgm_end_loop = 1; // タイトルではbgmの1ループが短い
int hidden_bgm2(){
    // se再生中はbgm2を止める
    for(int c = 0; c < SE_NUM; c++){
        if(wav_flag[c + BGM_NUM]){
            return 1;
        }
    }
    return 0;
}
void se_play(enum se c){
    wav_current[c + BGM_NUM] = wav_start[c + BGM_NUM];
    wav_flag[c + BGM_NUM] = 1;
}
void bgm_stop(){
    for(int c = 0; c < BGM_NUM; c++){
        wav_flag[c] = 0;
    }
    wav_flag[BGM_NUM + se_ready] = 0;
}
void bgm_change(enum GameState g){
    if(g == g_ready){
        se_play(se_ready);
    }
    bgm_fade = g == g_over;
    bgm_vol_current = 1;
    bgm_end_loop = g == g_title ? 1.0 / 6 : 1;
    for(int c = 0; c < BGM_NUM; c++){
        int f;
        switch(g){
        case g_title:
            f = c >= 3;
            break;
        case g_ready:
            f = 0;
            break;
        case g_main:
            f = 1;
            break;
        case g_over:
            f = c < 2;
            break;
        }
        if(f && !wav_flag[c]){
            wav_current[c] = wav_start[c];
        }
        wav_flag[c] = f;
    }
}

void init_wav(){
    for(int i = 0; i < WAV_NUM; i++){
        FILE *fp = fopen(wav_name[i], "rb");
        if(fp) {
            fseek(fp, 0, SEEK_END);
            size_t fsize = ftell(fp);
            rewind(fp);

            wav_bgm[i] = (int16_t*) malloc(sizeof(char) * fsize);
            fread((void*)wav_bgm[i], 1, fsize, fp);

            fclose(fp);

            for(uint8_t *p = (uint8_t*)wav_bgm[i]; p < (uint8_t*)wav_bgm[i] + fsize; p++){
                if(p[0] == 'd' && p[1] == 'a' && p[2] == 't' && p[3] == 'a'){
                    size_t data_len = p[4] | p[5] << 8 | p[6] << 16 | p[7] << 24;
                    wav_start[i] = (int16_t*)(p + 8);
                    wav_current[i] = wav_start[i];
                    wav_end[i] = (int16_t*)(p + 8 + data_len);
                    printf("%s data len = %lu\n", wav_name[i], (unsigned long)data_len);
                }
            }
        }else{
            printf("Error %i from open %s: %s\n", errno, wav_name[i], strerror(errno));
            wav_bgm[i] = NULL;
        }
    }
}

static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    // paTestData *data = (paTestData*)userData; 
    int16_t *out = (int16_t*)outputBuffer;
    
    for(size_t i = 0; i < framesPerBuffer; i++ )
    {
        int32_t out_l = 0, out_r = 0;
        for(int c = 0; c < WAV_NUM; c++){
            if(wav_flag[c]){
                int16_t out_l1, out_r1;
                out_l1 = *(wav_current[c]++);
                out_r1 = *(wav_current[c]++);
                if(c < BGM_NUM){
                    out_l1 *= bgm_vol_current;
                    out_r1 *= bgm_vol_current;
                }
                if(!(c == 2 && hidden_bgm2())){
                    out_l += out_l1;
                    out_r += out_r1;
                }
                int16_t *end = wav_start[c] + (size_t)((wav_end[c] - wav_start[c]) * bgm_end_loop);
                if(wav_current[c] >= end){
                    wav_current[c] = wav_start[c];
                    if(c >= BGM_NUM){
                        // seは終わったら止める
                        wav_flag[c] = 0;
                    }
                }
            }
        }
        if(out_l > 0x7fff){
            out_l = 0x7fff;
        }
        if(out_l < -0x8000){
            out_l = -0x8000;
        }
        if(out_r > 0x7fff){
            out_r = 0x7fff;
        }
        if(out_r < -0x8000){
            out_r = -0x8000;
        }
        *(out++) = out_l;  /* left */
        *(out++) = out_r;  /* right */

    }
    if(bgm_fade){
        bgm_vol_current -= framesPerBuffer / ((double)BGM_FADE_T * SAMPLE_RATE);
        if(bgm_vol_current < 0){
            bgm_vol_current = 0;
        }
    }
    return 0;
}


PaStream *stream;
void init_pa(){
    PaError err;
    err = Pa_Initialize();
    if( err != paNoError ){
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        return;
    };
    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream( &stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paInt16,  /* 32 bit floating point output */
                                SAMPLE_RATE,
                                256,        /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
                                patestCallback, /* this is your callback function */
                                NULL ); /*This is a pointer that will be passed to
                                               your callback*/
    if( err != paNoError ){
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        return;
    };
    err = Pa_StartStream( stream );
    if( err != paNoError ){
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        return;
    }
}

#ifdef IS_MAIN
int main(){
    init_wav();
    init_pa();
    bgm_change(g_main);
    while(1);
}
#endif
