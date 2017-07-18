/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file felica_lites.c
 * 
 */

#include <windows.h>
#include <winscard.h>

#include <stdint.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>

#include "scard_common.h"
#include "felica_lites.h"

int
FeliCa_GetIDm(SCardState_t * ctx, GetUID_Resp_t * pRes)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;
	SENDCOMM_t cmd = {5, {0xff, 0xca, 0x00, 0x00, 8}};
	uint32_t responseSize = sizeof(GetUID_Resp_t);

	assert(ctx != NULL);
	assert(pRes != NULL);

	result = TransmitCommand(ctx, &cmd, (uint8_t *)pRes, &responseSize);
	if (result != SCARD_S_SUCCESS) {
		_ftprintf_s(stdout, _T("SCardTransmit Error.\nErrorCode:0x%08X\n"), result);

		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

ERR_EXIT:
	return ret;
}

int
FeliCa_GetPMm(SCardState_t * ctx, GetUID_Resp_t * pRes)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;
	SENDCOMM_t cmd = {5, {0xff, 0xca, 0x01, 0x00, 8}};
	uint32_t responseSize = sizeof(GetUID_Resp_t);

	assert(ctx != NULL);
	assert(pRes != NULL);

	result = TransmitCommand(ctx, &cmd, (uint8_t *)pRes, &responseSize);
	if (result != SCARD_S_SUCCESS) {
		_ftprintf_s(stdout, _T("SCardTransmit Error.\nErrorCode:0x%08X\n"), result);

		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

ERR_EXIT:
	return ret;
}

int
FeliCa_ReadWCNT(SCardState_t * ctx, uint32_t * pWCnt)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;
	uint8_t res[16+2];
	SENDCOMM_t cmd = {5, {0xff, 0xb0, 0x00, 0x90, 0x00}};
	uint32_t responseSize = sizeof(res);
	uint8_t sw1;
	uint8_t sw2;

	assert(ctx != NULL);
	assert(pWCnt != NULL);

	result = TransmitCommand(ctx, &cmd, res, &responseSize);
	if (result != SCARD_S_SUCCESS) {
		_ftprintf_s(stdout, _T("SCardTransmit Error.\nErrorCode:0x%08X\n"), result);

		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	sw1 = res[responseSize-2];
	sw2 = res[responseSize-1];

	if (!(sw1 == 0x90 && sw2 == 0x00)) {
		_ftprintf_s(stdout, _T("Error: %02x %02x / %02x %02x\n"), sw1, sw2, res[0], res[1]);
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	*pWCnt = (((uint32_t)res[2])<<16) | (((uint32_t)res[1])<<8) | ((uint32_t)res[0]);

ERR_EXIT:
	return ret;
}

int
FeliCa_WriteRC(SCardState_t * ctx, uint8_t * rc1, uint8_t * rc2)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;
	uint8_t res[2];
	SENDCOMM_t cmd = {23, {0xff, 0xd6, 0x80, 0x01, 16+2,
						   0x80, 0x80,
						   rc1[0], rc1[1], rc1[2], rc1[3], rc1[4], rc1[5], rc1[6], rc1[7],
						   rc2[0], rc2[1], rc2[2], rc2[3], rc2[4], rc2[5], rc2[6], rc2[7]}};
	uint32_t responseSize = sizeof(res);

	result = TransmitCommand(ctx, &cmd, res, &responseSize);
	if (result != SCARD_S_SUCCESS) {
		_ftprintf_s(stdout, _T("SCardTransmit Error.\nErrorCode:0x%08X\n"), result);

		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	if (!(res[0] == 0x90 && res[1] == 0x00)) {
		_ftprintf_s(stdout, _T("WriteRC: %02x %02x\n"), res[0], res[1]);
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

ERR_EXIT:
	return ret;
}

int
FeliCa_WriteCardKey(SCardState_t * ctx, uint8_t * ck1, uint8_t * ck2)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;
	uint8_t res[2];
	SENDCOMM_t cmd = {23, {0xff, 0xd6, 0x80, 0x01, 16+2,
						   0x80, 0x87,
						   ck1[0], ck1[1], ck1[2], ck1[3], ck1[4], ck1[5], ck1[6], ck1[7],
						   ck2[0], ck2[1], ck2[2], ck2[3], ck2[4], ck2[5], ck2[6], ck2[7]}};
	uint32_t responseSize = sizeof(res);

	result = TransmitCommand(ctx, &cmd, res, &responseSize);
	if (result != SCARD_S_SUCCESS) {
		_ftprintf_s(stdout, _T("SCardTransmit Error.\nErrorCode:0x%08X\n"), result);

		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	if (!(res[0] == 0x90 && res[1] == 0x00)) {
		_ftprintf_s(stdout, _T("WriteCardKey: %02x %02x\n"), res[0], res[1]);
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

ERR_EXIT:
	return ret;
}

int
FeliCa_ReadIDwithMAC(SCardState_t * ctx, uint8_t * ckv, uint8_t * mac)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;
	uint8_t res[262];
	SENDCOMM_t cmd = {12, {0xff, 0xb0, 0x80, 0x03, 0x06, 0x80, 0x82, 0x80, 0x86, 0x80, 0x91, 16*3}};
	uint32_t responseSize = sizeof(res);
	int32_t idx;
	uint8_t sw1;
	uint8_t sw2;
	
	result = TransmitCommand(ctx, &cmd, res, &responseSize);
	if (result != SCARD_S_SUCCESS) {
		_ftprintf_s(stdout, _T("SCardTransmit Error.\nErrorCode:0x%08X\n"), result);

		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	sw1 = res[responseSize-2];
	sw2 = res[responseSize-1];

	if (!(sw1 == 0x90 && sw2 == 0x00)) {
		_ftprintf_s(stdout, _T("ReadIDwithMAC: %02x %02x\n"), sw1, sw2);
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	for (idx=0; idx<16; idx+=1) {
		ckv[idx] = res[idx + 16];
	}

	for (idx=0; idx<16; idx+=1) {
		mac[idx] = res[idx + 32];
	}	

ERR_EXIT:
	return ret;
}

int
FeliCa_SelectFile(SCardState_t * ctx, uint16_t svc_code)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;
	uint8_t res[2];
	uint8_t svc_lo = (uint8_t) (svc_code & 0x000ff);
	uint8_t svc_hi = (uint8_t)((svc_code & 0x0ff00) >> 8);
	SENDCOMM_t cmd = {7, {0xff, 0xa4, 0x00, 0x01, 0x02, svc_lo, svc_hi}};
	uint32_t responseSize = sizeof(res);

	result = TransmitCommand(ctx, &cmd, res, &responseSize);
	if (result != SCARD_S_SUCCESS) {
		_ftprintf_s(stdout, _T("SCardTransmit Error.\nErrorCode:0x%08X\n"), result);

		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	if (!(res[0] == 0x90 && res[1] == 0x00)) {
		_ftprintf_s(stdout, _T("SelectFile: %02x %02x\n"), res[0], res[1]);
		ret = EXIT_FAILURE;
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
