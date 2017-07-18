/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file WriteCK.c
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
	int32_t force_p = 0;
	uint8_t cardType = 0xff;
	SCardState_t ctx = {0};
	GetUID_Resp_t idm = {0};
	uint8_t ck1[8];
	uint8_t ck2[8];
	uint32_t wcnt = 0xffffffff;
	TCHAR * optstr_force = _T("--force");
	TCHAR * pszExpectedReaderName = _T("Sony FeliCa Port/PaSoRi 3.0");

	if (argc > 1) {
		if (_tcsncmp(optstr_force, argv[1], _tcslen(optstr_force)) == 0) {
			force_p = 1;
		}
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

	CHECK_ERROR(FeliCa_SelectFile(&ctx, FELICA_SVC00_RO));
	CHECK_ERROR(FeliCa_ReadWCNT(&ctx, &wcnt));
	_ftprintf_s(stdout, _T("WCNT: %06x\n"), wcnt);
	_ftprintf_s(stdout, _T("\n"));

	if (! force_p) {
		_ftprintf_s(stderr, _T("If you REALLY want to update CK1/CK2, invoke with '%s'\n"), optstr_force);
	}
	else {
		_ftprintf_s(stdout, _T("Writing CK1/CK2...\n"));

		CHECK_ERROR(FeliCa_SelectFile(&ctx, FELICA_SVC00_RW));
		CHECK_ERROR(FeliCa_WriteCardKey(&ctx, ck1, ck2));
		
		CHECK_ERROR(FeliCa_SelectFile(&ctx, FELICA_SVC00_RO));
		CHECK_ERROR(FeliCa_ReadWCNT(&ctx, &wcnt));
		_ftprintf_s(stdout, _T("WCNT: %06x\n"), wcnt);	
		_ftprintf_s(stdout, _T("\n"));

		_ftprintf_s(stdout, _T("Done.\n"));
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
