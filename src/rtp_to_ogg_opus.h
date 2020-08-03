#ifndef RTP_TO_OGG_OPUS_H
#define RTP_TO_OGG_OPUS_H

#include <napi.h>
#include "helpers.h"
// #include "speex_resampler.h"
#include "../deps/libopus/include/opus.h"
#include "../deps/libogg/include/ogg/ogg.h"

class RtpToOggOpus : public Napi::ObjectWrap<RtpToOggOpus>
{

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    RtpToOggOpus(const Napi::CallbackInfo &info);
    ~RtpToOggOpus();

    void Transform(const Napi::CallbackInfo &info);

private:
    static Napi::FunctionReference constructor;

    bool objectMode = false;

    int channels = 2;
    int sampleRate = 48000;

    int pages = 0;
    int headerPages = 0;
    bool pending = false;

    ogg_stream_state *stream;
    ogg_packet *head;
    ogg_packet *tags;

    ogg_int64_t granulepos;

    unsigned char *buffer;
    size_t bufferSize;
    size_t bufferOffset;

    // void SetChannels(const Napi::CallbackInfo &info, const Napi::Value &value);
    Napi::Value GetChannels(const Napi::CallbackInfo &info);

    // void SetSampleRate(const Napi::CallbackInfo &info, const Napi::Value &value);
    Napi::Value GetSampleRate(const Napi::CallbackInfo &info);
};

#endif