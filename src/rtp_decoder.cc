#include <napi.h>
#include <iostream>

#include "rtp_decoder.h"
#include "helpers.h"

Napi::FunctionReference RtpDecoder::constructor;

Napi::Object RtpDecoder::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "RtpDecoder", {InstanceMethod("transform", &RtpDecoder::Transform)});

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("RtpDecoder", func);
    return exports;
}

// Constructor
RtpDecoder::RtpDecoder(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RtpDecoder>(info)
{
    Napi::Env env = info.Env();

    if (info.Length() > 0)
    {
        if (!info[0].IsBoolean())
        {
            throw Napi::Error::New(env, "objectMode must be a boolean.");
        }
        this->objectMode = info[0].As<Napi::Boolean>();
    }
}

// Destructor
RtpDecoder::~RtpDecoder()
{
    // NOOP
}

void RtpDecoder::Transform(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 2)
    {
        throw Napi::Error::New(env, "chunk and callback arguments expected.");
    }
    if (!info[0].IsBuffer())
    {
        throw Napi::Error::New(env, "chunk must be a buffer.");
    }
    if (!info[1].IsFunction())
    {
        throw Napi::Error::New(env, "callback must be a function.");
    }

    Napi::Buffer<unsigned char> chunk = info[0].As<Napi::Buffer<unsigned char>>();
    Napi::Function callback = info[1].As<Napi::Function>();

    unsigned char *packet = chunk.Data();
    size_t packetSize = chunk.Length();

    rtp_header rtp;

    if (parse_rtp_header(packet, packetSize, &rtp) == 0)
    {

        packet += rtp.header_size;
        packetSize -= rtp.header_size;

        Napi::Buffer<unsigned char> payload = Napi::Buffer<unsigned char>::Copy(env, reinterpret_cast<unsigned char *>(packet), packetSize);

        if (this->objectMode)
        {
            Napi::Object retObj = Napi::Object::New(env);

            retObj.Set("version", rtp.version);
            retObj.Set("payloadType", rtp.payload_type);
            retObj.Set("pad", rtp.pad);
            retObj.Set("ext", rtp.ext);
            retObj.Set("cc", rtp.cc);
            retObj.Set("mark", rtp.mark);
            retObj.Set("seq", rtp.seq);
            retObj.Set("time", rtp.time);
            retObj.Set("ssrc", rtp.ssrc);
            retObj.Set("headerSize", rtp.header_size);
            retObj.Set("payload", payload);

            callback.Call({env.Null(), retObj});
        }
        else
        {
            callback.Call({env.Null(), payload});
        }

        return;
    }

    callback.Call({});
}
