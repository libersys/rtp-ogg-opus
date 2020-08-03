"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const debug_1 = __importDefault(require("debug"));
const stream_1 = require("stream");
const bindings_1 = __importDefault(require("bindings"));
const { RtpDecoder: RtpDecoderNative } = bindings_1.default('rtpoggopus');
const debug = debug_1.default('rtp-ogg-opus:*');
class RtpDecoder extends stream_1.Transform {
    constructor({ objectMode = false }) {
        super({ objectMode });
        this._decoder = new RtpDecoderNative(objectMode);
    }
    _transform(chunk, encoding, callback) {
        try {
            this._decoder.transform(chunk, callback);
        }
        catch (err) {
            debug('RtpDecoder error', err);
        }
    }
}
exports.RtpDecoder = RtpDecoder;
exports.default = RtpDecoder;
