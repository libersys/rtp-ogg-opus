#include <napi.h>
#include <iostream>

#include "rtp_encoder.h"
#include "helpers.h"

Napi::FunctionReference RtpEncoder::constructor;

Napi::Object RtpEncoder::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "RtpEncoder", {InstanceMethod("transform", &RtpEncoder::Transform)});

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("RtpEncoder", func);
    return exports;
}

// Constructor
RtpEncoder::RtpEncoder(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RtpEncoder>(info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1)
    {
        throw Napi::Error::New(env, "Payload type and samples per block expected.");
    }
    if (!info[0].IsNumber())
    {
        throw Napi::Error::New(env, "Payload type must be a number.");
    }
    if (!info[1].IsNumber())
    {
        throw Napi::Error::New(env, "Samples per block must be a number.");
    }

    if (info.Length() > 2)
    {
        if (!info[2].IsBoolean())
        {
            throw Napi::Error::New(env, "objectMode must be a boolean.");
        }
        this->objectMode = info[2].As<Napi::Boolean>();
    }

    this->rtp.version = 2;
    this->rtp.payload_type = info[0].As<Napi::Number>();
    this->rtp.pad = 0;
    this->rtp.ext = 0;
    this->rtp.cc = 0;
    this->rtp.mark = 0;
    this->rtp.seq = rand();
    this->rtp.time = rand();
    this->rtp.ssrc = rand();
    this->rtp.csrc = NULL;
    this->rtp.header_size = 0;
    this->rtp.payload_size = 0;

    this->samples = info[1].As<Napi::Number>();
}

// Destructor
RtpEncoder::~RtpEncoder()
{
    // NOOP
}

void RtpEncoder::Transform(const Napi::CallbackInfo &info)
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

    unsigned char *payload = chunk.Data();
    size_t payloadSize = chunk.Length();

    if (payloadSize > 0)
    {

        /* update the rtp header and send */
        this->rtp.header_size = 12 + 4 * this->rtp.cc;
        this->rtp.seq++;
        this->rtp.time += this->samples;
        this->rtp.payload_size = payloadSize;

        // Create RTP Packet
        unsigned char *packet;
        size_t packetSize = this->rtp.header_size + this->rtp.payload_size;
        packet = (unsigned char *)malloc(packetSize);
        if (!packet)
            throw Napi::Error::New(info.Env(), "Couldn't allocate packet buffer.");

        // Serialize header and copy to packet. Then copy payload to packet.
        serialize_rtp_header(packet, this->rtp.header_size, &this->rtp);
        memcpy(packet + this->rtp.header_size, payload, payloadSize);

        Napi::Buffer<unsigned char> output = Napi::Buffer<unsigned char>::Copy(env, reinterpret_cast<unsigned char *>(packet), packetSize);

        if (this->objectMode)
        {
            Napi::Object retObj = Napi::Object::New(env);

            retObj.Set("version", this->rtp.version);
            retObj.Set("payloadType", this->rtp.payload_type);
            retObj.Set("pad", this->rtp.pad);
            retObj.Set("ext", this->rtp.ext);
            retObj.Set("cc", this->rtp.cc);
            retObj.Set("mark", this->rtp.mark);
            retObj.Set("seq", this->rtp.seq);
            retObj.Set("time", this->rtp.time);
            retObj.Set("ssrc", this->rtp.ssrc);
            retObj.Set("headerSize", this->rtp.header_size);
            retObj.Set("packet", output);

            callback.Call({env.Null(), retObj});
        }
        else
        {
            callback.Call({env.Null(), output});
        }

        return;
    }

    callback.Call({});
}
