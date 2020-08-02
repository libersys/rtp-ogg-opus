import Debug from 'debug';
import { Transform, TransformCallback } from 'stream';
import bindings from 'bindings';

const { RtpOpusToPcm: RtpOpusToPcmNative } = bindings('rtpoggopus');
const debug = Debug('rtp-ogg-opus:*');

export interface RtpOpusToPcmOptions {
    sampleRate: number;
    channels: number;
}

export class RtpOpusToPcm extends Transform {
    private _transformer: typeof RtpOpusToPcmNative;

    constructor(options: RtpOpusToPcmOptions = { sampleRate: 48000, channels: 2 }) {
        super();
        const { sampleRate, channels } = options;
        this._transformer = new RtpOpusToPcmNative(sampleRate, channels);
    }

    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void {
        try {
            this._transformer.transform(chunk, callback);
        } catch (err) {
            debug('RtpOpusToPcm error', err);
        }
    }
}

export default RtpOpusToPcm;
