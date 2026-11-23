/*
 * hkdf.h - HKDF-SHA256 (RFC 5869) + HMAC-SHA256, bibliothèque C autonome.
 *
 * - Aucune dépendance externe (SHA-256 / HMAC inclus).
 * - Dérivation de clef avec nonce/salt et info prédéfinis (contexte hkdf_ctx).
 * - Vérification de tag HMAC en temps constant (tags tronqués supportés).
 */
#ifndef HKDF_H
#define HKDF_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HKDF_HASH_LEN      32u                  /* taille de sortie SHA-256 */
#define HKDF_MAX_OKM_LEN   (255u * HKDF_HASH_LEN)
#define HKDF_MAX_NONCE_LEN 64u                  /* taille max du nonce prédéfini */
#define HKDF_MAX_INFO_LEN  128u                 /* taille max de l'info prédéfinie */

typedef enum {
    HKDF_OK         =  0,
    HKDF_ERR_PARAM  = -1,   /* pointeur NULL / taille invalide */
    HKDF_ERR_LENGTH = -2,   /* okm_len > 255 * HashLen */
    HKDF_ERR_VERIFY = -3    /* tag HMAC invalide */
} hkdf_status;

/* ---------- SHA-256 incrémental ---------- */
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t  buf[64];
    size_t   buflen;
} sha256_ctx;

void sha256_init(sha256_ctx *c);
void sha256_update(sha256_ctx *c, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx *c, uint8_t out[HKDF_HASH_LEN]);

/* ---------- HMAC-SHA256 ---------- */
hkdf_status hmac_sha256(const uint8_t *key, size_t key_len,
                        const uint8_t *msg, size_t msg_len,
                        uint8_t out[HKDF_HASH_LEN]);

/*
 * Vérifie un tag HMAC-SHA256 en temps constant.
 * tag_len peut être < 32 (tag tronqué), minimum 1.
 * Retourne HKDF_OK si valide, HKDF_ERR_VERIFY sinon.
 */
hkdf_status hmac_sha256_verify(const uint8_t *key, size_t key_len,
                               const uint8_t *msg, size_t msg_len,
                               const uint8_t *tag, size_t tag_len);

/* ---------- HKDF (RFC 5869) ---------- */
hkdf_status hkdf_extract(const uint8_t *salt, size_t salt_len,
                         const uint8_t *ikm, size_t ikm_len,
                         uint8_t prk[HKDF_HASH_LEN]);

hkdf_status hkdf_expand(const uint8_t prk[HKDF_HASH_LEN],
                        const uint8_t *info, size_t info_len,
                        uint8_t *okm, size_t okm_len);

/* Extract + Expand en un appel */
hkdf_status hkdf(const uint8_t *salt, size_t salt_len,
                 const uint8_t *ikm, size_t ikm_len,
                 const uint8_t *info, size_t info_len,
                 uint8_t *okm, size_t okm_len);

/* ---------- Contexte avec nonce (salt) et info prédéfinis ---------- */
typedef struct {
    uint8_t nonce[HKDF_MAX_NONCE_LEN];
    size_t  nonce_len;
    uint8_t info[HKDF_MAX_INFO_LEN];
    size_t  info_len;
} hkdf_ctx;

/* Fixe le nonce (salt) et le contexte d'application (info). Copie les données. */
hkdf_status hkdf_ctx_init(hkdf_ctx *ctx,
                          const uint8_t *nonce, size_t nonce_len,
                          const uint8_t *info,  size_t info_len);

/* Dérive okm_len octets depuis ikm avec le nonce/info du contexte. */
hkdf_status hkdf_ctx_derive(const hkdf_ctx *ctx,
                            const uint8_t *ikm, size_t ikm_len,
                            uint8_t *okm, size_t okm_len);

/*
 * Dérive une clef MAC (32 octets) depuis ikm puis calcule le tag
 * HMAC-SHA256(msg). La clef intermédiaire est effacée.
 */
hkdf_status hkdf_ctx_compute_tag(const hkdf_ctx *ctx,
                                 const uint8_t *ikm, size_t ikm_len,
                                 const uint8_t *msg, size_t msg_len,
                                 uint8_t tag[HKDF_HASH_LEN]);

/*
 * Dérive la clef MAC depuis ikm puis vérifie le tag (temps constant).
 * tag_len : 1..32.
 */
hkdf_status hkdf_ctx_verify_tag(const hkdf_ctx *ctx,
                                const uint8_t *ikm, size_t ikm_len,
                                const uint8_t *msg, size_t msg_len,
                                const uint8_t *tag, size_t tag_len);

/* ---------- Utilitaires ---------- */
/* Comparaison en temps constant : retourne 0 si égaux. */
int  hkdf_ct_memcmp(const void *a, const void *b, size_t len);
/* Effacement mémoire non optimisable par le compilateur. */
void hkdf_secure_zero(void *p, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* HKDF_H */
