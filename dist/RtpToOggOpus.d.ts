/// <reference types="node" />
import { Transform, TransformCallback } from 'stream';
export interface RtpToOggOpusOptions {
    sampleRate?: number;
    channels?: number;
    objectMode?: boolean;
}
export declare class RtpToOggOpus extends Transform {
    private _transformer;
    constructor({ sampleRate, channels, objectMode }: RtpToOggOpusOptions);
    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void;
}
export default RtpToOggOpus;
