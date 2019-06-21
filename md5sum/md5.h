/*
 * Header file for md5.c source.
 * Ming Li(adagio.ming@gmail.com)
 * Date: 2018/11/16
 * Note - this source comes from RFC1321 for MD5 algorithm.
 */


/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.




Rivest                                                         [Page 8]

RFC 1321              MD5 Message-Digest Algorithm           April 1992


These notices must be retained in any copies of any part of this
documentation and/or software.
*/

#ifndef __MD5_H__
#define __MD5_H__

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/* MD5 context container */
typedef struct {
	uint32_t state[4];				/*state (ABCD), state[0] is low-order, corresponds to A*/
	uint32_t bitcount[2];			/*length of bits, modulo 2^64 bit, also maximum, count[0] is low-order byte*/
	uint32_t index[2];
	uint32_t bytecount[2];
	uint8_t buffer[64];				/*512 bit input message chunk for per round, 64x8 = 512bit*/
} MD5_CTX;

/* initialize A,B,C,D and message bit length */
void md5_ctx_init(MD5_CTX *context_p, unsigned long byte_len);

void md5_calc(MD5_CTX *context_p, const uint8_t *data, unsigned int len);

/* do message padding */
void md5_padding(MD5_CTX *context_p, unsigned int data_len, uint8_t *buffer);

/* concatenate A,B,C,D and format print */
void md5_print(uint32_t *state);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //__MD5_H__
