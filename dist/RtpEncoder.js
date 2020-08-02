"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const stream_1 = require("stream");
const debug_1 = __importDefault(require("debug"));
const RtpPacket_1 = __importDefault(require("./RtpPacket"));
const debug = debug_1.default('rtp-ogg-opus:*');
class RtpEncoder extends stream_1.Transform {
    constructor(type) {
        super();
        this._rtp = new RtpPacket_1.default();
        this._rtp.type = type;
    }
    _transform(chunk, encoding, callback) {
        try {
            this._rtp.payload = chunk;
            this._rtp.seq++;
            this._rtp.time += chunk.length / 2;
            this.push(this._rtp.packet);
            return callback();
        }
        catch (err) {
            debug('RtpEncoder error', err);
        }
    }
}
exports.RtpEncoder = RtpEncoder;
exports.default = RtpEncoder;
