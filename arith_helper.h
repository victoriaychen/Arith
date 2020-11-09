/*
 *     arith_helper.h
 *     Molly Clawson (mclaws01) and Victoria Chen (vchen05)
 *     Date: 10-26-20
 *     arith 
 *
 *     Purpose: Interface for arith_helper.c: contains functions 
 *              and structs declarations 
 *          
 */ 

#include <stdlib.h>
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "bitpack.h"
#include "arith40.h"
#include "math.h"

typedef struct Pnm_cv_T *Pnm_cv;
struct Codeword;
struct Image_data; 

/*Compression functions*/
Pnm_ppm read_and_populate(FILE *input_file, A2Methods_T methods);
void populate_new(int col, int row, A2Methods_UArray2 u2, 
                  void *elem, void *new_array);

Pnm_ppm convert_to_floating(Pnm_ppm pixmap);
void to_floating(int col, int row, A2Methods_UArray2 u2, 
                 void *elem, void *pixmap);
Pnm_ppm convert_to_rgb(Pnm_ppm pixmap);
void to_rgb(int col, int row, A2Methods_UArray2 u2, void *elem, void *pixmap);

Pnm_ppm block_arith(Pnm_ppm pixmap, A2Methods_T methods); 
Pnm_ppm average2x2(Pnm_ppm pixmap, A2Methods_T methods); 
void populate_small(int col, int row, A2Methods_UArray2 u2, 
                    void *elem, void *Image_data); 
void to_index(int col, int row, A2Methods_UArray2 u2, void *elem, void *cl);
void abcd_to_index(int col, int row, A2Methods_UArray2 u2, 
                   void *elem, void *cl); 
void compute_dct(float array[], struct Codeword *codeword); 

void print_codeword(int col, int row, A2Methods_UArray2 u2, 
                    void *elem, void *cl); 
void print_image(Pnm_ppm pixmap, A2Methods_T methods, 
                 A2Methods_UArray2 new_array);


/*Decompression functions*/
Pnm_ppm read_compressed_file(FILE *fp);
Pnm_ppm unpack_code(Pnm_ppm pixmap, A2Methods_T methods);
void to_chroma(int col, int row, A2Methods_UArray2 u2, void *elem, void*cl); 

Pnm_ppm expand_pixmap(Pnm_ppm pixmap, A2Methods_T methods);
void populate_big(int col, int row, A2Methods_UArray2 u2, 
                  void *elem, void *Image_data); 
void init_cv(Pnm_cv cv, float avg_pb, float avg_pr, float y); 

void index_to_abcd(int col, int row, A2Methods_UArray2 u2, 
                                    void *elem, void *cl);
void packing(int col, int row, A2Methods_UArray2 u2, 
                      void *elem, void *Image_data); 
void read_32bit (int col, int row, A2Methods_UArray2 u2, 
                                  void *elem, void *fp); 
void get_bits(int col, int row, A2Methods_UArray2 u2, void *elem, void *array); 


/* Helper functions*/
void push_into_range(float *value, float max, float min);
void free_old_array (A2Methods_UArray2 old_array, A2Methods_T methods); 

