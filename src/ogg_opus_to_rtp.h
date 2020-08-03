#ifndef OGG_OPUS_TO_RTP_H
#define OGG_OPUS_TO_RTP_H

#include <napi.h>
#include "helpers.h"
#include "../deps/libopus/include/opus.h"
#include "../deps/libogg/include/ogg/ogg.h"

class OggOpusToRtp : public Napi::ObjectWrap<OggOpusToRtp>
{

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    OggOpusToRtp(const Napi::CallbackInfo &info);
    ~OggOpusToRtp();

    void Transform(const Napi::CallbackInfo &info);

private:
    static Napi::FunctionReference constructor;
    bool objectMode = false;

    int sampleRate = 48000;
    rtp_header rtp;

    ogg_sync_state ostate;
    ogg_stream_state ostream;
    int headers = 0;
    int serialno = 0;

    int pending = 0;
};

#endif