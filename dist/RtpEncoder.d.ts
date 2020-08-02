/// <reference types="node" />
import { Transform, TransformCallback } from 'stream';
export declare class RtpEncoder extends Transform {
    private _rtp;
    constructor(type: number);
    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void;
}
export default RtpEncoder;
