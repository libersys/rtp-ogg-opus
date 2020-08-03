/// <reference types="node" />
import { Transform, TransformCallback } from 'stream';
export interface OggOpusToRtpOptions {
    payloadType?: number;
    sampleRate?: number;
    objectMode?: boolean;
}
export declare class OggOpusToRtp extends Transform {
    private _transformer;
    constructor(options?: OggOpusToRtpOptions);
    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void;
}
export default OggOpusToRtp;
