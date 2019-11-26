/*
 * Author: Ming Li
 * Code follow C99 stardard compilation
 * Date: 2018/11/05
 *
 * Algorithm: Austin Appleby(MurMur series), Glen Fowler(FNV series), Paul Hsieh(SuperFashHash) \
 *
 * Note - This code makes a few assumptions about how your machine behaves -
 *
 * Limitaions:
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian machines.
 *
 */

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

// platform macros
#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif
// macros
#define BIG_CONST(x) (x##ULL)

// 32bit hash function
extern uint32_t murmur3_32(void *key, size_t size);
extern uint32_t murmur2_32(void *key, size_t size);
extern uint32_t super_fast_hash_32(void *key, size_t size);
extern uint32_t fnv_1_32(void *key, size_t size);
extern uint32_t fnv_1a_32(void *key, size_t size);
extern uint32_t crc_32(void *key, size_t size);
extern uint32_t lose_lose_32(void *key, size_t size);
// 64bit hash function
extern uint64_t murmur_64(void *key, size_t size);
extern uint64_t fnv_1_64(void *key, size_t size);
extern uint64_t fnv_1a_64(void *key, size_t size);
extern uint64_t djb2_64(void *key, size_t size);
extern uint64_t sdbm_64(void *key, size_t size);

// crc table
static uint32_t crc32_table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

// inline functions like macros
static FORCE_INLINE uint32_t rotl32 (uint32_t x, int8_t r)
{
    return (x << r) | (x >> (32 - r));
}
static FORCE_INLINE uint32_t getblock32(uint32_t *p, int i)
{
    return p[i];
}
static FORCE_INLINE uint32_t fmix32(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85EBCA6B;
    h ^= h >> 13;
    h *= 0xC2B2AE35;
    h ^= h >> 16;

    return h;
}


uint32_t murmur3_32(void *key, size_t size)
{
    int8_t *p = key;
    int nblk = size / 4;    // 1 block = 32(4x8) bit width
    int i;
    uint32_t h1 = 0xEE6B27EB;

    uint32_t c1 = 0xCC9E2D51;
    uint32_t c2 = 0x1B873593;

    // body
    uint32_t *base = (uint32_t *)(p + nblk*4);
    uint32_t k1;
    for (i = -nblk; i; i++)
    {
        k1 = getblock32(base, i);

        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1*5 + 0xE6546B64;
    }

    // tail
    int8_t *tail = p + nblk*4;

    k1 = 0;
    switch(size & 3)
    {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
        default: break;
        k1 *= c1; k1 = rotl32(k1, 15); k1 *= c2, h1 ^= k1;
    };

    // finalization
    h1 ^= size;
    h1 = fmix32(h1);

    return h1;
}

uint32_t murmur2_32(void *key, size_t size)
{
    uint32_t *base = key;
    uint32_t h = 0xEE6B27EB;
    uint32_t c = 0x5BD1E995;
    int r = 24;
    uint32_t k;

    // body
    while(size >= 4)
    {
        k = *base;

        k *= c;
        k ^= k >> r;
        k *= c;

        h *= c;
        h ^= k;

        base++;
        size -= 4;
    }

    switch(size)
    {
        case 3: h ^= ((int8_t *)base)[2] << 16;
        case 2: h ^= ((int8_t *)base)[1] << 8;
        case 1: h ^= ((int8_t *)base)[0];
        default: break;
        h *= c;
    };

    // finalization
    h ^= h >> 13;
    h *= c;
    h ^= h >> 15;

    return h;
}

uint64_t murmur_64(void *key, size_t size)
{
    int nblk = size / 4;
    uint32_t *base = key;

    uint32_t c = 0x5BD1E995;
    int8_t r = 24;  // shift
    uint32_t h1 = 0xEE6B27EB;
    uint32_t h2 = 0;
    uint32_t k1, k2;

    // body
    while(nblk >= 8)
    {
        k1 = *base++;
        k1 *= c; k1 ^= k1 >> r; k1 *= c;
        h1 *= c; h1 ^= k1;
        nblk -= 4;

        k2 = *base++;
        k2 *= c; k2 ^= k2 >> r; k2 *= c;
        h2 *= c; h2 ^= k2;
        nblk -= 4;
    }

    if(nblk >= 4)
    {
        k1 = *base++;
        k1 *= c; k1 ^= k1 >> r; k1 *= c;
        h1 *= c; h1 ^= k1;
        nblk -= 4;
    }

    switch(nblk)
    {
        case 3: h2 ^= ((int8_t *)base)[2] << 16;
        case 2: h2 ^= ((int8_t *)base)[1] << 8;
        case 1: h2 ^= ((int8_t *)base)[0];
        default: break;
        h2 *= c;
    };

    // finalization
    h1 ^= h2 >> 18; h1 *= c;
    h2 ^= h1 >> 22; h2 *= c;
    h1 ^= h2 >> 17; h1 *= c;
    h2 ^= h1 >> 19; h2 *= c;

    uint64_t h = h1;
    h = (h << 32) | h2;     // link 2 parts
    return h;
}

