"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.RtpEncoder = void 0;
const debug_1 = __importDefault(require("debug"));
const stream_1 = require("stream");
const bindings_1 = __importDefault(require("bindings"));
const { RtpEncoder: RtpEncoderNative } = bindings_1.default('rtpoggopus');
const debug = debug_1.default('rtp-ogg-opus:*');
class RtpEncoder extends stream_1.Transform {
    constructor(options) {
        var _a;
        super({ objectMode: (_a = options === null || options === void 0 ? void 0 : options.objectMode) !== null && _a !== void 0 ? _a : false });
        const { payloadType, samples, objectMode } = Object.assign({ payloadType: 120, samples: 160, objectMode: false }, options);
        this._encoder = new RtpEncoderNative(payloadType, samples, objectMode);
    }
    _transform(chunk, encoding, callback) {
        try {
            this._encoder.transform(chunk, callback);
        }
        catch (err) {
            debug('RtpEncoder error', err);
        }
    }
}
exports.RtpEncoder = RtpEncoder;
exports.default = RtpEncoder;
