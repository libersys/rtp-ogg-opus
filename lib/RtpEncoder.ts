import Debug from 'debug';
import { Transform, TransformCallback } from 'stream';
import bindings from 'bindings';

const { RtpEncoder: RtpEncoderNative } = bindings('rtpoggopus');
const debug = Debug('rtp-ogg-opus:*');

export interface RtpEncoderOptions {
    payloadType?: number;
    samples?: number;
    objectMode?: boolean;
}

export class RtpEncoder extends Transform {
    private _encoder: typeof RtpEncoderNative;

    constructor(options?: RtpEncoderOptions) {
        super({ objectMode: options?.objectMode ?? false });
        const { payloadType, samples, objectMode } = {
            payloadType: 120,
            samples: 160,
            objectMode: false,
            ...options,
        };
        this._encoder = new RtpEncoderNative(payloadType, samples, objectMode);
    }

    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void {
        try {
            this._encoder.transform(chunk, callback);
        } catch (err) {
            debug('RtpEncoder error', err);
        }
    }
}

export default RtpEncoder;
