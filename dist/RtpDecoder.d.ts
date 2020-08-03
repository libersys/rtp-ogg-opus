/// <reference types="node" />
import { Transform, TransformCallback } from 'stream';
export interface RtpDecoderOptions {
    objectMode?: boolean;
}
export declare class RtpDecoder extends Transform {
    private _decoder;
    constructor({ objectMode }: RtpDecoderOptions);
    _transform(chunk: any, encoding: BufferEncoding, callback: TransformCallback): void;
}
export default RtpDecoder;
