/*
 * Author: Ming Li(adagio.ming@gmail.com)
 * Date: 2018/11/15
 * Algorithm: MD5 (message-digest algorithm), designed by Ronald Rivest(MIT).
 * License:
 *   No License, for learning purpose.
 */


#include "md5.h"


/* Linux kernel way to detect system endianness. */
static union {char c[4]; int32_t test;} endian_test = {{'l', '?', '?', 'b'}};
#define ENDIANNESS ((char)endian_test.test)

/* endianness tranform */
// For big-endian to little-endian
#define BigToLittle16(X)					\
	((((uint16_t)(X) & 0xff00) >> 8) |		\
	(((uint16_t)(X) & 0x00ff) << 8))
#define BigToLittle32(X)					\
	((((uint32_t)(X) & 0xff000000) >> 24) |	\
	(((uint32_t)(X) & 0x00ff0000) >> 8) |	\
	(((uint32_t)(X) & 0x0000ff00) << 8) |	\
	(((uint32_t)(X) & 0x000000ff) << 24))
#define BigToLittle64(X)					\
	((((uint64_t)(X) & 0xff00000000000000) >> 56) |	\
	(((uint64_t)(X) & 0x00ff000000000000) >> 40) |	\
	(((uint64_t)(X) & 0x0000ff0000000000) >> 24) |	\
	(((uint64_t)(X) & 0x000000ff00000000) >> 8) |	\
	(((uint64_t)(X) & 0x00000000ff000000) << 8) |	\
	(((uint64_t)(X) & 0x0000000000ff0000) << 24) |	\
	(((uint64_t)(X) & 0x000000000000ff00) << 40) |	\
	(((uint64_t)(X) & 0x00000000000000ff) << 56))

// For little-endian to big-endian
#define LittleToBig16(X) BigToLittle16(X)
#define LittleToBig32(X) BigToLittle32(X)
#define LittleToBig64(X) BigToLittle64(X)

/* function for updating hash value in context once a block(64 byte) */
static void md5_transform(uint32_t *state, const uint8_t *block);



static uint8_t padding[64] = {	0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
								0x00, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
								0x00, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
								0x00, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
							};


/* calculating equations macros */
#define ROTL32(x, r)	(((x) << (r)) | ((x) >> (32 - (r))))

#define F(x, y, z)		(((x) & (y)) | (~(x) & (z)))
#define G(x, y, z)		(((x) & (z)) | ((y) & ~(z)))
#define H(x, y, z)		((x) ^ (y) ^ (z))
#define I(x, y, z)		((y) ^ ((x) | ~(z)))

#define FF(a, b, c, d, x, s, t)					\
	{											\
		(a) += F((b), (c), (d)) + (x) + (t);	\
		(a) = ROTL32((a), (s));					\
		(a) += (b);								\
	}

#define GG(a, b, c, d, x, s, t)					\
	{											\
		(a) += G((b), (c), (d)) + (x) + (t);	\
		(a) = ROTL32((a), (s));					\
		(a) += (b);								\
	}

#define HH(a, b, c, d, x, s, t)					\
	{											\
		(a) += H((b), (c), (d)) + (x) + (t);	\
		(a) = ROTL32((a), (s));					\
		(a) += (b);								\
	}

#define II(a, b, c, d, x, s, t)					\
	{											\
		(a) += I((b), (c), (d)) + (x) + (t);	\
		(a) = ROTL32((a), (s));					\
		(a) += (b);								\
	}

/*
 * function: decode uint8_t data to uint32_t data, assuming bytes is a multiple of 4.
 * X: uint32_t* output
 * Y: uint8_t* input
 * z: size_t byte_len
 */
#define DECODE(X, Y, Z)							\
{												\
	int i, j;									\
	for((i)=0,(j)=0; (j)<(Z); (i)++,(j)+=4)		\
	{											\
		X[(i)] = (((uint32_t)(Y)[(j)])) |		\
			(((uint32_t)(Y)[(j)+1]) << 8) |		\
			(((uint32_t)(Y)[(j)+2]) << 16) |	\
			(((uint32_t)(Y)[(j)+3]) << 24);		\
	}											\
}

/*
 * function: encode uint32_t data to uint8_t data, assuming bytes is a multiple of 4.
 * X: uint8_t* output
 * Y: uint32_t* input
 * z: size_t byte_len
 */
#define ENCODE(X, Y, Z)							\
{												\
	int i, j;									\
	for((i)=0,(j)=0; (j)<(Z); (i)++,(j)+=4)		\
	{											\
		X[(j)] = (uint8_t)((Y)[(i)] & 0xff);			\
		X[(j)+1] = (uint8_t)(((Y)[(i)] >> 8) & 0xff);	\
		X[(j)+2] = (uint8_t)(((Y)[(i)] >> 16) & 0xff);	\
		X[(j)+3] = (uint8_t)(((Y)[(i)] >> 24) & 0xff);	\
	}													\
}

