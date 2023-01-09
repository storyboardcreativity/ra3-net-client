#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    unsigned char gs_peerchat_1;
    unsigned char gs_peerchat_2;
    unsigned char gs_peerchat_crypt[256];
} gs_peerchat_ctx;

void gs_peerchat_init(gs_peerchat_ctx *ctx, unsigned char *chall, unsigned char *gamekey);
void gs_peerchat(gs_peerchat_ctx *ctx, unsigned char *data, int size);

#ifdef __cplusplus
}
#endif
