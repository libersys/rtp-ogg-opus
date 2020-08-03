#include <napi.h>
#include <iostream>

#include "rtp_to_ogg_opus.h"
#include "helpers.h"

Napi::FunctionReference RtpToOggOpus::constructor;

Napi::Object RtpToOggOpus::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "RtpToOggOpus", {
                                                               InstanceMethod("transform", &RtpToOggOpus::Transform),
                                                               InstanceAccessor<&RtpToOggOpus::GetChannels>("channels"),
                                                               InstanceAccessor<&RtpToOggOpus::GetSampleRate>("sampleRate"),
                                                           });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("RtpToOggOpus", func);
    return exports;
}

Napi::Value RtpToOggOpus::GetChannels(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->channels);
}

Napi::Value RtpToOggOpus::GetSampleRate(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->sampleRate);
}

// Constructor
RtpToOggOpus::RtpToOggOpus(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RtpToOggOpus>(info)
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

    if (info.Length() > 2)
    {
        if (!info[2].IsBoolean())
        {
            throw Napi::Error::New(env, "objectMode must be a boolean.");
        }
        this->objectMode = info[2].As<Napi::Boolean>();
    }

    // TODO: What is the minimum size of an ogg bitstream?
    //       What buffer size is appropiate?
    this->bufferSize = 8192;
    this->buffer = (unsigned char *)malloc(this->bufferSize);

    this->bufferOffset = 0;

    // Allocate memory for ogg stream.
    this->stream = (ogg_stream_state *)malloc(sizeof(ogg_stream_state));

    if (ogg_stream_init(this->stream, rand()) < 0)
        throw Napi::Error::New(info.Env(), "Couldn't initialize the ogg_stream_state struct and allocates appropriate memory.");

    // Create ogg head and tags.
    this->head = op_opushead(this->sampleRate, this->channels);
    this->tags = op_opustags();

    // Write first time headers.
    ogg_stream_packetin(this->stream, this->head);
    ogg_stream_packetin(this->stream, this->tags);

    // Flush the header to be ready for the first packet.
    this->headerPages = ogg_flush(this->stream, this->buffer, this->bufferSize, this->bufferOffset);
    this->pending = true;
}

// Destructor
RtpToOggOpus::~RtpToOggOpus()
{
    // if (this->decoder)
    //     opus_decoder_destroy(this->decoder);

    // this->decoder = nullptr;

    // delete this->outPcm;
    // this->outPcm = nullptr;
}

void RtpToOggOpus::Transform(const Napi::CallbackInfo &info)
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
    unsigned char *packet = chunk.Data();
    size_t packetSize = chunk.Length();

    Napi::Function callback = info[1].As<Napi::Function>();

    rtp_header rtp;

    if (parse_rtp_header(packet, packetSize, &rtp) == 0)
    {
        packet += rtp.header_size;
        packetSize -= rtp.header_size;

        if (this->pending == false)
        {
            // Initialize ogg stream, only if there is not pending data.
            if (ogg_stream_reset_serialno(this->stream, rand()) != 0)
                throw Napi::Error::New(info.Env(), "Couldn't initialize the ogg_stream_state struct and allocates appropriate memory.");

            // Write headers
            ogg_stream_packetin(this->stream, this->head);
            ogg_stream_packetin(this->stream, this->tags);

            this->headerPages = 0;
            this->bufferOffset = 0;
            this->pages = 0;

            this->headerPages = ogg_flush(this->stream, this->buffer, this->bufferSize, this->bufferOffset);

            this->pending = true;
        }

        // Write packet to ogg stream.
        ogg_packet *body;
        body = op_from_pkt(packet, packetSize);
        body->packetno = rtp.seq;

        int samples;
        samples = opus_packet_get_nb_samples(packet, packetSize, this->sampleRate);
        if (samples > 0)
            this->granulepos += samples;

        body->granulepos = this->granulepos;
        ogg_stream_packetin(this->stream, body);

        // ogg_pageout returns the number of written pages.
        this->pages += ogg_pageout(this->stream, this->buffer, this->bufferSize, this->bufferOffset);

        if (this->pages > 0)
        {

            Napi::Buffer<unsigned char> container = Napi::Buffer<unsigned char>::Copy(env, reinterpret_cast<unsigned char *>(this->buffer), this->bufferOffset);

            if (this->objectMode)
            {
                Napi::Object retObj = Napi::Object::New(env);
                retObj.Set("channels", this->channels);
                retObj.Set("sampleRate", this->sampleRate);
                retObj.Set("container", container);
                retObj.Set("serialno", this->stream->serialno);
                retObj.Set("pages", this->pages);
                retObj.Set("headerPages", this->headerPages);
                retObj.Set("bufferSize", this->bufferSize);
                retObj.Set("bufferOffset", this->bufferOffset);
                retObj.Set("packetNumber", rtp.seq);

                callback.Call({env.Null(), retObj});
            }
            else
            {
                callback.Call({env.Null(), container});
            }

            this->pending = false;

            return;
        }
        else
        {
            this->pending = true;
        }
    }

    callback.Call({});
}
