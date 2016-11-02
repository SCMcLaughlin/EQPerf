
#ifndef BIT_H
#define BIT_H

#define bit_mask(n) ((1 << (n)) - 1)
#define bit_mask64(n) ((1ULL << (n)) - 1ULL)
#define bit_is_pow2(n) ((n) && (((n) & ((n) - 1)) == 0))
#define bit_rotate(x, n) (((x)<<(n)) | ((x)>>(-(int)(n)&(8*sizeof(x)-1))))
#define bit_get(val, n) (val & (1 << n))
#define bit_get64(val, n) (val & (1ULL << (n)))

#endif/*BIT_H*/
