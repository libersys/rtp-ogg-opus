/// <reference types="node" />
import { Transform, TransformCallback } from 'stream';
export interface RtpEncoderOptions {
    payloadType?: number;
    samples?: number;
    objectMode?: boolean;
}
export declare class RtpEncoder extends Transform {
    private _encoder;
    constructor(options?: RtpEncoderOptions);
    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void;
}
export default RtpEncoder;
