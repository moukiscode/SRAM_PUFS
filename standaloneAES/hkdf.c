/* hkdf.c - HKDF-SHA256 (RFC 5869) + HMAC-SHA256 (FIPS 180-4 / RFC 2104) */
#include "hkdf.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Utilitaires                                                         */
/* ------------------------------------------------------------------ */
void hkdf_secure_zero(void *p, size_t len)
{
    volatile uint8_t *v = (volatile uint8_t *)p;
    while (len--) *v++ = 0;
}

int hkdf_ct_memcmp(const void *a, const void *b, size_t len)
{
    const volatile uint8_t *pa = (const volatile uint8_t *)a;
    const volatile uint8_t *pb = (const volatile uint8_t *)b;
    uint8_t diff = 0;
    for (size_t i = 0; i < len; i++) diff |= (uint8_t)(pa[i] ^ pb[i]);
    return diff;   /* 0 si égaux */
}

/* ------------------------------------------------------------------ */
/* SHA-256                                                             */
/* ------------------------------------------------------------------ */
static const uint32_t K256[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROR32(x,n) (((x) >> (n)) | ((x) << (32 - (n))))

static void sha256_transform(sha256_ctx *c, const uint8_t blk[64])
{
    uint32_t w[64], a, b, d, e, f, g, h, cc, t1, t2;
    for (int i = 0; i < 16; i++)
        w[i] = ((uint32_t)blk[4*i] << 24) | ((uint32_t)blk[4*i+1] << 16) |
               ((uint32_t)blk[4*i+2] << 8) | (uint32_t)blk[4*i+3];
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = ROR32(w[i-15],7) ^ ROR32(w[i-15],18) ^ (w[i-15] >> 3);
        uint32_t s1 = ROR32(w[i-2],17) ^ ROR32(w[i-2],19)  ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    a = c->state[0]; b = c->state[1]; cc = c->state[2]; d = c->state[3];
    e = c->state[4]; f = c->state[5]; g = c->state[6];  h = c->state[7];
    for (int i = 0; i < 64; i++) {
        uint32_t S1 = ROR32(e,6) ^ ROR32(e,11) ^ ROR32(e,25);
        uint32_t ch = (e & f) ^ (~e & g);
        t1 = h + S1 + ch + K256[i] + w[i];
        uint32_t S0 = ROR32(a,2) ^ ROR32(a,13) ^ ROR32(a,22);
        uint32_t mj = (a & b) ^ (a & cc) ^ (b & cc);
        t2 = S0 + mj;
        h = g; g = f; f = e; e = d + t1;
        d = cc; cc = b; b = a; a = t1 + t2;
    }
    c->state[0] += a; c->state[1] += b; c->state[2] += cc; c->state[3] += d;
    c->state[4] += e; c->state[5] += f; c->state[6] += g;  c->state[7] += h;
    hkdf_secure_zero(w, sizeof w);
}

void sha256_init(sha256_ctx *c)
{
    c->state[0] = 0x6a09e667; c->state[1] = 0xbb67ae85;
    c->state[2] = 0x3c6ef372; c->state[3] = 0xa54ff53a;
    c->state[4] = 0x510e527f; c->state[5] = 0x9b05688c;
    c->state[6] = 0x1f83d9ab; c->state[7] = 0x5be0cd19;
    c->bitlen = 0;
    c->buflen = 0;
}

void sha256_update(sha256_ctx *c, const uint8_t *data, size_t len)
{
    if (len == 0) return;
    c->bitlen += (uint64_t)len * 8u;
    while (len > 0) {
        size_t n = 64 - c->buflen;
        if (n > len) n = len;
        memcpy(c->buf + c->buflen, data, n);
        c->buflen += n; data += n; len -= n;
        if (c->buflen == 64) {
            sha256_transform(c, c->buf);
            c->buflen = 0;
        }
    }
}

void sha256_final(sha256_ctx *c, uint8_t out[HKDF_HASH_LEN])
{
    uint64_t bits = c->bitlen;
    c->buf[c->buflen++] = 0x80;
    if (c->buflen > 56) {
        memset(c->buf + c->buflen, 0, 64 - c->buflen);
        sha256_transform(c, c->buf);
        c->buflen = 0;
    }
    memset(c->buf + c->buflen, 0, 56 - c->buflen);
    for (int i = 0; i < 8; i++)
        c->buf[56 + i] = (uint8_t)(bits >> (56 - 8*i));
    sha256_transform(c, c->buf);
    for (int i = 0; i < 8; i++) {
        out[4*i]   = (uint8_t)(c->state[i] >> 24);
        out[4*i+1] = (uint8_t)(c->state[i] >> 16);
        out[4*i+2] = (uint8_t)(c->state[i] >> 8);
        out[4*i+3] = (uint8_t)(c->state[i]);
    }
    hkdf_secure_zero(c, sizeof *c);
}

/* ------------------------------------------------------------------ */
/* HMAC-SHA256                                                         */
/* ------------------------------------------------------------------ */
#define BLOCK 64u

/* Calcule HMAC(key, p1 || p2) ; p2 peut être NULL/0. */
static void hmac2(const uint8_t *key, size_t key_len,
                  const uint8_t *p1, size_t l1,
                  const uint8_t *p2, size_t l2,
                  uint8_t out[HKDF_HASH_LEN])
{
    uint8_t k[BLOCK] = {0}, pad[BLOCK], inner[HKDF_HASH_LEN];
    sha256_ctx c;

    if (key_len > BLOCK) {
        sha256_init(&c);
        sha256_update(&c, key, key_len);
        sha256_final(&c, k);
    } else if (key_len > 0) {
        memcpy(k, key, key_len);
    }

    for (size_t i = 0; i < BLOCK; i++) pad[i] = k[i] ^ 0x36;
    sha256_init(&c);
    sha256_update(&c, pad, BLOCK);
    sha256_update(&c, p1, l1);
    sha256_update(&c, p2, l2);
    sha256_final(&c, inner);

    for (size_t i = 0; i < BLOCK; i++) pad[i] = k[i] ^ 0x5c;
    sha256_init(&c);
    sha256_update(&c, pad, BLOCK);
    sha256_update(&c, inner, HKDF_HASH_LEN);
    sha256_final(&c, out);

    hkdf_secure_zero(k, sizeof k);
    hkdf_secure_zero(pad, sizeof pad);
    hkdf_secure_zero(inner, sizeof inner);
}

hkdf_status hmac_sha256(const uint8_t *key, size_t key_len,
                        const uint8_t *msg, size_t msg_len,
                        uint8_t out[HKDF_HASH_LEN])
{
    if (!out || (!key && key_len) || (!msg && msg_len)) return HKDF_ERR_PARAM;
    hmac2(key, key_len, msg, msg_len, NULL, 0, out);
    return HKDF_OK;
}

hkdf_status hmac_sha256_verify(const uint8_t *key, size_t key_len,
                               const uint8_t *msg, size_t msg_len,
                               const uint8_t *tag, size_t tag_len)
{
    uint8_t calc[HKDF_HASH_LEN];
    hkdf_status st;

    if (!tag || tag_len == 0 || tag_len > HKDF_HASH_LEN) return HKDF_ERR_PARAM;
    st = hmac_sha256(key, key_len, msg, msg_len, calc);
    if (st != HKDF_OK) return st;

    st = (hkdf_ct_memcmp(calc, tag, tag_len) == 0) ? HKDF_OK : HKDF_ERR_VERIFY;
    hkdf_secure_zero(calc, sizeof calc);
    return st;
}

/* ------------------------------------------------------------------ */
/* HKDF                                                                */
/* ------------------------------------------------------------------ */
hkdf_status hkdf_extract(const uint8_t *salt, size_t salt_len,
                         const uint8_t *ikm, size_t ikm_len,
                         uint8_t prk[HKDF_HASH_LEN])
{
    static const uint8_t zeros[HKDF_HASH_LEN] = {0};
    if (!prk || (!salt && salt_len) || (!ikm && ikm_len)) return HKDF_ERR_PARAM;
    if (!salt || salt_len == 0) { salt = zeros; salt_len = HKDF_HASH_LEN; }
    /* PRK = HMAC(salt, IKM) */
    hmac2(salt, salt_len, ikm, ikm_len, NULL, 0, prk);
    return HKDF_OK;
}

hkdf_status hkdf_expand(const uint8_t prk[HKDF_HASH_LEN],
                        const uint8_t *info, size_t info_len,
                        uint8_t *okm, size_t okm_len)
{
    uint8_t t[HKDF_HASH_LEN];
    uint8_t buf[HKDF_HASH_LEN + HKDF_MAX_INFO_LEN + 1];
    size_t  t_len = 0, done = 0;
    uint8_t counter = 1;

    if (!prk || !okm || (!info && info_len)) return HKDF_ERR_PARAM;
    if (okm_len > HKDF_MAX_OKM_LEN) return HKDF_ERR_LENGTH;
    if (info_len > HKDF_MAX_INFO_LEN) return HKDF_ERR_PARAM;

    /* T(i) = HMAC(PRK, T(i-1) || info || i) */
    while (done < okm_len) {
        size_t n = 0, take;
        if (t_len) { memcpy(buf, t, t_len); n = t_len; }
        if (info_len) { memcpy(buf + n, info, info_len); n += info_len; }
        buf[n++] = counter;

        hmac2(prk, HKDF_HASH_LEN, buf, n, NULL, 0, t);
        t_len = HKDF_HASH_LEN;

        take = okm_len - done;
        if (take > HKDF_HASH_LEN) take = HKDF_HASH_LEN;
        memcpy(okm + done, t, take);
        done += take;
        counter++;
    }
    hkdf_secure_zero(t, sizeof t);
    hkdf_secure_zero(buf, sizeof buf);
    return HKDF_OK;
}

hkdf_status hkdf(const uint8_t *salt, size_t salt_len,
                 const uint8_t *ikm, size_t ikm_len,
                 const uint8_t *info, size_t info_len,
                 uint8_t *okm, size_t okm_len)
{
    uint8_t prk[HKDF_HASH_LEN];
    hkdf_status st = hkdf_extract(salt, salt_len, ikm, ikm_len, prk);
    if (st == HKDF_OK) st = hkdf_expand(prk, info, info_len, okm, okm_len);
    hkdf_secure_zero(prk, sizeof prk);
    return st;
}

/* ------------------------------------------------------------------ */
/* Contexte nonce/info prédéfinis                                      */
/* ------------------------------------------------------------------ */
hkdf_status hkdf_ctx_init(hkdf_ctx *ctx,
                          const uint8_t *nonce, size_t nonce_len,
                          const uint8_t *info,  size_t info_len)
{
    if (!ctx || (!nonce && nonce_len) || (!info && info_len)) return HKDF_ERR_PARAM;
    if (nonce_len > HKDF_MAX_NONCE_LEN || info_len > HKDF_MAX_INFO_LEN)
        return HKDF_ERR_PARAM;
    memset(ctx, 0, sizeof *ctx);
    if (nonce_len) memcpy(ctx->nonce, nonce, nonce_len);
    if (info_len)  memcpy(ctx->info,  info,  info_len);
    ctx->nonce_len = nonce_len;
    ctx->info_len  = info_len;
    return HKDF_OK;
}

hkdf_status hkdf_ctx_derive(const hkdf_ctx *ctx,
                            const uint8_t *ikm, size_t ikm_len,
                            uint8_t *okm, size_t okm_len)
{
    if (!ctx) return HKDF_ERR_PARAM;
    return hkdf(ctx->nonce, ctx->nonce_len, ikm, ikm_len,
                ctx->info, ctx->info_len, okm, okm_len);
}

hkdf_status hkdf_ctx_compute_tag(const hkdf_ctx *ctx,
                                 const uint8_t *ikm, size_t ikm_len,
                                 const uint8_t *msg, size_t msg_len,
                                 uint8_t tag[HKDF_HASH_LEN])
{
    uint8_t mk[HKDF_HASH_LEN];
    hkdf_status st = hkdf_ctx_derive(ctx, ikm, ikm_len, mk, sizeof mk);
    if (st == HKDF_OK) st = hmac_sha256(mk, sizeof mk, msg, msg_len, tag);
    hkdf_secure_zero(mk, sizeof mk);
    return st;
}

hkdf_status hkdf_ctx_verify_tag(const hkdf_ctx *ctx,
                                const uint8_t *ikm, size_t ikm_len,
                                const uint8_t *msg, size_t msg_len,
                                const uint8_t *tag, size_t tag_len)
{
    uint8_t mk[HKDF_HASH_LEN];
    hkdf_status st = hkdf_ctx_derive(ctx, ikm, ikm_len, mk, sizeof mk);
    if (st == HKDF_OK) st = hmac_sha256_verify(mk, sizeof mk, msg, msg_len, tag, tag_len);
    hkdf_secure_zero(mk, sizeof mk);
    return st;
}
