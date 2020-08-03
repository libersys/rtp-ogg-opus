#ifndef RTP_ENCODER_H
#define RTP_ENCODER_H

#include <napi.h>
#include "helpers.h"

class RtpEncoder : public Napi::ObjectWrap<RtpEncoder>
{

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    RtpEncoder(const Napi::CallbackInfo &info);
    ~RtpEncoder();

    void Transform(const Napi::CallbackInfo &info);

private:
    static Napi::FunctionReference constructor;

    bool objectMode = false;

    int samples;
    rtp_header rtp;
};

#endif