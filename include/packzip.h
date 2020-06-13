#define OFFZIP_WBITS_ZLIB		15
#define OFFZIP_WBITS_DEFLATE	-15


// from quickbms.c
enum {
    LZMA_FLAGS_NONE         = 0,
    LZMA_FLAGS_86_HEADER    = 1,
    LZMA_FLAGS_86_DECODER   = 2,
    LZMA_FLAGS_EFS          = 4,
    LZMA_FLAGS_PROP0        = 0x1000,
    LZMA_FLAGS_NOP
};

int offzip_util(const char *input, const char *output_dir, const char *basename, int wbits);
int packzip_util(const char *input, const char *output, uint32_t offset, int wbits);
