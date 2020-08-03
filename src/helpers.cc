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

#include <string.h>
#include <stdexcept>

#include "../deps/libogg/include/ogg/ogg.h"
#include "../deps/libopus/include/opus.h"

#include "helpers.h"

/* helper, write a little-endian 32 bit int to memory */
void le32(unsigned char *p, int v)
{
    p[0] = v & 0xff;
    p[1] = (v >> 8) & 0xff;
    p[2] = (v >> 16) & 0xff;
    p[3] = (v >> 24) & 0xff;
}

/* helper, write a little-endian 16 bit int to memory */
void le16(unsigned char *p, int v)
{
    p[0] = v & 0xff;
    p[1] = (v >> 8) & 0xff;
}

/* helper, write a big-endian 32 bit int to memory */
void be32(unsigned char *p, int v)
{
    p[0] = (v >> 24) & 0xff;
    p[1] = (v >> 16) & 0xff;
    p[2] = (v >> 8) & 0xff;
    p[3] = v & 0xff;
}

/* helper, write a big-endian 16 bit int to memory */
void be16(unsigned char *p, int v)
{
    p[0] = (v >> 8) & 0xff;
    p[1] = v & 0xff;
}

/* helper, read a big-endian 16 bit int from memory */
static int rbe16(const unsigned char *p)
{
    int v = p[0] << 8 | p[1];
    return v;
}

/* helper, read a big-endian 32 bit int from memory */
static int rbe32(const unsigned char *p)
{
    int v = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    return v;
}

/* helper, read a native-endian 32 bit int from memory */
static int rne32(const unsigned char *p)
{
    /* On x86 we could just cast, but that might not meet
   * arm alignment requirements. */
    int d = 0;
    memcpy(&d, p, 4);
    return d;
}

/* free a packet and its contents */
void op_free(ogg_packet *op)
{
    if (op)
    {
        if (op->packet)
        {
            free(op->packet);
        }
        free(op);
    }
}

void free_state(state *st)
{
    if (st)
    {
        // if (st->stream)
        // {
        //     free(st->stream);
        // }
        if (st->output)
        {
            free(st->output);
        }
        free(st);
    }
}

/* manufacture a generic OpusHead packet */
ogg_packet *op_opushead(int samplerate, int channels)
{
    int size = 19;
    unsigned char *data = (unsigned char *)malloc(size);
    ogg_packet *op = (ogg_packet *)malloc(sizeof(*op));

    if (!data)
    {
        free(op);
        throw std::invalid_argument("Couldn't allocate data buffer.");
    }

    if (!op)
    {
        free(data);
        throw std::invalid_argument("Couldn't allocate Ogg packet.");
    }

    memcpy(data, "OpusHead", 8); /* identifier */
    data[8] = 1;                 /* version */
    data[9] = channels;          /* channels */
    le16(data + 10, 0);          /* pre-skip */
    le32(data + 12, samplerate); /* original sample rate */
    le16(data + 16, 0);          /* gain */
    data[18] = 0;                /* channel mapping family */

    op->packet = data;
    op->bytes = size;
    op->b_o_s = 1;
    op->e_o_s = 0;
    op->granulepos = 0;
    op->packetno = 0;

    return op;
}

/* manufacture a generic OpusTags packet */
ogg_packet *op_opustags(void)
{
    char *vendor = "rtp-ogg-opus using libopus";
    int size = 8 + 4 + strlen(vendor) + 4;
    unsigned char *data = (unsigned char *)malloc(size);
    ogg_packet *op = (ogg_packet *)malloc(sizeof(*op));

    if (!data)
    {
        free(op);
        throw std::invalid_argument("Couldn't allocate data buffer.");
    }

    if (!op)
    {
        free(data);
        throw std::invalid_argument("Couldn't allocate Ogg packet.");
    }

    memcpy(data, "OpusTags", 8);
    le32(data + 8, strlen(vendor));
    memcpy(data + 12, vendor, strlen(vendor));
    le32(data + 12 + strlen(vendor), 0);

    op->packet = data;
    op->bytes = size;
    op->b_o_s = 0;
    op->e_o_s = 0;
    op->granulepos = 0;
    op->packetno = 1;

    return op;
}

ogg_packet *op_from_pkt(const unsigned char *pkt, int len)
{
    ogg_packet *op = (ogg_packet *)malloc(sizeof(*op));
    if (!op) /* "Couldn't allocate Ogg packet." */
        return NULL;

    op->packet = (unsigned char *)pkt;
    op->bytes = len;
    op->b_o_s = 0;
    op->e_o_s = 0;

    return op;
}

/* check if an ogg page begins an opus stream */
int is_opus(ogg_page *og)
{
    ogg_stream_state os;
    ogg_packet op;

    ogg_stream_init(&os, ogg_page_serialno(og));
    ogg_stream_pagein(&os, og);
    if (ogg_stream_packetout(&os, &op) == 1)
    {
        if (op.bytes >= 19 && !memcmp(op.packet, "OpusHead", 8))
        {
            ogg_stream_clear(&os);
            return 1;
        }
    }
    ogg_stream_clear(&os);
    return 0;
}

