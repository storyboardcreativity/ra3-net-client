// Source: https://aluigi.altervista.org/papers/gs_peerchat.h
// Done some minor modifications

#include "peerchat_encryption.h"

void gs_peerchat_init(gs_peerchat_ctx *ctx, unsigned char *chall, unsigned char *gamekey)
{
    unsigned char challenge[16],
        *l,
        *l1,
        *p,
        *p1,
        *crypt,
        t,
        t1;

    ctx->gs_peerchat_1 = 0;
    ctx->gs_peerchat_2 = 0;
    crypt = ctx->gs_peerchat_crypt;

    p = challenge;
    l = challenge + 16;
    p1 = gamekey;
    do
    {
        if (!*p1)
            p1 = gamekey;
        *p++ = *chall++ ^ *p1++; // avoid a memcpy(challenge, chall, 16);
    } while (p != l);

    t1 = 255;
    p1 = crypt;
    l1 = crypt + 256;
    do
    {
        *p1++ = t1--;
    } while (p1 != l1);

    t1++; // means t1 = 0;
    p = crypt;
    p1 = challenge;
    do
    {
        t1 += *p1 + *p;
        t = crypt[t1];
        crypt[t1] = *p;
        *p = t;
        p++;
        p1++;
        if (p1 == l)
            p1 = challenge;
    } while (p != l1);
}

void gs_peerchat(gs_peerchat_ctx *ctx, unsigned char *data, int size)
{
    unsigned char num1,
        num2,
        t,
        *crypt;

    num1 = ctx->gs_peerchat_1;
    num2 = ctx->gs_peerchat_2;
    crypt = ctx->gs_peerchat_crypt;

    while (size--)
    {
        t = crypt[++num1];
        num2 += t;
        crypt[num1] = crypt[num2];
        crypt[num2] = t;
        t += crypt[num1];
        *data++ ^= crypt[t];
    }

    ctx->gs_peerchat_1 = num1;
    ctx->gs_peerchat_2 = num2;
}
