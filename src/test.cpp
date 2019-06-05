int memcpy(void* dest, void* src, size_t length) {
    if ((dest == NULL) || (src == NULL)) {
        return -1;
    }

    char* char_dest = (char*)dest;
    char* char_src = (char*)src;

    char* dest_end = char_dest + length;
    char* src_end = char_src + length;
    if ((dest_end < char_src) || (char_dest > src_end)) {
        for (size_t i = 0; i < length; i++) {
            *char_dest = *char_src;
            char_dest++;
            char_src++;
        }
        return 0;
    } else {
        if (char_dest < char_src) {
            for (size_t i = 0; i < length; i++) {
                *char_dest = *char_src;
                char_dest++;
                char_src++;
            }
        } else {
            for (int i = length - 1; i > 0; i--) {
                *(char_dest + i) = *(char_src + i);
            }
        }
        return 0;
    }

    int start = 0;
    int step = 0;
    int end = 0;
    if ((dest_end < char_src) || (char_dest > src_end) || (char_dest > char_src)) {
        start = 0;
        end = length;
        step = 1;
    } else {
        start = length - 1;
        end = -1;
        step = -1;
    }

    for (int i = start; i != end; i+=step) {
        *(char_dest + i) = *(char_src + i);
    }

    return 0;
}
