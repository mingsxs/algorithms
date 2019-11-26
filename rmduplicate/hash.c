/*
 * Algorithm: DJB2_32 Hash, by Dan Bernstein
 * Date: 2019/07/18
 * Note: -
 *  This is the extern Hash function used by remove-duplicate algorithm for
 *  generating string hash values.
 *
 *  Ming Li(adagio.ming@gmail.com)
 */


#include <stdint.h>
#include <sys/types.h>


uint32_t djb2_32(const void *buf, size_t size);
uint32_t (*calc_hash)(const void *buf, size_t size) = djb2_32;


uint32_t
djb2_32(const void *buf, size_t size)
{
    uint32_t hash = 5381; /* seed */
    uint8_t *p;

    for(p=(uint8_t *)buf; size>0; --size, ++p)
        hash = ((hash << 5) + hash) + (uint32_t )*p;

    return hash;
}
