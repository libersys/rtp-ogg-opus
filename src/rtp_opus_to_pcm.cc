#include <napi.h>
#include <iostream>

#include "rtp_opus_to_pcm.h"
#include "helpers.h"

const char *getDecodeError(int decodedSamples)
{
    switch (decodedSamples)
    {
    case OPUS_BAD_ARG:
        return "One or more invalid/out of range arguments";
    case OPUS_BUFFER_TOO_SMALL:
        return "The mode struct passed is invalid";
    case OPUS_INTERNAL_ERROR:
        return "An internal error was detected";
    case OPUS_INVALID_PACKET:
        return "The compressed data passed is corrupted";
    case OPUS_UNIMPLEMENTED:
        return "Invalid/unsupported request number";
    case OPUS_INVALID_STATE:
        return "An encoder or decoder structure is invalid or already freed.";
    case OPUS_ALLOC_FAIL:
        return "Memory allocation has failed";
    default:
        return "Unknown OPUS error";
    }
}

Napi::FunctionReference RtpOpusToPcm::constructor;

Napi::Object RtpOpusToPcm::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "RtpOpusToPcm", {
                                                               InstanceMethod("transform", &RtpOpusToPcm::Transform),
                                                               InstanceAccessor<&RtpOpusToPcm::GetChannels>("channels"),
                                                               InstanceAccessor<&RtpOpusToPcm::GetSampleRate>("sampleRate"),
                                                           });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("RtpOpusToPcm", func);
    return exports;
}

Napi::Value RtpOpusToPcm::GetChannels(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->channels);
}

Napi::Value RtpOpusToPcm::GetSampleRate(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->sampleRate);
}

// Constructor
RtpOpusToPcm::RtpOpusToPcm(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RtpOpusToPcm>(info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 2)
    {
        throw Napi::Error::New(env, "Sample rate and channels expected.");
    }
    if (!info[0].IsNumber())
    {
        throw Napi::Error::New(env, "Sample rate must be a number.");
    }
    if (!info[1].IsNumber())
    {
        throw Napi::Error::New(env, "Channels must be a number.");
    }

    this->sampleRate = info[0].As<Napi::Number>();
    this->channels = info[1].As<Napi::Number>();

    int error;
    this->decoder = opus_decoder_create(this->sampleRate, this->channels, &error);
    if (error != OPUS_OK)
    {
        Napi::Error::New(env, "Could not create decoder. Check the decoder parameters").ThrowAsJavaScriptException();
    }

    this->outPcm = new opus_int16[this->channels * MAX_FRAME_SIZE];
}

// Destructor
RtpOpusToPcm::~RtpOpusToPcm()
{
    if (this->decoder)
        opus_decoder_destroy(this->decoder);

    this->decoder = nullptr;

    delete this->outPcm;
    this->outPcm = nullptr;
}

void RtpOpusToPcm::Transform(const Napi::CallbackInfo &info)
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
    unsigned char *input = chunk.Data();
    size_t inputSize = chunk.Length();

    Napi::Function callback = info[1].As<Napi::Function>();

    rtp_header rtp;

    if (parse_rtp_header(input, inputSize, &rtp) == 0)
    {
        input += rtp.header_size;
        inputSize -= rtp.header_size;

        /* skip header packets */
        if (inputSize >= 19 && !memcmp(input, "OpusHead", 8))
        {
            callback.Call({});
            return;
        }
        if (inputSize >= 16 && !memcmp(input, "OpusTags", 8))
        {
            callback.Call({});
            return;
        }

        int decodedSamples = opus_decode(
            this->decoder,
            input,
            inputSize,
            &(this->outPcm[0]),
            MAX_FRAME_SIZE,
            /* decode_fec */ 0);

        if (decodedSamples < 0)
        {
            Napi::TypeError::New(env, getDecodeError(decodedSamples)).ThrowAsJavaScriptException();
        }

        int decodedLength = decodedSamples * 2 * this->channels;

        Napi::Buffer<char> output = Napi::Buffer<char>::Copy(env, reinterpret_cast<char *>(this->outPcm), decodedLength);

        if (!output.IsEmpty())
        {
            callback.Call({env.Null(), output});
            return;
        }
    }

    callback.Call({});
}
