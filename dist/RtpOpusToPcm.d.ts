/// <reference types="node" />
import { Transform, TransformCallback } from 'stream';
export interface RtpOpusToPcmOptions {
    sampleRate?: number;
    channels?: number;
}
export declare class RtpOpusToPcm extends Transform {
    private _transformer;
    constructor(options?: RtpOpusToPcmOptions);
    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void;
}
export default RtpOpusToPcm;
