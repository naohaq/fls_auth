/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file mac_fls.c
 * 
 */

#include <winsock2.h>
#include <windows.h>
#include <winscard.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <assert.h>

#include <gcrypt.h>
#include "mac_fls.h"

static void
reverse_byteorder8(const uint8_t * src, uint8_t * dst)
{
	dst[0] = src[7];
	dst[1] = src[6];
	dst[2] = src[5];
	dst[3] = src[4];
	dst[4] = src[3];
	dst[5] = src[2];
	dst[6] = src[1];
	dst[7] = src[0];
}

static void
xor_chunk8(const uint8_t * src1, const uint8_t * src2, uint8_t * dst)
{
	int32_t idx;
	for (idx=0; idx<8; idx+=1) {
		dst[idx] = src1[idx] ^ src2[idx];
	}
}

int32_t
gcrypt_init( void )
{
	int32_t ret = 0;
	/* gcry_error_t err = 0; */

	if (! gcry_check_version(GCRYPT_VERSION)) {
		_ftprintf_s(stderr, _T("gcrypt library version mismatch.\n"));
		ret = -1;
		goto ERR_EXIT;
	}

	gcry_control(GCRYCTL_DISABLE_SECMEM, 0);

	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

ERR_EXIT:
	return ret;
}

int32_t
FeliCa_MAC_Init(MAC_FLS_t * ctx, const uint8_t * id_m, const uint8_t * ck1, const uint8_t * ck2)
{
	int32_t ret = 0;
	gcry_error_t err = 0;
	int32_t idx;
	uint8_t key_3des[24];

	assert(ctx != NULL);
	assert(id_m != NULL);
	assert(ck1 != NULL);
	assert(ck2 != NULL);

	memset(ctx, 0, sizeof(MAC_FLS_t));

	for (idx=0; idx<8; idx+=1) {
		ctx->id_m[idx] = id_m[idx];
	}

	for (idx=0; idx<8; idx+=1) {
		ctx->ck1[idx] = ck1[idx];
	}

	for (idx=0; idx<8; idx+=1) {
		ctx->ck2[idx] = ck2[idx];
	}

	for (idx=0; idx<8; idx+=1) {
		key_3des[idx     ] = ctx->ck1[7 - idx];
		key_3des[idx +  8] = ctx->ck2[7 - idx];
		key_3des[idx + 16] = ctx->ck1[7 - idx];
	}

	err = gcry_cipher_open(&(ctx->hd_ck), GCRY_CIPHER_3DES, GCRY_CIPHER_MODE_ECB, 0);
	if (err != 0) {
		_ftprintf_s(stderr, _T("gcry_cipher_open: %s\n"), gcry_strerror(err));
		ret = -1;
		goto ERR_EXIT;
	}

	err = gcry_cipher_setkey(ctx->hd_ck, key_3des, 24);
	if (err != 0) {
		_ftprintf_s(stderr, _T("gcry_cipher_setkey: %s\n"), gcry_strerror(err));
		ret = -1;
		goto ERR_EXIT;
	}

	ctx->ck_valid_p = 1;

ERR_EXIT:
	memset(key_3des, 0, 24);
	return ret;
}

int32_t
FeliCa_MAC_Finish(MAC_FLS_t * ctx)
{
	int32_t ret = 0;
	
	assert(ctx != NULL);

	if (! ctx->ck_valid_p) {
		_ftprintf_s(stderr, _T("%s: invalid state transition\n"), __func__);
		ret = -1;
		goto ERR_EXIT;
	}

	gcry_cipher_close(ctx->hd_ck);

	memset(ctx->ck1, 0, 8);
	memset(ctx->ck2, 0, 8);

	ctx->ck_valid_p = 0;

ERR_EXIT:
	return ret;
}

int32_t
FeliCa_MAC_StartSession(MAC_FLS_t * ctx)
{
	int32_t ret = 0;
	gcry_error_t err = 0;
	uint8_t rc1_buf[8];
	uint8_t rc2_buf[8];
	uint8_t sk1_buf[8];
	uint8_t sk2_buf[8];
	uint8_t ir1_buf[8];
	int32_t idx;
	uint8_t key_3des[24];

	assert(ctx != NULL);

	if (! ctx->ck_valid_p) {
		_ftprintf_s(stderr, _T("%s: invalid state transition\n"), __func__);
		ret = -1;
		goto ERR_EXIT;
	}

	gcry_randomize(ctx->rc1, 8, GCRY_STRONG_RANDOM);
	gcry_randomize(ctx->rc2, 8, GCRY_STRONG_RANDOM);

	reverse_byteorder8(ctx->rc1, rc1_buf);
	reverse_byteorder8(ctx->rc2, rc2_buf);

	err = gcry_cipher_encrypt(ctx->hd_ck, sk1_buf, 8, rc1_buf, 8);
	if (err != 0) {
		_ftprintf_s(stderr, _T("gcry_cipher_encrypt: %s\n"), gcry_strerror(err));
		ret = -1;
		goto ERR_EXIT;
	}

	xor_chunk8(sk1_buf, rc2_buf, ir1_buf);

	err = gcry_cipher_encrypt(ctx->hd_ck, sk2_buf, 8, ir1_buf, 8);
	if (err != 0) {
		_ftprintf_s(stderr, _T("gcry_cipher_encrypt: %s\n"), gcry_strerror(err));
		ret = -1;
		goto ERR_EXIT;
	}

	/* The value of session key should not be stored. */
	/* reverse_byteorder8(sk1_buf, ctx->sk1); */
	/* reverse_byteorder8(sk2_buf, ctx->sk2); */

	for (idx=0; idx<8; idx+=1) {
		key_3des[idx     ] = sk1_buf[idx];
		key_3des[idx +  8] = sk2_buf[idx];
		key_3des[idx + 16] = sk1_buf[idx];
	}

	err = gcry_cipher_open(&(ctx->hd_sk), GCRY_CIPHER_3DES, GCRY_CIPHER_MODE_ECB, 0);
	if (err != 0) {
		_ftprintf_s(stderr, _T("gcry_cipher_open: %s\n"), gcry_strerror(err));
		ret = -1;
		goto ERR_EXIT;
	}

	err = gcry_cipher_setkey(ctx->hd_sk, key_3des, 24);
	if (err != 0) {
		_ftprintf_s(stderr, _T("gcry_cipher_setkey: %s\n"), gcry_strerror(err));
		ret = -1;
		goto ERR_EXIT;
	}

	ctx->sk_valid_p = 1;

ERR_EXIT:
	memset(rc1_buf, 0, 8);
	memset(rc2_buf, 0, 8);
	memset(sk1_buf, 0, 8);
	memset(sk2_buf, 0, 8);
	memset(ir1_buf, 0, 8);
	memset(key_3des, 0, 24);
	return ret;
}

