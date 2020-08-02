/* Copyright (C)2012 Xiph.Org Foundation

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HELPERS_H
#define HELPERS_H

#include "../deps/libopus/include/opus.h"
#include "../deps/libogg/include/ogg/ogg.h"

#define DYNAMIC_PAYLOAD_TYPE_MIN 96

#define RTP_HEADER_MIN 12

typedef struct
{
    int version;
    int payload_type;
    int pad, ext, cc, mark;
    int seq, time;
    int ssrc;
    int *csrc;
    int header_size;
    int payload_size;
} rtp_header;

/* state struct for passing around our handles */
typedef struct
{
    ogg_stream_state *stream;
    unsigned char *output;
    size_t output_size;
    int pages;
    int seq;
    ogg_int64_t granulepos;
    // int dst_port;
    // int payload_type;
} state;

/* RTP functions */
int parse_rtp_header(const unsigned char *packet, size_t size, rtp_header *rtp);
int serialize_rtp_header(unsigned char *packet, int size, rtp_header *rtp);
int update_rtp_header(rtp_header *rtp);

/* Ogg and Opus functions*/
int ogg_opus_encapsulate(const unsigned char *packet, int packet_len, int packetno, int samplerate, int channels, state *ogg_state);
void free_state(state *st);
int is_opus(ogg_page *og);

ogg_packet *op_opushead(int samplerate, int channels);
ogg_packet *op_opustags(void);

int ogg_flush(ogg_stream_state *stream, unsigned char *buffer, size_t buffer_size, size_t &offset);
int ogg_write(ogg_stream_state *stream, unsigned char *output, size_t &output_size);
int ogg_pageout(ogg_stream_state *stream, unsigned char *buffer, size_t buffer_size, size_t &offset);

ogg_packet *op_from_pkt(const unsigned char *pkt, int len);

#endif
