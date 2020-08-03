"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const debug_1 = __importDefault(require("debug"));
const stream_1 = require("stream");
const bindings_1 = __importDefault(require("bindings"));
const { RtpToOggOpus: RtpToOggOpusNative } = bindings_1.default('rtpoggopus');
const debug = debug_1.default('rtp-ogg-opus:*');
class RtpToOggOpus extends stream_1.Transform {
    constructor({ sampleRate = 48000, channels = 2, objectMode = false }) {
        super({ objectMode });
        this._transformer = new RtpToOggOpusNative(sampleRate, channels, objectMode);
    }
    _transform(chunk, encoding, callback) {
        try {
            this._transformer.transform(chunk, callback);
        }
        catch (err) {
            debug('RtpToOggOpus error', err);
        }
    }
}
exports.RtpToOggOpus = RtpToOggOpus;
exports.default = RtpToOggOpus;
