/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file scard_common.h
 *
 */

#ifndef SCARD_COMMON_H_
#define SCARD_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PCSC_TRANS_BUFF_LEN	(262)
#define PCSC_RECV_BUFF_LEN	(262)

struct ST_SCARD_STATE {
	SCARDCONTEXT hSCContext;
	SCARDHANDLE  hCard;
	DWORD        dwActiveProtocol;
	LPTSTR       lpszReaderName;
};

typedef struct ST_SCARD_STATE SCardState_t;

typedef struct {
	INT		iLength;
	BYTE	bCommand[PCSC_TRANS_BUFF_LEN];
} SENDCOMM_t;

extern uint32_t TransmitCommand(SCardState_t * ctx, SENDCOMM_t * cmd, uint8_t * res, uint32_t * resSize);
extern int EstablishContext(SCardState_t * ctx);
extern int FindReader(SCardState_t * ctx, TCHAR * pszExpectedReaderName);
extern int ConnectToCard(SCardState_t * ctx);
extern void CleanupContext(SCardState_t * ctx);

extern int GetCardType(SCardState_t * ctx, uint8_t * pCardType);

#ifdef __cplusplus
}
#endif
#endif /* SCARD_COMMON_H_ */

/*
 * Local Variables:
 * indent-tabs-mode: t
 * tab-width: 4
 * End:
 */
