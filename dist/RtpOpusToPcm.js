"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.RtpOpusToPcm = void 0;
const debug_1 = __importDefault(require("debug"));
const stream_1 = require("stream");
const bindings_1 = __importDefault(require("bindings"));
const { RtpOpusToPcm: RtpOpusToPcmNative } = bindings_1.default('rtpoggopus');
const debug = debug_1.default('rtp-ogg-opus:*');
class RtpOpusToPcm extends stream_1.Transform {
    constructor(options) {
        super();
        const { sampleRate, channels } = Object.assign({ sampleRate: 48000, channels: 2 }, options);
        this._transformer = new RtpOpusToPcmNative(sampleRate, channels);
    }
    _transform(chunk, encoding, callback) {
        try {
            this._transformer.transform(chunk, callback);
        }
        catch (err) {
            debug('RtpOpusToPcm error', err);
        }
    }
}
exports.RtpOpusToPcm = RtpOpusToPcm;
exports.default = RtpOpusToPcm;
