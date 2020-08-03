#ifndef RTP_OPUS_TO_PCM_H
#define RTP_OPUS_TO_PCM_H

#define FRAME_SIZE 960
#define MAX_FRAME_SIZE 6 * 960
#define MAX_PACKET_SIZE 3 * 1276
#define BITRATE 64000

#include <napi.h>
#include "helpers.h"
#include "../deps/libopus/include/opus.h"

class RtpOpusToPcm : public Napi::ObjectWrap<RtpOpusToPcm>
{

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    RtpOpusToPcm(const Napi::CallbackInfo &info);
    ~RtpOpusToPcm();

    void Transform(const Napi::CallbackInfo &info);

private:
    static Napi::FunctionReference constructor;

    OpusDecoder *decoder;
    opus_int16 *outPcm;

    int channels = 2;
    int sampleRate = 48000;

    Napi::Value GetChannels(const Napi::CallbackInfo &info);
    Napi::Value GetSampleRate(const Napi::CallbackInfo &info);
};

#endif