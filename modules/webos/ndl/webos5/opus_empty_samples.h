#pragma once

static const unsigned char opus_empty_frame_220[7] = {
        0xe8, 0x02, 0xff, 0xfe, 0xe8, 0xff, 0xfe
};

static const unsigned char opus_empty_frame_211[3] = {
        0xec, 0xff, 0xfe
};

static const unsigned char opus_empty_frame_642[15] = {
        0xec, 0x02, 0xff, 0xfe, 0xec, 0x02, 0xff, 0xfe,
        0xe8, 0x02, 0xff, 0xfe, 0xe8, 0xff, 0xfe
};

static const unsigned char opus_empty_frame_880[31] = {
        0xe8, 0x02, 0xff, 0xfe, 0xe8, 0x02, 0xff, 0xfe,
        0xe8, 0x02, 0xff, 0xfe, 0xe8, 0x02, 0xff, 0xfe,
        0xe8, 0x02, 0xff, 0xfe, 0xe8, 0x02, 0xff, 0xfe,
        0xe8, 0x02, 0xff, 0xfe, 0xe8, 0xff, 0xfe
};

static const unsigned char opus_empty_frame_853[19] = {
        0xec, 0x02, 0xff, 0xfe, 0xec, 0x02, 0xff, 0xfe,
        0xec, 0x02, 0xff, 0xfe, 0xe8, 0x02, 0xff, 0xfe,
        0xe8, 0xff, 0xfe
};