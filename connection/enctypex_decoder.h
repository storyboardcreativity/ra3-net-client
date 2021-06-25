#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

// can be used to decode
int enctypex_wrapper(unsigned char *key, unsigned char *validate, unsigned char *data, int size);

// data must be enough big to include the 23 bytes header, remember it: data = realloc(data, size + 23);
int enctypex_quick_encrypt(unsigned char *key, unsigned char *validate, unsigned char *data, int size);

#ifdef __cplusplus
}
#endif
