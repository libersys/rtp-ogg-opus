import _ from 'lodash';
import { v4 as uuid } from 'uuid';
import Debug from 'debug';
import fs from 'fs';
import path from 'path';

import { RtpDecoder } from '../lib';

const debug = Debug('rtp-ogg-opus:*');
const attrs = ['version', 'payloadType', 'pad', 'ext', 'cc', 'mark', 'seq', 'time', 'ssrc', 'headerSize', 'payload'];
const testFile = path.join(__dirname, 'utils/rtppacket.dat');

describe('RtpDecoder', () => {
    let fileStream: fs.ReadStream;

    beforeEach(async () => {
        try {
            fileStream = fs.createReadStream(testFile);
        } catch (err) {
            console.error(err);
        }
    });

    afterEach(async () => {});

    it('should return a buffer (default mode)', done => {
        const decoder = new RtpDecoder();

        decoder.on('data', (data: any) => {
            expect(Buffer.isBuffer(data)).toBe(true);

            done();
        });

        fileStream.pipe(decoder);
    });

    it('should return a valid object (objectMode: true)', done => {
        const decoder = new RtpDecoder({ objectMode: true });

        decoder.on('data', (obj: any) => {
            for (const key of attrs) {
                expect(_.has(obj, key)).toBeTruthy();
            }

            expect(obj.version).toBe(2);

            expect(Buffer.isBuffer(obj.payload)).toBeTruthy();

            done();
        });

        fileStream.pipe(decoder);
    });
});
