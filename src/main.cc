#include <napi.h>

#include "rtp_decoder.h"
#include "rtp_encoder.h"
#include "ogg_opus_to_rtp.h"
#include "rtp_to_ogg_opus.h"
#include "rtp_opus_to_pcm.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
    RtpDecoder::Init(env, exports);
    RtpEncoder::Init(env, exports);
    OggOpusToRtp::Init(env, exports);
    RtpToOggOpus::Init(env, exports);
    return RtpOpusToPcm::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