int32_t
FeliCa_MAC_EndSession(MAC_FLS_t * ctx)
{
	int32_t ret = 0;

	assert(ctx != NULL);

	if (! ctx->sk_valid_p) {
		_ftprintf_s(stderr, _T("%s: invalid state transition\n"), __func__);
		ret = -1;
		goto ERR_EXIT;
	}

	gcry_cipher_close(ctx->hd_sk);

	memset(ctx->rc1, 0, 8);
	memset(ctx->rc2, 0, 8);

	/* The value of session key should not be stored. */
	/* memset(ctx->sk1, 0, 8); */
	/* memset(ctx->sk2, 0, 8); */

	ctx->sk_valid_p = 0;

ERR_EXIT:
	return ret;
}

static int32_t
Calc_MAC_Step(MAC_FLS_t * ctx, const uint8_t * vec0_r, const uint8_t * ir0, uint8_t * mac)
{
	int32_t ret = 0;
	gcry_error_t err = 0;
	uint8_t enc_buf0[8];
	uint8_t enc_buf1[8];

	assert(ctx != NULL);
	assert(vec0_r != NULL);
	assert(ir0 != NULL);
	assert(mac != NULL);
	assert(ctx->sk_valid_p);

	reverse_byteorder8(vec0_r, enc_buf0);
	xor_chunk8(enc_buf0, ir0, enc_buf1);

	err = gcry_cipher_encrypt(ctx->hd_sk, mac, 8, enc_buf1, 8);
	if (err != 0) {
		_ftprintf_s(stderr, _T("gcry_cipher_encrypt: %s\n"), gcry_strerror(err));
		ret = -1;
	}

	return ret;
}


int32_t
FeliCa_MAC_CheckAuth(MAC_FLS_t * ctx, const uint8_t * ckv, const uint8_t * mac)
{
	int32_t ret = 0;
	uint8_t ivec[8] = {0x82, 0x00, 0x86, 0x00, 0x91, 0x00, 0xff, 0xff};
	uint8_t enc_buf0[8];
	uint8_t enc_buf1[8];
	uint8_t enc_buf2[8];
	int32_t idx;

	assert(ctx != NULL);

	if (! ctx->sk_valid_p) {
		_ftprintf_s(stderr, _T("%s: invalid state transition\n"), __func__);
		ret = -1;
		goto ERR_EXIT;
	}

	reverse_byteorder8(ctx->rc1, enc_buf0);
	if (Calc_MAC_Step(ctx, ivec, enc_buf0, enc_buf1) < 0) {
		ret = -1;
		goto ERR_EXIT;
	}

	if (Calc_MAC_Step(ctx, ctx->id_m, enc_buf1, enc_buf0) < 0) {
		ret = -1;
		goto ERR_EXIT;
	}

	memset(enc_buf2, 0, 8);
	if (Calc_MAC_Step(ctx, enc_buf2, enc_buf0, enc_buf1) < 0) {
		ret = -1;
		goto ERR_EXIT;
	}

	if (Calc_MAC_Step(ctx, &(ckv[0]), enc_buf1, enc_buf0) < 0) {
		ret = -1;
		goto ERR_EXIT;
	}

	if (Calc_MAC_Step(ctx, &(ckv[8]), enc_buf0, enc_buf1) < 0) {
		ret = -1;
		goto ERR_EXIT;
	}

	reverse_byteorder8(enc_buf1, enc_buf2);

	{
		int32_t match_p = 1;
		for (idx=0; idx<8 && match_p; idx+=1) {
			if (enc_buf2[idx] != mac[idx]) {
				match_p = 0;
			}
		}

		ret = match_p;
	}

ERR_EXIT:
	return ret;
}


/*
 * Local Variables:
 * indent-tabs-mode: t
 * tab-width: 4
 * End:
 */
