#pragma once

static const unsigned char opus_empty_frame_220[11] = {
        0xf9, 0x02, 0xff, 0xfe, 0xff, 0xfe, 0xf9, 0xff,
        0xfe, 0xff, 0xfe
};

static const unsigned char opus_empty_frame_211[5] = {
        0xfd, 0xff, 0xfe, 0xff, 0xfe
};

static const unsigned char opus_empty_frame_642[23] = {
        0xfd, 0x02, 0xff, 0xfe, 0xff, 0xfe, 0xfd, 0x02,
        0xff, 0xfe, 0xff, 0xfe, 0xf9, 0x02, 0xff, 0xfe,
        0xff, 0xfe, 0xf9, 0xff, 0xfe, 0xff, 0xfe
};

static const unsigned char opus_empty_frame_880[47] = {
        0xf9, 0x02, 0xff, 0xfe, 0xff, 0xfe, 0xf9, 0x02,
        0xff, 0xfe, 0xff, 0xfe, 0xf9, 0x02, 0xff, 0xfe,
        0xff, 0xfe, 0xf9, 0x02, 0xff, 0xfe, 0xff, 0xfe,
        0xf9, 0x02, 0xff, 0xfe, 0xff, 0xfe, 0xf9, 0x02,
        0xff, 0xfe, 0xff, 0xfe, 0xf9, 0x02, 0xff, 0xfe,
        0xff, 0xfe, 0xf9, 0xff, 0xfe, 0xff, 0xfe
};

static const unsigned char opus_empty_frame_853[29] = {
        0xfd, 0x02, 0xff, 0xfe, 0xff, 0xfe, 0xfd, 0x02,
        0xff, 0xfe, 0xff, 0xfe, 0xfd, 0x02, 0xff, 0xfe,
        0xff, 0xfe, 0xf9, 0x02, 0xff, 0xfe, 0xff, 0xfe,
        0xf9, 0xff, 0xfe, 0xff, 0xfe
};

static int opus_empty_frames_copy(unsigned char *out, int channels, int streams, int coupled) {
    switch (channels * 100 + streams * 10 + coupled) {
        case 220:
            memcpy(out, opus_empty_frame_220, sizeof(opus_empty_frame_220));
            return sizeof(opus_empty_frame_220);
        case 211:
            memcpy(out, opus_empty_frame_211, sizeof(opus_empty_frame_211));
            return sizeof(opus_empty_frame_211);
        case 660:
            // Fallthrough because it will be converted to 642
        case 642:
            memcpy(out, opus_empty_frame_642, sizeof(opus_empty_frame_642));
            return sizeof(opus_empty_frame_642);
        case 880:
            memcpy(out, opus_empty_frame_880, sizeof(opus_empty_frame_880));
            return sizeof(opus_empty_frame_880);
        case 853:
            memcpy(out, opus_empty_frame_853, sizeof(opus_empty_frame_853));
            return sizeof(opus_empty_frame_853);
        default:
            return 0;
    }
}