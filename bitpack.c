/*
 *     bitpack.c
 *     Molly Clawson (mclaws01) and Victoria Chen (vchen05)
 *     Date: 10-26-20
 *     arith 
 *
 *     Purpose: contains width-test fucntions, 
 *              field-extraction functions, and the field-
 *              update functions to pack 
 *              multiple small value into a 64 bit word.     
 */ 

#include "bitpack.h"
#include "math.h"
#include "assert.h"
#include <stdlib.h>
#include <stdio.h>

/* Bitpack_fitsu()
 * Purpose: Determine whether an unsiged integer can fit into a 
            bitpacked word in a given number of bits
 * Parameters: The unsiged integer to be checked, and the width
 * Returns: True if the integer will fit, else false
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    assert(width > 0 && width <= 64);
    /* the number of ints we can represent is 
    2^width - 1 (the -1 accounts for 0) */
    uint64_t num_ints = pow(2, width) - 1;
    uint64_t range_hi = num_ints;

    /* if n is in the range of 0 to num_ints, then it fits */
    if(n <= range_hi){
        return true;
    } else {
        return false;
    }
}

/* Bitpack_fitss()
 * Purpose: Determine whether a signed integer can fit into a 
            bitpacked word in a given number of bits
 * Parameters: The signed integer to be checked, and the width
 * Returns: True if the integer will fit, else false
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    assert(width > 0 && width <= 64);
    int64_t num_ints = pow(2, width) - 1;
    int64_t range_hi = num_ints / 2;
    int64_t range_lo = range_hi - num_ints;
    
    if(n <= range_hi && n >= range_lo){
        return true;
    }
    return false;

}

/* Bitpack_getu()
 * Purpose: Extract an unsigned integer from a bitpacked word
 * Parameters: The bitpacked word, the width, and the least significant bit
 * Returns: The extracted unsigned integer
 */
uint64_t Bitpack_getu(uint64_t n, unsigned width, unsigned lsb)
{
    assert(width > 0 && width <= 64);
    assert((lsb + width) <= 64);
    uint64_t mask = ~0;
    uint64_t r_shift = lsb;
    uint64_t l_shift = 64 - width - lsb;

    mask = mask >> r_shift;
    mask = mask << r_shift;

    mask = mask << l_shift;
    mask = mask >> l_shift;

    uint64_t extracted = n & mask;
    extracted = extracted >> lsb;

    return extracted;
}

/* Bitpack_gets()
 * Purpose: Extract a signed integer from a bitpacked word
 * Parameters: The bitpacked word, the width, and the least significant bit
 * Returns: The extracted signed integer
 */
int64_t Bitpack_gets(uint64_t n, unsigned width, unsigned lsb)
{
    int64_t new_n = (int64_t)n;
    assert(width > 0 && width <= 64);
    assert((lsb + width) <= 64);

    uint64_t shift = 64 - width - lsb;

    new_n = new_n << shift;
    new_n = new_n >> shift;

    new_n = new_n >> lsb;

    return new_n;

}

/* Bitpack_newu()
 * Purpose: Field-update function that take in a unsigned value 
 *          and update the given word with the unsigned value. 
 * Parameters: 64 bit word, width of a field, location of 
 *             the least significant bit, updated value 
 * Returns: a new word with the updated value 
 */
uint64_t Bitpack_newu(uint64_t n, unsigned width, unsigned lsb, uint64_t value)
{
    assert(width > 0 && width <= 64);
    assert((lsb + width) <= 64);
    assert(Bitpack_fitsu(value, width) == 1);
    
    uint64_t mask1 = ~0; 
    uint64_t l_shift = width + lsb;  
    mask1 = mask1 << l_shift;
    
    uint64_t mask2 = ~0;
    uint64_t r_shift = 64 - lsb;
    mask2 = mask2 >> r_shift; 

    uint64_t full_mask = mask1 | mask2;
    uint64_t cleared_val = n & full_mask;

    value = value << lsb;
     
    uint64_t updated = value | cleared_val;
    return updated; 
}

/* Bitpack_news()
 * Purpose: Field-update function that take in a signed value 
 *          and update the given word with the signed value. 
 * Parameters: 64 bit word, width of a field, location of 
 *             the least significant bit, updated value 
 * Returns: a new word with the updated value 
 */
uint64_t Bitpack_news(uint64_t n, unsigned width, unsigned lsb, int64_t value)
{
    assert(width > 0 && width <= 64);
    assert((lsb + width) <= 64);
    assert(Bitpack_fitss(value, width) == 1);
    
    uint64_t mask1 = ~0; 
    uint64_t l_shift = width + lsb;  
    mask1 = mask1 << l_shift;
    
    uint64_t mask2 = ~0;
    uint64_t r_shift = 64 - lsb;
    mask2 = mask2 >> r_shift; 

    uint64_t full_mask = mask1 | mask2;  
    uint64_t cleared_val = n & full_mask;

    uint64_t mask3 = ~0; 
    mask3 = mask3 >> (64 - (width + lsb) ); 
    value = value << lsb;

    uint64_t new_value = mask3 & value;  
    uint64_t updated = new_value | cleared_val;
    return updated; 

}
