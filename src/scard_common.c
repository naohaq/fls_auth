/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file scard_common.c
 * 
 */

#include <windows.h>
#include <winscard.h>

#include <stdint.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>

#include "scard_common.h"

static SCARD_IO_REQUEST *
CardProtocol2PCI(DWORD dwProtocol)
{
	if (dwProtocol == SCARD_PROTOCOL_T0) {
		return (SCARD_IO_REQUEST *)SCARD_PCI_T0;
	} else if (dwProtocol == SCARD_PROTOCOL_T1) {
		return (SCARD_IO_REQUEST *)SCARD_PCI_T1;
	} else if (dwProtocol == SCARD_PROTOCOL_RAW) {
		return (SCARD_IO_REQUEST *)SCARD_PCI_RAW;
	} else if (dwProtocol == SCARD_PROTOCOL_UNDEFINED) {
		assert(0);
		return NULL;
	}

	return (SCARD_IO_REQUEST *)SCARD_PCI_T1;
}

uint32_t
TransmitCommand(SCardState_t * ctx, SENDCOMM_t * cmd, uint8_t * res, uint32_t * resSize)
{
	LONG lResult;

	assert(ctx != NULL);
	assert(cmd != NULL);
	assert(res != NULL);
	assert(resSize != NULL);

	lResult = SCardTransmit(ctx->hCard, CardProtocol2PCI(ctx->dwActiveProtocol), cmd->bCommand, cmd->iLength, NULL, res, (LPDWORD)resSize);

	return (uint32_t)lResult;
}

int
EstablishContext(SCardState_t * ctx)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;

	assert(ctx != NULL);

	result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &(ctx->hSCContext));
	if (result != SCARD_S_SUCCESS) {
		if (result == SCARD_E_NO_SERVICE) {
			_ftprintf_s(stdout, _T("Smart Card Servise is not Started.\n"));
		} else {
			_ftprintf_s(stdout, _T("SCardEstablishContext Error.\nErrorCode %08X\n"), result);
		}

		ret = EXIT_FAILURE;
	}

	return ret;
}

int
FindReader(SCardState_t * ctx, TCHAR * pszExpectedReaderName)
{
	int ret = EXIT_SUCCESS;
	DWORD dwAutoAllocate = SCARD_AUTOALLOCATE;
	uint32_t result;

	assert(ctx != NULL);

	result = SCardListReaders(ctx->hSCContext, NULL, (LPTSTR)&ctx->lpszReaderName, &dwAutoAllocate);

	if (result != SCARD_S_SUCCESS) {
		if (result == SCARD_E_NO_READERS_AVAILABLE)  {
			_ftprintf_s(stdout, _T("Reader/Writer is not Found.\n"));
		} else {
			_ftprintf_s(stdout, _T("SCardListReaders Error.\nErrorCode %08X\n"), result);
		}
		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	if (_tcsncmp(pszExpectedReaderName, ctx->lpszReaderName, _tcslen(pszExpectedReaderName)) != 0) {
		_ftprintf_s(stdout, _T("Reader/Writer is not Found.\n"));
		ret = EXIT_FAILURE;
	}

ERR_EXIT:
	return ret;
}

int
ConnectToCard(SCardState_t * ctx)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;

	assert(ctx != NULL);

	result = SCardConnect(ctx->hSCContext, ctx->lpszReaderName, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &ctx->hCard, &ctx->dwActiveProtocol);
	if (result != SCARD_S_SUCCESS) {
		if (result == SCARD_W_REMOVED_CARD) {
			_ftprintf_s(stdout, _T("Card is not Found.\n"));
		} else {
			_ftprintf_s(stdout, _T("SCardConnect Error.\nErrorCode %08X\n"), result);
		}

		ret = EXIT_FAILURE;
	}

	return ret;
}

int
GetCardType(SCardState_t * ctx, uint8_t * pCardType)
{
	int ret = EXIT_SUCCESS;
	uint32_t result;
	uint8_t res[3];
	SENDCOMM_t cmd = {5, {0xff, 0xca, 0xf3, 0x00, 0x01}};
	uint32_t responseSize = sizeof(res);
	
	assert(ctx != NULL);
	assert(pCardType != NULL);

	result = TransmitCommand(ctx, &cmd, res, &responseSize);
	if (result != SCARD_S_SUCCESS) {
		_ftprintf_s(stdout, _T("SCardTransmit Error.\nErrorCode:0x%08X\n"), result);

		ret = EXIT_FAILURE;
		goto ERR_EXIT;
	}

	_ftprintf_s(stdout, _T("Card Type: "));
	switch (res[0]) {
	case 0x04:
		_ftprintf_s(stdout, _T("FeliCa\n"));
		break;

	default:
		_ftprintf_s(stdout, _T("Unknown(%d)\n"), res[0]);
		break;
	}

	*pCardType = res[0];

ERR_EXIT:
	return ret;
}

void
CleanupContext(SCardState_t * ctx)
{
	assert(ctx != NULL);

	if (ctx->hCard != 0) {
		SCardDisconnect(ctx->hCard, SCARD_LEAVE_CARD);
		ctx->hCard = 0;
	}

	if ((ctx->hSCContext != 0) && (ctx->lpszReaderName != NULL)) {
		SCardFreeMemory(ctx->hSCContext, ctx->lpszReaderName);
		ctx->lpszReaderName = NULL;
	}

	if (ctx->hSCContext != 0) {
		SCardReleaseContext(ctx->hSCContext);
		ctx->hSCContext = 0;
	}
}




/*
 * Local Variables:
 * indent-tabs-mode: t
 * tab-width: 4
 * End:
 */
