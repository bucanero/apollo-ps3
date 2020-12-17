#define OFFZIP_WBITS_ZLIB		15
#define OFFZIP_WBITS_DEFLATE	-15


int offzip_util(const char *input, const char *output_dir, const char *basename, int wbits);
int packzip_util(const char *input, const char *output, uint32_t offset, int wbits);

// Naughty Dog save data encryption
int nd_decrypt_data(uint32_t* data, uint32_t size);
int nd_encrypt_data(uint32_t* data, uint32_t size);
