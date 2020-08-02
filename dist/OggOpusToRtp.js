"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const debug_1 = __importDefault(require("debug"));
const stream_1 = require("stream");
const bindings_1 = __importDefault(require("bindings"));
const { OggOpusToRtp: OggOpusToRtpNative } = bindings_1.default('rtpoggopus');
const debug = debug_1.default('rtp-ogg-opus:*');
class OggOpusToRtp extends stream_1.Transform {
    constructor(options = { payloadType: 120, sampleRate: 48000, objectMode: false }) {
        super({ objectMode: options.objectMode });
        const { payloadType, sampleRate, objectMode } = options;
        this._transformer = new OggOpusToRtpNative(payloadType, sampleRate, objectMode);
    }
    _transform(chunk, encoding, callback) {
        try {
            this._transformer.transform(chunk, callback);
        }
        catch (err) {
            debug('OggOpusToRtp error', err);
        }
    }
}
exports.OggOpusToRtp = OggOpusToRtp;
exports.default = OggOpusToRtp;