int parse_rtp_header(const unsigned char *packet, size_t size, rtp_header *rtp)
{
    if (!packet || !rtp)
    {
        return -2;
    }
    if (size < RTP_HEADER_MIN) /* Packet too short for RTP. */
        return -1;

    rtp->version = (packet[0] >> 6) & 3;
    rtp->pad = (packet[0] >> 5) & 1;
    rtp->ext = (packet[0] >> 4) & 1;
    rtp->cc = packet[0] & 7;
    rtp->header_size = 12 + 4 * rtp->cc;

    if (rtp->ext == 1)
    {
        uint16_t ext_length;
        rtp->header_size += 4;
        ext_length = rbe16(packet + rtp->header_size - 2);
        rtp->header_size += ext_length * 4;
    }
    rtp->payload_size = size - rtp->header_size;

    rtp->mark = (packet[1] >> 7) & 1;
    rtp->payload_type = (packet[1]) & 127;
    rtp->seq = rbe16(packet + 2);
    rtp->time = rbe32(packet + 4);
    rtp->ssrc = rbe32(packet + 8);
    rtp->csrc = NULL;

    if (size < (size_t)rtp->header_size) // "Packet too short for RTP header."
        return -1;

    return 0;
}

int serialize_rtp_header(unsigned char *packet, int size, rtp_header *rtp)
{
    int i;

    if (!packet || !rtp)
        return -2;

    if (size < RTP_HEADER_MIN) // Packet buffer too short for RTP
        return -1;

    if (size < rtp->header_size) // Packet buffer too short for declared RTP header size
        return -3;

    packet[0] = ((rtp->version & 3) << 6) |
                ((rtp->pad & 1) << 5) |
                ((rtp->ext & 1) << 4) |
                ((rtp->cc & 7));
    packet[1] = ((rtp->mark & 1) << 7) |
                ((rtp->payload_type & 127));

    be16(packet + 2, rtp->seq);
    be32(packet + 4, rtp->time);
    be32(packet + 8, rtp->ssrc);

    if (rtp->cc && rtp->csrc)
    {
        for (i = 0; i < rtp->cc; i++)
        {
            be32(packet + 12 + i * 4, rtp->csrc[i]);
        }
    }

    return 0;
}

int update_rtp_header(rtp_header *rtp)
{
    rtp->header_size = 12 + 4 * rtp->cc;
    return 0;
}

int ogg_opus_encapsulate(const unsigned char *packet, int packet_len, int packetno, int samplerate, int channels, state *params)
{
    ogg_packet *head;
    ogg_packet *tags;
    ogg_packet *body;
    int samples;

    params->stream = (ogg_stream_state *)malloc(sizeof(ogg_stream_state));
    if (!params->stream)
        return 1;

    if (ogg_stream_init(params->stream, rand()) < 0)
    {
        // Couldn't initialize Ogg stream state.
        // free_state(params);
        return 1;
    }

    params->output = (unsigned char *)malloc(0);
    params->output_size = 0;
    params->granulepos = 0;
    params->pages = 0;

    /* Write stream headers */
    head = op_opushead(samplerate, channels);
    ogg_stream_packetin(params->stream, head);
    // op_free(op);
    tags = op_opustags();
    ogg_stream_packetin(params->stream, tags);
    // op_free(op);
    // ogg_flush(params);

    // STREAM LOOP - WRITE PAYLOAD
    body = op_from_pkt(packet, packet_len);
    body->packetno = packetno;
    samples = opus_packet_get_nb_samples(packet, packet_len, 48000);
    if (samples > 0)
        params->granulepos += samples;
    body->granulepos = params->granulepos;
    ogg_stream_packetin(params->stream, body);
    // free(op);
    // op_free(op);
    // ogg_write(params);

    // /* write outstanding data */
    // // if (params->output_size > 0)
    // // {
    // //     ogg_flush(params);
    // //     free(params->output);
    // // }

    // /* clean up */
    ogg_stream_destroy(params->stream);
    // // free(params);
    return 0;
}

/* helper, write out available ogg pages */
int ogg_write(ogg_stream_state *stream, unsigned char *output, size_t &output_size)
{

    size_t page_size;

    ogg_page page;

    while (ogg_stream_pageout(stream, &page))
    {

        // Reallocate memory for the page (header and body).
        page_size = (size_t)(page.header_len + page.body_len);
        output = (unsigned char *)realloc(output, output_size + page_size);
        output_size += page_size;

        // TODO: Check data has been written.
    }

    return 0;
}

int ogg_pageout(ogg_stream_state *stream, unsigned char *buffer, size_t buffer_size, size_t &offset)
{
    ogg_page page;

    int pages = 0;
    size_t written = 0;
    size_t writtenBytes = 0;

    while (ogg_stream_pageout(stream, &page))
    {

        memcpy(buffer + offset, page.header, page.header_len);
        memcpy(buffer + offset + page.header_len, page.body, page.body_len);

        written = (size_t)(page.header_len + page.body_len);

        if (written > 0)
        {
            offset += written;
            writtenBytes += written;
        }

        pages++;
        // TODO: Check data has been written.
    }

    return pages;
}

int ogg_flush(ogg_stream_state *stream, unsigned char *buffer, size_t buffer_size, size_t &offset)
{
    ogg_page page;

    int pages = 0;
    size_t written = 0;
    size_t writtenBytes = 0;

    while (ogg_stream_flush(stream, &page))
    {

        memcpy(buffer + offset, page.header, page.header_len);
        memcpy(buffer + offset + page.header_len, page.body, page.body_len);

        written = (size_t)(page.header_len + page.body_len);

        // From Andres Jaan Tack (https://stackoverflow.com/questions/9945938/appending-to-buffer-in-c/9946019#9946019)
        if (written > 0)
        {
            offset += written;
            writtenBytes += written;
        }

        pages++;
        // TODO: Check data has been written.
    }

    return pages;
}
