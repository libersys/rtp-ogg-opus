import { Transform, TransformCallback } from 'stream';
import Debug from 'debug';

import RtpPacket from './RtpPacket';

const debug = Debug('rtp-ogg-opus:*');

export class RtpEncoder extends Transform {
    private _rtp: RtpPacket;

    constructor(type: number) {
        super();
        this._rtp = new RtpPacket();
        this._rtp.type = type;
    }

    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void {
        try {
            this._rtp.payload = chunk;
            this._rtp.seq++;
            this._rtp.time += chunk.length / 2;
            this.push(this._rtp.packet);
            return callback();
        } catch (err) {
            debug('RtpEncoder error', err);
        }
    }
}

export default RtpEncoder;
