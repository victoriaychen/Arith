/*
 *     a2plain.c
 *     Molly Clawson (mclaws01) and Victoria Chen (vchen05)
 *     Date: 10-12-20
 *     Locality
 *
 *     Purpose: Implementation for the uarray_methods_plain struct,
 *              which stores a suite of function pointers which 
 *              can initialize, modify, and ask for information about
 *              a UArray2. When used with a2blocked.c, provides 
 *              polymorhpic Object-Oriented style functions which 
 *              can be called on either a UArray2 or a UArray2b. 
 */ 

#include <string.h>
#include <a2plain.h>
#include "uarray2.h"


static A2Methods_UArray2 new(int width, int height, int size)
{
  return UArray2_new(width, height, size);
}

static A2Methods_UArray2 new_with_blocksize(int width, int height, 
                                            int size, int blocksize)
{
  (void) blocksize;
  return UArray2_new(width, height, size);
}

static void a2free(A2Methods_UArray2 *uarray2)
{
    UArray2_free((UArray2_T *)uarray2);
}

static int width(A2Methods_UArray2 uarray2)
{
	return UArray2_width(uarray2);
}

static int height(A2Methods_UArray2 uarray2)
{
	return UArray2_height(uarray2);
}

static int size(A2Methods_UArray2 uarray2)
{
	return UArray2_size(uarray2);
}

static int blocksize(A2Methods_UArray2 uarray2)
{
  //the blocksize of a uarray2 is 1
  (void)uarray2; 
	return 1;
}

static A2Methods_Object *at(A2Methods_UArray2 uarray2, int col, int row)
{
	return UArray2_at(uarray2, col, row);
}


static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
  UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
  UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
  A2Methods_smallapplyfun *apply; 
  void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
  struct small_closure *cl = vcl;
  (void)i;
  (void)j;
  (void)uarray2;
  cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
  struct small_closure mycl = { apply, cl };
  UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
  struct small_closure mycl = { apply, cl };
  UArray2_map_col_major(a2, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
  new,
  new_with_blocksize,
  a2free, 
  width, 
  height, 
  size, 
  blocksize,
  at,
  map_row_major, 
  map_col_major, 
  NULL,                   //map_block_major
  map_col_major,          //map default
  small_map_row_major,
  small_map_col_major,
  NULL,                   //small_map_block_major
  small_map_col_major,    //small map default

};


A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
