/* -*- mode: c; coding: utf-8-unix -*- */
/**
 * @file key_db.c
 * 
 */

#include <winsock2.h>
#include <windows.h>
#include <winscard.h>

#include <stdint.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>

struct ST_FLS_KEY_RECORD {
	uint8_t idm[8];
	uint8_t ck1[8];
	uint8_t ck2[8];
};

typedef struct ST_FLS_KEY_RECORD FLS_Key_Record_t;

static FLS_Key_Record_t s_keydb[] = {
#include "key_db_secrets.c"
	{
		/* End of Table  */
		{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
		{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
		{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}
	}
};

int32_t
FeliCa_Lookup_KeyDB(const uint8_t * idm, uint8_t * ck1, uint8_t * ck2)
{
	int32_t found_p = 0;
	int32_t j = 0;

	do {
		int32_t k;
		int32_t eot = 1;

		for (k=0; k<8 && eot; k+=1) {
			if (s_keydb[j].idm[k] != 0xff) {
				eot = 0;
			}
		}

		if (eot) {
			break;
		}

		found_p = 1;
		for (k=0; k<8 && found_p; k+=1) {
			if (s_keydb[j].idm[k] != idm[k]) {
				found_p = 0;
			}
		}

		if (!found_p) {
			j += 1;
		}
	} while (!found_p);

	if (found_p) {
		int32_t k;
		for (k=0; k<8; k+=1) {
			ck1[k] = s_keydb[j].ck1[k];
			ck2[k] = s_keydb[j].ck2[k];
		}
	}

	return found_p;
}


/*
 * Local Variables:
 * indent-tabs-mode: t
 * tab-width: 4
 * End:
 */
