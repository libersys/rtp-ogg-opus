#ifndef RTP_DECODER_H
#define RTP_DECODER_H

#include <napi.h>

class RtpDecoder : public Napi::ObjectWrap<RtpDecoder>
{

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    RtpDecoder(const Napi::CallbackInfo &info);
    ~RtpDecoder();

    void Transform(const Napi::CallbackInfo &info);

private:
    static Napi::FunctionReference constructor;
    bool objectMode = false;

    Napi::Object self;
    Napi::Function push;
};

#endif