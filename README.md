# rtp-ogg-opus

RTP Ogg/Opus Tools for NodeJS

## Introduction

This module contains five useful transform streams to work with RTP and Ogg/Opus. This classes are based on native bindings to
libopus 1.3.1 and libogg 1.3.4. The core of this library is based on [opus-tools 0.2](https://opus-codec.org/release/dev/2018/09/18/opus-tools-0_2.html).

OggOpusToRtp: used to read ogg/opus files or streams and send them through RTP.
RtpDecoder: used to decode RTP packets into streams (payload).
RtpEncoder: used to encode streams into RTP packets.
RtpOpusToPcm: used to read RTP Opus encoded packets and converts them to Linear PCM.
RtpToOggOpus: used to read RTP Opus encoded packets and encapsulate them in Ogg containers.

## Installation

```sh
npm install rtp-ogg-opus
```

## Usage

Check out our [examples project](https://github.com/libersys/rtp-ogg-opus-examples).
