import Debug from 'debug';
import { Transform, TransformCallback } from 'stream';
import bindings from 'bindings';

const { RtpEncoder: RtpEncoderNative } = bindings('rtpoggopus');
const debug = Debug('rtp-ogg-opus:*');

export interface RtpEncoderOptions {
    payloadType: number;
    samples: number;
    objectMode: boolean;
}

export class RtpEncoder extends Transform {
    private _decoder: typeof RtpEncoderNative;

    constructor(options: RtpEncoderOptions = { payloadType: 120, samples: 160, objectMode: false }) {
        super({ objectMode: options.objectMode });
        const { objectMode } = options;
        this._decoder = new RtpEncoderNative(objectMode);
    }

    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void {
        try {
            this._decoder.transform(chunk, callback);
        } catch (err) {
            debug('RtpEncoder error', err);
        }
    }
}

export default RtpEncoder;