/* context_p is the pointer to current md5 context, data is input message chunk, data_len is data size in byte.
*  Note -
*	padding is to pad input message to be in multiple size of 64 byte/512 bit/ chunk.
*	because padding job will only be done at last chunk, e.g. getting to the end of message/file.
*	so function will only be involked once.
*
*  Here is how padding job works,
*	when last message chunk arrives, padding will work.
*	we need to append bit '1' to the tail of the chunk first, then some bit '0' follows.
*	the appending job was done when the length of the chunk reaches to 56 bytes, eg. 448 bits, eg. 448 bits.
*	we know a chunk must be in 64 byte/512 bit size exactly, last reserved 64 bit will be padded with 
*	the message size in bit which should be less than 2 pow 64. 448 + 64 = 512 bit.
*
*  Need to know,
*	since the originally arrived message chunk length is unknown, and we must append at least 8 bit(1 byte) 
*	1/0 bit first, so 56 byte will be a threshold. but we know the arriving chunk is less than 64 byte. 
*	so if size is larger than 56 byte we need to allocate 1 more chunk/64 byte to do padding job.
*/

// transform md5 hash for each chunk, chunk size is fixed to 64 bytes
static void md5_transform(uint32_t *state, const uint8_t *block)
{
	uint32_t x[16] = {0};
	DECODE(x, block, 64);

	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];

// Round 1
	FF(a, b, c, d, x[ 0], 7 , 0xd76aa478); /* 1 */
	FF(d, a, b, c, x[ 1], 12, 0xe8c7b756); /* 2 */
	FF(c, d, a, b, x[ 2], 17, 0x242070db); /* 3 */
	FF(b, c, d, a, x[ 3], 22, 0xc1bdceee); /* 4 */
	FF(a, b, c, d, x[ 4], 7 , 0xf57c0faf); /* 5 */
	FF(d, a, b, c, x[ 5], 12, 0x4787c62a); /* 6 */
	FF(c, d, a, b, x[ 6], 17, 0xa8304613); /* 7 */
	FF(b, c, d, a, x[ 7], 22, 0xfd469501); /* 8 */
	FF(a, b, c, d, x[ 8], 7 , 0x698098d8); /* 9 */
	FF(d, a, b, c, x[ 9], 12, 0x8b44f7af); /* 10 */
	FF(c, d, a, b, x[10], 17, 0xffff5bb1); /* 11 */
	FF(b, c, d, a, x[11], 22, 0x895cd7be); /* 12 */
	FF(a, b, c, d, x[12], 7 , 0x6b901122); /* 13 */
	FF(d, a, b, c, x[13], 12, 0xfd987193); /* 14 */
	FF(c, d, a, b, x[14], 17, 0xa679438e); /* 15 */
	FF(b, c, d, a, x[15], 22, 0x49b40821); /* 16 */

// Roud2
	GG(a, b, c, d, x[ 1], 5 , 0xf61e2562); /* 17 */
	GG(d, a, b, c, x[ 6], 9 , 0xc040b340); /* 18 */
	GG(c, d, a, b, x[11], 14, 0x265e5a51); /* 19 */
	GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /* 20 */
	GG(a, b, c, d, x[ 5], 5 , 0xd62f105d); /* 21 */
	GG(d, a, b, c, x[10], 9 ,  0x2441453); /* 22 */
	GG(c, d, a, b, x[15], 14, 0xd8a1e681); /* 23 */
	GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /* 24 */
	GG(a, b, c, d, x[ 9], 5 , 0x21e1cde6); /* 25 */
	GG(d, a, b, c, x[14], 9 , 0xc33707d6); /* 26 */
	GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); /* 27 */
	GG(b, c, d, a, x[ 8], 20, 0x455a14ed); /* 28 */
	GG(a, b, c, d, x[13], 5 , 0xa9e3e905); /* 29 */
	GG(d, a, b, c, x[ 2], 9 , 0xfcefa3f8); /* 30 */
	GG(c, d, a, b, x[ 7], 14, 0x676f02d9); /* 31 */
	GG(b, c, d, a, x[12], 20, 0x8d2a4c8a); /* 32 */

// Roud3
	HH(a, b, c, d, x[ 5], 4 , 0xfffa3942); /* 33 */
	HH(d, a, b, c, x[ 8], 11, 0x8771f681); /* 34 */
	HH(c, d, a, b, x[11], 16, 0x6d9d6122); /* 35 */
	HH(b, c, d, a, x[14], 23, 0xfde5380c); /* 36 */
	HH(a, b, c, d, x[ 1], 4 , 0xa4beea44); /* 37 */
	HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9); /* 38 */
	HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); /* 39 */
	HH(b, c, d, a, x[10], 23, 0xbebfbc70); /* 40 */
	HH(a, b, c, d, x[13], 4 , 0x289b7ec6); /* 41 */
	HH(d, a, b, c, x[ 0], 11, 0xeaa127fa); /* 42 */
	HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); /* 43 */
	HH(b, c, d, a, x[ 6], 23,  0x4881d05); /* 44 */
	HH(a, b, c, d, x[ 9], 4 , 0xd9d4d039); /* 45 */
	HH(d, a, b, c, x[12], 11, 0xe6db99e5); /* 46 */
	HH(c, d, a, b, x[15], 16, 0x1fa27cf8); /* 47 */
	HH(b, c, d, a, x[ 2], 23, 0xc4ac5665); /* 48 */

