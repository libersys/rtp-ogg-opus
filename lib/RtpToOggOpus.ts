import Debug from 'debug';
import { Transform, TransformCallback } from 'stream';
import bindings from 'bindings';

const { RtpToOggOpus: RtpToOggOpusNative } = bindings('rtpoggopus');
const debug = Debug('rtp-ogg-opus:*');

export interface RtpToOggOpusOptions {
    sampleRate?: number;
    channels?: number;
    objectMode?: boolean;
}

export class RtpToOggOpus extends Transform {
    private _transformer: typeof RtpToOggOpusNative;

    constructor(options?: RtpToOggOpusOptions) {
        super({ objectMode: options?.objectMode ?? false });
        const { sampleRate, channels, objectMode } = { sampleRate: 48000, channels: 2, objectMode: false, ...options };
        this._transformer = new RtpToOggOpusNative(sampleRate, channels, objectMode);
    }

    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void {
        try {
            this._transformer.transform(chunk, callback);
        } catch (err) {
            debug('RtpToOggOpus error', err);
        }
    }
}

export default RtpToOggOpus;
