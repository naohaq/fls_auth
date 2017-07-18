/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file mac_fls.h
 *
 */

#ifndef MAC_FELICA_LITE_S_H_
#define MAC_FELICA_LITE_S_H_

#ifdef __cplusplus
extern "C" {
#endif

struct ST_MAC_FELICA_LITE_S {
	uint8_t id_m[8];
	uint8_t ck1[8];
	uint8_t ck2[8];
	uint8_t rc1[8];
	uint8_t rc2[8];
	/* The value of session key should not be stored. */
	/* uint8_t sk1[8]; */
	/* uint8_t sk2[8]; */
	int32_t ck_valid_p;
	int32_t sk_valid_p;
	gcry_cipher_hd_t hd_ck;
	gcry_cipher_hd_t hd_sk;
};

typedef struct ST_MAC_FELICA_LITE_S MAC_FLS_t;

extern int32_t gcrypt_init( void );
extern int32_t FeliCa_MAC_Init(MAC_FLS_t * ctx, const uint8_t * id_m, const uint8_t * ck1, const uint8_t * ck2);
extern int32_t FeliCa_MAC_Finish(MAC_FLS_t * ctx);
extern int32_t FeliCa_MAC_StartSession(MAC_FLS_t * ctx);
extern int32_t FeliCa_MAC_EndSession(MAC_FLS_t * ctx);
extern int32_t FeliCa_MAC_CheckAuth(MAC_FLS_t * ctx, const uint8_t * ckv, const uint8_t * mac);

#ifdef __cplusplus
}
#endif
#endif /* MAC_FELICA_LITE_S_H_ */

/*
 * Local Variables:
 * indent-tabs-mode: t
 * tab-width: 4
 * End:
 */
