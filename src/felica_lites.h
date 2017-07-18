/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file felica_lites.h
 *
 */

#ifndef FELICA_LITE_S_H_
#define FELICA_LITE_S_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t bUID[8];
	uint8_t bSW1;
	uint8_t bSW2;
} GetUID_Resp_t;

extern int FeliCa_GetIDm(SCardState_t * ctx, GetUID_Resp_t * pRes);
extern int FeliCa_GetPMm(SCardState_t * ctx, GetUID_Resp_t * pRes);
extern int FeliCa_SelectFile(SCardState_t * ctx, uint16_t svc_code);
extern int FeliCa_ReadWCNT(SCardState_t * ctx, uint32_t * pWCnt);
extern int FeliCa_WriteRC(SCardState_t * ctx, uint8_t * rc1, uint8_t * rc2);
extern int FeliCa_WriteCardKey(SCardState_t * ctx, uint8_t * ck1, uint8_t * ck2);
extern int FeliCa_ReadIDwithMAC(SCardState_t * ctx, uint8_t * ckv, uint8_t * mac);

#define FELICA_SVC00_RO  (0x000b)
#define FELICA_SVC00_RW  (0x0009)

#ifdef __cplusplus
}
#endif
#endif /* FELICA_LITE_S_H_ */

/*
 * Local Variables:
 * indent-tabs-mode: t
 * tab-width: 4
 * End:
 */
