/// <reference types="node" />
export declare class RtpPacket {
    private _packet;
    constructor();
    get type(): number;
    set type(value: number);
    get seq(): number;
    set seq(value: number);
    get time(): number;
    set time(value: number);
    get source(): number;
    set source(value: number);
    get payload(): Buffer;
    set payload(value: Buffer);
    get packet(): Buffer;
    private toUnsigned;
}
export default RtpPacket;
