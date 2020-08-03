import Debug from 'debug';
import { Transform, TransformCallback } from 'stream';
import bindings from 'bindings';

const { OggOpusToRtp: OggOpusToRtpNative } = bindings('rtpoggopus');
const debug = Debug('rtp-ogg-opus:*');

export interface OggOpusToRtpOptions {
    payloadType?: number;
    sampleRate?: number;
    objectMode?: boolean;
}

export class OggOpusToRtp extends Transform {
    private _transformer: typeof OggOpusToRtpNative;

    constructor(options?: OggOpusToRtpOptions) {
        super({ objectMode: options?.objectMode ?? false });
        const { payloadType, sampleRate, objectMode } = {
            payloadType: 120,
            sampleRate: 48000,
            objectMode: false,
            ...options,
        };
        this._transformer = new OggOpusToRtpNative(payloadType, sampleRate, objectMode);
    }

    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void {
        try {
            this._transformer.transform(chunk, callback);
        } catch (err) {
            debug('OggOpusToRtp error', err);
        }
    }
}

export default OggOpusToRtp;
