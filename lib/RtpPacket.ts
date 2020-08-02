/*
 This is a modified typescript version of the great code published 
 by Brian White on https://github.com/mscdex/node-rtp.
*/

/*
Copyright 2010 Brian White. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE. 
*/

export class RtpPacket {
    private _packet: Buffer;

    constructor() {
        // See RFC3550 for more details: http://www.ietf.org/rfc/rfc3550.txt
        // V  = 2, // version. always 2 for this RFC (2 bits)
        // P  = 0, // padding. not supported yet, so always 0 (1 bit)
        // X  = 0, // header extension (1 bit)
        // CC = 0, // CSRC count (4 bits)
        // M  = 0, // marker (1 bit)
        // PT = 0, // payload type. see section 6 in RFC3551 for valid types: http://www.ietf.org/rfc/rfc3551.txt (7 bits)
        // SN = Math.floor(1000 * Math.random()), // sequence number. SHOULD be random (16 bits)
        // TS = 1, // timestamp in the format of NTP (# sec. since 0h UTC 1 January 1900)? (32 bits)
        // SSRC = 1; // synchronization source (32 bits)
        // //CSRC = 0, // contributing sources. not supported yet (32 bits)
        // //DP = 0, // header extension, 'Defined By Profile'. not supported yet (16 bits)
        // //EL = 0; // header extension length. not supported yet (16 bits)
        this._packet = Buffer.alloc(12);
        // bufpkt[0]  = (V << 6 | P << 5 | X << 4 | CC);
        // bufpkt[1]  = (M << 7 | PT);
        // bufpkt[2]  = (SN >>> 8)
        // bufpkt[3]  = (SN & 0xFF);
        // bufpkt[4]  = (TS >>> 24);
        // bufpkt[5]  = (TS >>> 16 & 0xFF);
        // bufpkt[6]  = (TS >>> 8 & 0xFF);
        // bufpkt[7]  = (TS & 0xFF);
        // bufpkt[8]  = (SSRC >>> 24);
        // bufpkt[9]  = (SSRC >>> 16 & 0xFF);
        // bufpkt[10] = (SSRC >>> 8 & 0xFF);
        // bufpkt[11] = (SSRC & 0xFF);
        this._packet[0] = 0x80;
        this._packet[1] = 0;
        const SN = Math.floor(1000 * Math.random());
        this._packet[2] = SN >>> 8;
        this._packet[3] = SN & 0xff;
        this._packet[4] = 0;
        this._packet[5] = 0;
        this._packet[6] = 0;
        this._packet[7] = 1;
        this._packet[8] = 0;
        this._packet[9] = 0;
        this._packet[10] = 0;
        this._packet[11] = 1;
    }

    get type() {
        return this._packet[1] & 0x7f;
    }

    set type(value: number) {
        value = this.toUnsigned(value);
        if (value <= 127) {
            this._packet[1] -= this._packet[1] & 0x7f;
            this._packet[1] |= value;
        }
    }

    get seq() {
        return (this._packet[2] << 8) | this._packet[3];
    }

    set seq(value: number) {
        value = this.toUnsigned(value);
        if (value <= 65535) {
            this._packet[2] = value >>> 8;
            this._packet[3] = value & 0xff;
        }
    }

    get time() {
        return (this._packet[4] << 24) | (this._packet[5] << 16) | (this._packet[6] << 8) | this._packet[7];
    }

    set time(value: number) {
        value = this.toUnsigned(value);
        if (value <= 4294967295) {
            this._packet[4] = value >>> 24;
            this._packet[5] = (value >>> 16) & 0xff;
            this._packet[6] = (value >>> 8) & 0xff;
            this._packet[7] = value & 0xff;
        }
    }

    get source() {
        return (this._packet[8] << 24) | (this._packet[9] << 16) | (this._packet[10] << 8) | this._packet[11];
    }

    set source(value: number) {
        value = this.toUnsigned(value);
        if (value <= 4294967295) {
            this._packet[8] = value >>> 24;
            this._packet[9] = (value >>> 16) & 0xff;
            this._packet[10] = (value >>> 8) & 0xff;
            this._packet[11] = value & 0xff;
        }
    }

    get payload() {
        return this._packet.slice(12, this._packet.length);
    }

    set payload(value: Buffer) {
        // TODO: validate buffer size
        // if (Buffer.isBuffer(value) && value.length <= 512) {
        // if (Buffer.isBuffer(value)) {
        const newsize = 12 + value.length;
        if (this._packet.length === newsize) value.copy(this._packet, 12, 0);
        else {
            const newbuf = Buffer.alloc(newsize);
            this._packet.copy(newbuf, 0, 0, 12);
            value.copy(newbuf, 12, 0);
            this._packet = newbuf;
        }
        // }
    }

    get packet() {
        return this._packet;
    }

    private toUnsigned(value: number) {
        return (value >>> 1) * 2 + (value & 1);
    }
}

export default RtpPacket;
