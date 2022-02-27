"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.RtpToOggOpus = void 0;
const debug_1 = __importDefault(require("debug"));
const stream_1 = require("stream");
const bindings_1 = __importDefault(require("bindings"));
const { RtpToOggOpus: RtpToOggOpusNative } = (0, bindings_1.default)('rtpoggopus');
const debug = (0, debug_1.default)('rtp-ogg-opus:*');
class RtpToOggOpus extends stream_1.Transform {
    constructor(options) {
        var _a;
        super({ objectMode: (_a = options === null || options === void 0 ? void 0 : options.objectMode) !== null && _a !== void 0 ? _a : false });
        const { sampleRate, channels, objectMode } = Object.assign({ sampleRate: 48000, channels: 2, objectMode: false }, options);
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
