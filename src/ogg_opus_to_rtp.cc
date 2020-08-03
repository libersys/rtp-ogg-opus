#include <napi.h>
#include <iostream>

#include "ogg_opus_to_rtp.h"
#include "helpers.h"

Napi::FunctionReference OggOpusToRtp::constructor;

Napi::Object OggOpusToRtp::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "OggOpusToRtp", {InstanceMethod("transform", &OggOpusToRtp::Transform)});

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("OggOpusToRtp", func);
    return exports;
}

// Constructor
OggOpusToRtp::OggOpusToRtp(const Napi::CallbackInfo &info) : Napi::ObjectWrap<OggOpusToRtp>(info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 2)
    {
        throw Napi::Error::New(env, "Payload type and sample rate expected.");
    }
    if (!info[0].IsNumber())
    {
        throw Napi::Error::New(env, "Payload type must be a number.");
    }
    if (!info[1].IsNumber())
    {
        throw Napi::Error::New(env, "Sample rate must be a number.");
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

    this->sampleRate = info[1].As<Napi::Number>();

    // Initialize our ogg sync state.
    if (ogg_sync_init(&this->ostate) < 0)
        throw Napi::Error::New(env, "Couldn't initialize Ogg sync state.");
}

// Destructor
OggOpusToRtp::~OggOpusToRtp()
{
    ogg_sync_clear(&this->ostate);
    // ogg_sync_destroy(&this->ostate);
}

void OggOpusToRtp::Transform(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 3)
    {
        throw Napi::Error::New(env, "chunk, callback and parent arguments expected.");
    }
    if (!info[0].IsBuffer())
    {
        throw Napi::Error::New(env, "chunk must be a buffer.");
    }
    if (!info[1].IsFunction())
    {
        throw Napi::Error::New(env, "callback must be a function.");
    }
    if (!info[2].IsObject())
    {
        throw Napi::Error::New(info.Env(), "parent must be a stream transform object.");
    }

    Napi::Buffer<char> chunk = info[0].As<Napi::Buffer<char>>();
    Napi::Function callback = info[1].As<Napi::Function>();
    Napi::Object thisObj = info[2].As<Napi::Object>();
    Napi::Function push = thisObj.Get("push").As<Napi::Function>();

    char *input = chunk.Data();
    size_t inputSize = chunk.Length();

    if (!input)
        throw Napi::Error::New(env, "Couldn't open input.");

    ogg_page opage;
    ogg_packet opacket;

    // Create an ogg buffer
    char *obuffer;
    long obufferSize = (long)inputSize;

    obuffer = ogg_sync_buffer(&this->ostate, obufferSize);
    if (!obuffer)
        throw Napi::Error::New(env, "ogg_sync_buffer failed.");

    memcpy(obuffer, input, inputSize);
    // ogg_sync_wrote function is used to tell the ogg_sync_state struct how many bytes we wrote into the buffer.
    if (ogg_sync_wrote(&this->ostate, obufferSize) < 0)
        throw Napi::Error::New(env, "ogg_sync_wrote failed.");

    int pages = 0;
    int packetsout = 0;
    int invalid = 0;
    int eos = 0;

    /* LOOP START */
    while (ogg_sync_pageout(&this->ostate, &opage) == 1)
    {
        pages++;

        if (this->headers == 0)
        {
            if (is_opus(&opage))
            {
                /* this is the start of an Opus stream */
                this->serialno = ogg_page_serialno(&opage);
                if (ogg_stream_init(&this->ostream, ogg_page_serialno(&opage)) < 0)
                    throw Napi::Error::New(env, "ogg_stream_init failed.");

                this->headers++;
            }
            else if (!ogg_page_bos(&opage))
            {
                // We're past the header and haven't found an Opus stream.
                // Time to give up.
                // push.Call(thisObj, {env.Null()});
                break;
            }
            else
            {
                /* try again */
                continue;
            }
        }

        eos = ogg_page_eos(&opage);

        /* submit the page for packetization */
        if (ogg_stream_pagein(&this->ostream, &opage) < 0)
            throw Napi::Error::New(env, "ogg_stream_pagein failed.");

        /* read and process available packets */
        while (ogg_stream_packetout(&this->ostream, &opacket) == 1)
        {

            packetsout++;

            int samples;
            /* skip header packets */
            if (this->headers == 1 && opacket.bytes >= 19 && !memcmp(opacket.packet, "OpusHead", 8))
            {
                this->headers++;
                continue;
            }
            if (this->headers == 2 && opacket.bytes >= 16 && !memcmp(opacket.packet, "OpusTags", 8))
            {
                this->headers++;
                continue;
            }

            /* get packet duration */
            samples = opus_packet_get_nb_samples(opacket.packet, opacket.bytes, this->sampleRate);
            if (samples <= 0)
            {
                invalid++;
                continue; // skipping invalid packet
            }

            /* update the rtp header and send */
            this->rtp.header_size = 12 + 4 * this->rtp.cc;
            this->rtp.seq++;
            this->rtp.time += samples;
            this->rtp.payload_size = opacket.bytes;

            // Create RTP Packet
            unsigned char *packet;
            size_t packetSize = this->rtp.header_size + this->rtp.payload_size;
            packet = (unsigned char *)malloc(packetSize);
            if (!packet)
                throw Napi::Error::New(info.Env(), "Couldn't allocate packet buffer.");

            // Serialize header and copy to packet. Then copy payload to packet.
            serialize_rtp_header(packet, this->rtp.header_size, &this->rtp);
            memcpy(packet + this->rtp.header_size, opacket.packet, opacket.bytes);

            Napi::Buffer<unsigned char> output = Napi::Buffer<unsigned char>::Copy(env, reinterpret_cast<unsigned char *>(packet), packetSize);

            push.Call(thisObj, {output});
        }

        if (eos > 0)
        {
            // End of the logical bitstream, clear headers to reset.
            this->headers = 0;
        }
    }

    /* CLEAN UP */
    if (eos > 0)
    {
        ogg_stream_clear(&this->ostream);
        ogg_sync_clear(&this->ostate);
    }

    callback.Call({});
}
