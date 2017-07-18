/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file key_db.h
 *
 */

#ifndef FELICA_KEY_DB_H_
#define FELICA_KEY_DB_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int32_t FeliCa_Lookup_KeyDB(const uint8_t * idm, uint8_t * ck1, uint8_t * ck2);

#ifdef __cplusplus
}
#endif
#endif /* FELICA_KEY_DB_H_ */

/*
 * Local Variables:
 * indent-tabs-mode: t
 * tab-width: 4
 * End:
 */
