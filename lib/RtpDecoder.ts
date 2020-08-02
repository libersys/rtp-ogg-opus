import Debug from 'debug';
import { Transform, TransformCallback } from 'stream';
import bindings from 'bindings';

const { RtpDecoder: RtpDecoderNative } = bindings('rtpoggopus');
const debug = Debug('rtp-ogg-opus:*');

export interface RtpDecoderOptions {
    objectMode: boolean;
}

export class RtpDecoder extends Transform {
    private _decoder: typeof RtpDecoderNative;

    constructor(options: RtpDecoderOptions = { objectMode: false }) {
        super({ objectMode: options.objectMode });
        const { objectMode } = options;
        this._decoder = new RtpDecoderNative(objectMode);
    }

    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void {
        try {
            this._decoder.transform(chunk, callback);
        } catch (err) {
            debug('RtpDecoder error', err);
        }
    }
}

export default RtpDecoder;