// Roud4
	II(a, b, c, d, x[ 0], 6 , 0xf4292244); /* 49 */
	II(d, a, b, c, x[ 7], 10, 0x432aff97); /* 50 */
	II(c, d, a, b, x[14], 15, 0xab9423a7); /* 51 */
	II(b, c, d, a, x[ 5], 21, 0xfc93a039); /* 52 */
	II(a, b, c, d, x[12], 6 , 0x655b59c3); /* 53 */
	II(d, a, b, c, x[ 3], 10, 0x8f0ccc92); /* 54 */
	II(c, d, a, b, x[10], 15, 0xffeff47d); /* 55 */
	II(b, c, d, a, x[ 1], 21, 0x85845dd1); /* 56 */
	II(a, b, c, d, x[ 8], 6 , 0x6fa87e4f); /* 57 */
	II(d, a, b, c, x[15], 10, 0xfe2ce6e0); /* 58 */
	II(c, d, a, b, x[ 6], 15, 0xa3014314); /* 59 */
	II(b, c, d, a, x[13], 21, 0x4e0811a1); /* 60 */
	II(a, b, c, d, x[ 4], 6 , 0xf7537e82); /* 61 */
	II(d, a, b, c, x[11], 10, 0xbd3af235); /* 62 */
	II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /* 63 */
	II(b, c, d, a, x[ 9], 21, 0xeb86d391); /* 64 */

// Update md5 context state,
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
// Wipe out freed memory block.
	memset(x, 0, 16);
}

/* initialize current md5 context container */
void md5_ctx_init(MD5_CTX *context_p, unsigned long byte_len)
{
	// intialize A,B,C,D with specific numbers, count for bit-length.
	memset(context_p, 0, sizeof(*context_p));
	context_p->state[0] = 0x67452301;  // A, low-order of 128bit final result
	context_p->state[1] = 0xefcdab89;  // B
	context_p->state[2] = 0x98badcfe;  // C
	context_p->state[3] = 0x10325476;  // D, high-order

	// for big-endian mochine conversion, default little-endian.
	if(ENDIANNESS == 'b') {
		BigToLittle64(byte_len);
	}

	context_p->bitcount[0] = (uint32_t)(byte_len & 0xFFFFFFFFULL) << 3; // low-order
	context_p->bitcount[1] = (uint32_t)(byte_len >> 29); // high-order
	context_p->bytecount[0] = (uint32_t)(byte_len);
	context_p->bytecount[1] = (uint32_t)(byte_len >> 32);

	memset(&context_p->index, 0x0, 8);
}

/* pad last chunk(64 bytes) */
void md5_padding(MD5_CTX *context_p, unsigned int data_len, uint8_t *buffer)
{
	unsigned int padlen = (data_len < 56 )? (56 - data_len) : (120 - data_len);

	memcpy((void *)buffer, (void *)(context_p->buffer), data_len);

	//append the padding data.
	memcpy((void *)(&buffer[data_len]), (void *)padding, padlen);

	uint8_t bits[8];

	//encode 32bit bit length to 8bit.
	ENCODE(bits, context_p->bitcount, 8);

	//append bit length to the end.
	memcpy(&buffer[data_len + padlen], bits, 8);
}


/* calc md5 value for given data block and update context container */
void md5_calc(MD5_CTX *context_p, const uint8_t *data, unsigned int len)
{
	unsigned int i;
	for(i = 0; (i + 63) < len; i += 64) {
		md5_transform(context_p->state, &data[i]);
		if((context_p->index[0] += 64) < context_p->index[0]) context_p->index[1]++;
	}

	unsigned int tail = len - i;

	const uint32_t low = context_p->bytecount[0];
	const uint32_t high = context_p->bytecount[1];

	if ((low - context_p->index[0] < 64) && (high == context_p->index[1])) {
		uint8_t buffer[128] = {0};
		unsigned int rounds = (tail < 56 )? 1 : 2;
		memcpy((void *)(context_p->buffer), (void *)(&data[i]), tail);

		md5_padding(context_p, tail, buffer);

		for(i = 0; i < rounds; i++) {
			md5_transform(context_p->state, &buffer[64*i]);
			if((context_p->index[0] += 64) < context_p->index[0]) context_p->index[1]++;
		}
	}
}


/* formate result printing */
void md5_print(uint32_t *state)
{
	uint8_t digest[16] = {0};
	ENCODE(digest, state, 16);

	int i;
	for(i = 0; i < 16; i++) {
		printf("%02x", digest[i]);
	}
	printf("\n");
}