uint32_t fnv_1_32(void *key, size_t size)
{
    int8_t *p = key;
    uint32_t h = 0x811C9DC5;

    while (size--)
    {
        h *= 0x01000193;
        h ^= *p++;   // no need to trafer data type
    }
    return h;
}

uint64_t fnv_1_64(void *key, size_t size)
{
    int8_t *p = key;
    uint64_t h = BIG_CONST(0xCBF29CE484222325);

    while(size--)
    {
        h *= BIG_CONST(0x100000001B3);
        h ^= *p++;
    }
    return h;
}

uint32_t fnv_1a_32(void *key, size_t size)
{
    int8_t *p = key;
    uint32_t h = 0x811C9DC5;

    while(size--)
    {
        h ^= *p++;
        h *= 0x01000193;
    }
    return h;
}

uint64_t fnv_1a_64(void *key, size_t size)
{
    int8_t *p = key;
    uint64_t h = BIG_CONST(0xCBF29CE484222325);

    while(size--)
    {
        h ^= *p++;
        h *= BIG_CONST(0x100000001B3);
    }
    return h;
}

uint32_t super_fast_hash_32(void *key, size_t size)
{
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
    || defined(_MSC_VER) || defined(__BORLANDC__) || defined(__TURBOC__)
#define get16bits(d) (*((uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((int8_t *)(d))[1])) << 8) \
        + (uint32_t)(((int8_t *)(d))[0]) )
#endif

    int8_t *p = key;
    uint32_t tmp, h = 0x1;
    int rem = size & 3;

    size >>= 2;

    // body
    for (; size > 0; size--) {
        h += get16bits (p);
        tmp = (get16bits (p + 2) << 11) ^ h;
        h = (h << 16) ^ tmp;
        p += 4;
        h += h >> 11;
    }

    // handle ending cases
    switch(rem) {
        case 3: h += get16bits (p);
                h ^= h << 16;
                h ^= p[2] << 18;
                h ^= h >> 11;
                break;
        case 2: h += get16bits (p);
                h ^= h << 11;
                h += h >> 17;
                break;
        case 1: h += *p;
                h ^= h << 10;
                h += h >> 1;
                break;
        default: break;
    }

    // avanlanching of final 127bits
    h ^= h << 3;
    h += h >> 5;
    h ^= h << 4;
    h += h >> 17;
    h ^= h << 25;
    h += h >> 6;

    return h;
}


/*
 * Algorithm: DJB2 Hash, by Dan Bernstein
 * Date: 2018/11/12
 * Note-
 *  1. code follow C99 stardard compilation.
 *  2. djb2 is one of the best string hash functions.
 *  3. excellent distribution and speed on many different sets of keys and table sizes.
 *
*/
uint64_t djb2_64(void *key, size_t size)
{
    uint64_t hash = 5381;
    int8_t *p = key;

    while(size--)
        hash = ((hash << 5) + hash) + *p++;
        /* Another magic number k = 33, why 33 is better than other number was never explained.
         * code shown as below,
         *
         * hash = (hash*33) ^ (uint64_t)*p++;
         */

    return hash;
}

/*
 * Algorithm: SDBM Hash, created for sdbm database library
 * Date: 2018/11/12
 * Note-
 *  1. code follow C99 stardard compilation.
 *  2. good in scrambling bits, causing better distribution of the keys and fewer splits.
 *  3. good general hashing function with good distribution.
 *  4. below one is used in berkeley db and elsewhere.
 *
 */
uint64_t sdbm_64(void *key, size_t size)
{
    uint64_t hash = 0;
    int8_t *p = key;

    while(size--)
        hash = *p++ + (hash << 6) + (hash << 16) - hash;

    return hash;
}

/*
 * Algorithm: Lose Lose Hash, first appeared in K&R
 * Note-
 *  1. code follow C99 stardard compilation.
 *  2. not the best possible algorithm, but it has the merit of extreme simplicity.
 *  3. actually, this hash function was proved sucks.
 *
 */
uint32_t lose_lose_32(void *key, size_t size)
{
    uint32_t hash = 0;
    int8_t *p = key;

    while(size--)
        hash += *p++;

    return hash;
}

/*
 * Code shown above can be generated at runtime using below code.
 * polynomial:
 * $edb88320
 *
 * static uint32_t crc32_table[256];
 *
 * void make_crc32_table()
 * {
 *      uint32_t c;
 *      int i, bit;
 *
 *      for(i = 0; i < 256; i++)
 *      {
 *          c = (uint32_t)i;
 *
 *          for(bit = 0; bit < 8; bit++)
 *          {
 *              if(c & 1) {
 *                  c = (c >> 1)^0xEBD88320;
 *              } else {
 *                  c = c >> 1;
 *              }
 *          }
 *
 *          crc32_table[i] = c;
 *      }
 * }
 *
 */
uint32_t crc_32(void *key, size_t size)
{
    int8_t *p = key;
    uint32_t crc;

    crc = ~0U;
    while (size--)
        crc = crc32_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

    return crc ^ ~0U;
}
