/*
 *     compress40.c
 *     Molly Clawson (mclaws01) and Victoria Chen (vchen05)
 *     Date: 10-26-20
 *     arith 
 *
 *     Purpose: Compress or decomporess an image from a file
 */

#include "compress40.h"
#include "arith_helper.h"

/* compress40()
 * Purpose: Compress a ppm file that was provided bu the user
 * Parameters: A file pointer which accesses the file to be compressed
 * Returns: None
 */
extern void compress40(FILE *input)
{
    A2Methods_T methods = uarray2_methods_blocked;
    Pnm_ppm pixmap = read_and_populate(input, methods);
    pixmap = convert_to_floating(pixmap);
    pixmap = block_arith(pixmap, methods); 
    Pnm_ppmfree(&pixmap);
}


/* decompress40()
 * Purpose: decompress a ppm file that was provided bu the user
 * Parameters: A file pointer which accesses the file to be decompressed
 * Returns: None
 */
extern void decompress40(FILE *input)
{
    A2Methods_T methods = uarray2_methods_blocked;
    Pnm_ppm pixmap = read_compressed_file(input); 
    pixmap = unpack_code(pixmap, methods); 
    pixmap = convert_to_rgb(pixmap);
    Pnm_ppmwrite(stdout, pixmap);
    Pnm_ppmfree(&pixmap);
}


