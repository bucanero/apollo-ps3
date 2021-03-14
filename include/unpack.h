#define OFFZIP_WBITS_ZLIB		15
#define OFFZIP_WBITS_DEFLATE	-15


int offzip_util(const char *input, const char *output_dir, const char *basename, int wbits);
int packzip_util(const char *input, const char *output, uint32_t offset, int wbits);

// Diablo 3 save data encryption
void diablo_decrypt_data(uint8_t* data, uint32_t size);
void diablo_encrypt_data(uint8_t* data, uint32_t size);

// Blowfish ECB save data encryption
void blowfish_ecb_encrypt(uint8_t* data, uint32_t len, uint8_t* key, uint32_t key_len);
void blowfish_ecb_decrypt(uint8_t* data, uint32_t len, uint8_t* key, uint32_t key_len);

// AES ECB save data encryption
void aes_ecb_decrypt(uint8_t* data, uint32_t len, uint8_t* key, uint32_t key_len);
void aes_ecb_encrypt(uint8_t* data, uint32_t len, uint8_t* key, uint32_t key_len);

// DES ECB save data encryption
void des_ecb_decrypt(uint8_t* data, uint32_t len, uint8_t* key, uint32_t key_len);
void des_ecb_encrypt(uint8_t* data, uint32_t len, uint8_t* key, uint32_t key_len);

// 3-DES CBC save data encryption
void des3_cbc_decrypt(uint8_t* data, uint32_t len, const uint8_t* key, uint32_t key_len, uint8_t* iv, uint32_t iv_len);
void des3_cbc_encrypt(uint8_t* data, uint32_t len, const uint8_t* key, uint32_t key_len, uint8_t* iv, uint32_t iv_len);

// NFS Undercover save data encryption
void nfsu_decrypt_data(uint8_t* data, uint32_t size);
void nfsu_encrypt_data(uint8_t* data, uint32_t size);

// Silent Hill 3 save data encryption
void sh3_decrypt_data(uint8_t* data, uint32_t size);
void sh3_encrypt_data(uint8_t* data, uint32_t size);

// Final Fantasy XIII (1/2/3) save data encryption
void ff13_decrypt(uint32_t game, uint8_t* data, uint32_t len, const uint8_t* key, uint32_t key_len);
void ff13_encrypt(uint32_t game, uint8_t* data, uint32_t len, const uint8_t* key, uint32_t key_len);
