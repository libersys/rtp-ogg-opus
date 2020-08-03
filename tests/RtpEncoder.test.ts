import _ from 'lodash';
// import { v4 as uuid } from 'uuid';
// import Debug from 'debug';
import fs from 'fs';
import path from 'path';

import { RtpEncoder } from '../lib';
import { randomInt } from './utils/random';

// const debug = Debug('rtp-ogg-opus:*');
const attrs = ['version', 'payloadType', 'pad', 'ext', 'cc', 'mark', 'seq', 'time', 'ssrc', 'headerSize', 'packet'];
const testFile = path.join(__dirname, 'utils/rtppayload.raw');

describe('RtpEncoder', () => {
    let fileStream: fs.ReadStream;
    let chunk: Buffer;

    beforeEach(async () => {
        try {
            fileStream = fs.createReadStream(testFile);
            chunk = fs.readFileSync(testFile);
        } catch (err) {
            console.error(err);
        }
    });

    // afterEach(async () => {});

    it('should return a buffer (default mode)', done => {
        const encoder = new RtpEncoder();

        encoder.on('data', (data: any) => {
            expect(Buffer.isBuffer(data)).toBeTruthy();
            done();
        });

        fileStream.pipe(encoder);
    });

    it('should return a valid object (objectMode: true)', done => {
        const encoder = new RtpEncoder({ objectMode: true });

        encoder.on('data', (obj: any) => {
            for (const key of attrs) {
                expect(_.has(obj, key)).toBeTruthy();
            }

            expect(Buffer.isBuffer(obj.packet)).toBeTruthy();

            done();
        });

        fileStream.pipe(encoder);
    });

    it('should increase seq by one and time by samples', done => {
        const samples = randomInt(100, 900);
        const decoder = new RtpEncoder({ objectMode: true, samples });
        let counter = 0;
        let lastSeq = 0;
        let lastTime = 0;

        decoder.on('data', (obj: any) => {
            if (lastSeq > 0) {
                expect(obj.seq - lastSeq).toBe(1);
            }
            lastSeq = obj.seq;

            if (lastTime > 0) {
                expect(obj.time - lastTime).toBe(samples);
            }
            lastTime = obj.time;
            counter++;
            if (counter >= 5) done();
        });

        for (let i = 0; i < 5; i++) {
            decoder.write(chunk);
        }
    });
});
