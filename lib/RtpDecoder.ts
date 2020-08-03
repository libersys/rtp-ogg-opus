import Debug from 'debug';
import { Transform, TransformCallback } from 'stream';
import bindings from 'bindings';
import _ from 'lodash';

const { RtpDecoder: RtpDecoderNative } = bindings('rtpoggopus');
const debug = Debug('rtp-ogg-opus:*');

export interface RtpDecoderOptions {
    objectMode?: boolean;
}

export class RtpDecoder extends Transform {
    private _decoder: typeof RtpDecoderNative;

    constructor(options?: RtpDecoderOptions) {
        super({ objectMode: options?.objectMode ?? false });
        this._decoder = new RtpDecoderNative(options?.objectMode ?? false);
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
