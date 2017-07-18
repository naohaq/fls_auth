/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file VerifyMAC.c
 * 
 */

#include <winsock2.h>
#include <windows.h>
#include <winscard.h>

#include <stdint.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>

#include <gcrypt.h>
#include "scard_common.h"
#include "felica_lites.h"
#include "mac_fls.h"
#include "key_db.h"

#define CHECK_ERROR(x)  do {					\
		if ((x) != EXIT_SUCCESS) {				\
			ret = EXIT_FAILURE;					\
			goto ERR_EXIT;						\
		}										\
	} while(0)

static int
PrintID(const TCHAR * name, const GetUID_Resp_t * p)
{
	return _ftprintf_s(stdout, _T("%s: %02x%02x-%02x%02x%02x%02x%02x%02x\n"), name, p->bUID[0], p->bUID[1], p->bUID[2], p->bUID[3], p->bUID[4], p->bUID[5], p->bUID[6], p->bUID[7]);
}


int
main(int argc, _TCHAR * argv[])
{
	int ret = EXIT_SUCCESS;
	uint8_t cardType = 0xff;
	SCardState_t ctx = {0};
	GetUID_Resp_t idm = {0};
	GetUID_Resp_t pmm = {0};
	uint8_t ck1[8];
	uint8_t ck2[8];
	uint32_t wcnt = 0xffffffff;
	MAC_FLS_t mac_ctx = {0};
	TCHAR * pszExpectedReaderName = _T("Sony FeliCa Port/PaSoRi 3.0");

	if (gcrypt_init( ) < 0) {
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	CHECK_ERROR(EstablishContext(&ctx));
	CHECK_ERROR(FindReader(&ctx, pszExpectedReaderName));
	CHECK_ERROR(ConnectToCard(&ctx));
	CHECK_ERROR(GetCardType(&ctx, &cardType));

	if (cardType != 0x04) {
		_ftprintf_s(stdout, _T("Not a FeliCa.\n"));
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	CHECK_ERROR(FeliCa_GetIDm(&ctx, &idm));
	PrintID(_T("IDm"), &idm);

	if (! FeliCa_Lookup_KeyDB(idm.bUID, ck1, ck2)) {
		_ftprintf_s(stdout, _T("IDm not found.\n"));
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	if (FeliCa_MAC_Init(&mac_ctx, idm.bUID, ck1, ck2) < 0) {
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	CHECK_ERROR(FeliCa_GetPMm(&ctx, &pmm));
	PrintID(_T("PMm"), &pmm);

	CHECK_ERROR(FeliCa_SelectFile(&ctx, FELICA_SVC00_RO));
	CHECK_ERROR(FeliCa_ReadWCNT(&ctx, &wcnt));
	_ftprintf_s(stdout, _T("WCNT: %06x\n"), wcnt);	

	if (FeliCa_MAC_StartSession(&mac_ctx) < 0) {
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	CHECK_ERROR(FeliCa_SelectFile(&ctx, FELICA_SVC00_RW));
	CHECK_ERROR(FeliCa_WriteRC(&ctx, mac_ctx.rc1, mac_ctx.rc2));

	CHECK_ERROR(FeliCa_SelectFile(&ctx, FELICA_SVC00_RO));

	{
		uint8_t ckv[16];
		uint8_t mac_a[16];
		int32_t match_p;
		CHECK_ERROR(FeliCa_ReadIDwithMAC(&ctx, ckv, mac_a));

		match_p = FeliCa_MAC_CheckAuth(&mac_ctx, ckv, mac_a);
		if (match_p < 0) {
			ret = EXIT_FAILURE;
			goto ERR_EXIT;
		}

		if (match_p) {
			_ftprintf_s(stdout, _T("Auth SUCCESS!\n"));
		}
		else {
			_ftprintf_s(stdout, _T("Auth FAILURE!\n"));
		}
	}

	if (FeliCa_MAC_EndSession(&mac_ctx) < 0) {
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	if (FeliCa_MAC_Finish(&mac_ctx) < 0) {
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

ERR_EXIT:
	CleanupContext(&ctx);
	return ret;
}

/*
 * Local Variables:
 * indent-tabs-mode: t
 * tab-width: 4
 * End:
 */
