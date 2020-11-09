/*
 *     arith_helper.c
 *     Molly Clawson (mclaws01) and Victoria Chen (vchen05)
 *     Date: 10-26-20
 *     arith 
 *
 *     Purpose: Contains series of arithmetic functions which 
 *              read from image files, decompress images, and 
 *              print the decompressed images in big-endian order.
 *              Also contains arithmetic functions which do the 
 *              reverse: read from compressed files, unpack bitpacked 
 *              words, and print out a similar image to the original 
 *              (decompressed) image. 
 */ 


#include "arith_helper.h"

/* stores compnent video data */
struct Pnm_cv_T
{
    /* the y, pb, and pr values of a pixel */
    float y, pb, pr;
};

/* stores data that needs to be kept track of as we 
iterate through a 2x2 block of the original image */
struct Image_data 
{
    /* the sum of the pb and pr values on the 2x2 block */
    float pb_sum, pr_sum;
    /* the 4 y values in a 2x2 block */
    float y_array[4]; 
    /* the small array which is populated with compressed data */
    A2Methods_UArray2 array;
    /* the methods we're currently using */
    A2Methods_T methods;  
};

/* store the values that are needed to pack into a 32 bit word */
struct Codeword
{
    float avg_pb, avg_pr;
    unsigned pb_index, pr_index;
    float a, b, c, d;
    int b_int, c_int, d_int; 
    unsigned a_int;
    A2Methods_T methods; 
}; 

/*read_and_populate()
 * Purpose: read from a file and populate an A2Methods_UArray 
 *          with a blocksize of 2 with pixel information from the file
 * Parameters: A pointer to an open file, and the methods suite
 * Returns: The populated Pnm_ppm pixmap
 * Notes: Cuts off the last row or column of an image 
          if and only if either appear in an odd number
 */
Pnm_ppm read_and_populate(FILE *input, A2Methods_T methods)
{
    assert(input != NULL);
    assert(methods != NULL); 
    /* Since we cannot choose the blocksize in ppmread, we must 
    populate a default blocked array and manually dictate the blocksize */
    Pnm_ppm pixmap = Pnm_ppmread(input, methods);
    assert(pixmap != NULL);

    int width = pixmap->width;
    int height = pixmap->height;
    int size = methods->size(pixmap->pixels);

    /*if the width or the height is an odd number, 
    delete the last row or column*/
    if ((width % 2) != 0) {
        width--;
    }
    if ((height % 2) != 0) {
        height--;
    }

    /* Create a new UArray2b with a blocksize of 2,
    which will make accessing a 2x2 block simpler */
    A2Methods_UArray2 new_array = methods->new_with_blocksize(
                                  width, height, size, 2);
    assert(new_array != NULL);

    A2Methods_UArray2 old_array = pixmap->pixels;
    methods->map_block_major(old_array, populate_new, new_array);
    pixmap->pixels = new_array;
    free_old_array(old_array, methods); 

    /*if the width and height were changed, 
    update the corresponding pixmap fields*/
    pixmap->width = width;
    pixmap->height = height;

    return pixmap;
}

/* populate_new()
 * Purpose: An apply function which will populate one pixel in the pixmap
 * Parameters: The current column and row, the original pixels array, 
               a pointer to the current element, and the new array
 * Returns: Nothing
 * Notes: Populates the new pixmap
 */
void populate_new(int col, int row, A2Methods_UArray2 u2, 
                  void *elem, void *new_array)
{
    (void)u2;
    A2Methods_UArray2 array = (A2Methods_UArray2)new_array;
    A2Methods_T methods = uarray2_methods_blocked;
    
    Pnm_rgb new_location = (Pnm_rgb)methods->at(array, col, row);
    *new_location = *(Pnm_rgb)elem;
}

/* convert_to_floating()
 * Purpose: Convert the rgb values of a pixmap to floating component video
 * Parameters: The pixmap and the methods suite
 * Returns: The Pnm_ppm pixmap, newly populated with component video data
 */
Pnm_ppm convert_to_floating(Pnm_ppm pixmap)
{ 
    A2Methods_UArray2 old_array = pixmap->pixels;
    A2Methods_UArray2 new_array = pixmap->methods->new_with_blocksize(
                                  pixmap->width, pixmap->height, 
                                  sizeof(struct Pnm_cv_T), 2);
    pixmap->pixels = new_array;
    pixmap->methods->map_block_major(old_array, to_floating, pixmap);
    if (old_array != NULL) {
       pixmap->methods->free(&old_array);
    }
    return pixmap;
}

/*to_floating()
 * Purpose: An apply function which converts one rgb pixel to component video
 * Parameters: The current column and row, the original pixels array, 
               a pointer to the current element, and the pixmap
 * Returns: none
 * Notes: Information on the original rgb values of each pixel is lost 
 */
void to_floating(int col, int row, A2Methods_UArray2 u2, 
                 void *elem, void *pixmap)
{
    (void)u2;
    Pnm_ppm pix = (Pnm_ppm)pixmap; 
    A2Methods_T methods = uarray2_methods_blocked;
    Pnm_rgb rgb = (Pnm_rgb)elem;
     
    float red = (float)rgb->red / pix->denominator;
    float blue = (float)rgb->blue / pix->denominator;
    float green = (float)rgb->green / pix->denominator;

    float y = 0.299 * red + 0.587 * green + 0.114 * blue;
    float pb = -0.168736 * red - 0.331264 * green + 0.5 * blue;
    float pr = 0.5 * red - 0.418688 * green - 0.081312 * blue;

    push_into_range(&y, 1.0, 0.0);
    push_into_range(&pb, 0.5, -0.5);
    push_into_range(&pr, 0.5, -0.5);

    struct Pnm_cv_T comp_vid; 
    comp_vid.y = y;
    comp_vid.pb = pb;
    comp_vid.pr = pr;

    Pnm_cv new_location = (Pnm_cv)methods->at(pix->pixels, col, row);
    *new_location = comp_vid;
}

/* convert_to_rgb()
 * Purpose: Convert the component video values of a pixmap to rgb values
 * Parameters: The pixmap and the methods suite
 * Returns: The Pnm_ppm pixmap, newly populated with rgb data
 */
Pnm_ppm convert_to_rgb(Pnm_ppm pixmap)
{
    A2Methods_UArray2 old_array = pixmap->pixels;
    A2Methods_UArray2 new_array = pixmap->methods->new_with_blocksize(
                                  pixmap->width, pixmap->height, 
                                  sizeof(struct Pnm_rgb), 2);
    pixmap->pixels = new_array;
    pixmap->methods->map_block_major(old_array, to_rgb, pixmap);
    
    if (old_array != NULL) {
        pixmap->methods->free(&old_array);
    }
    return pixmap; 
}

/*to_floating()
 * Purpose: An apply function which converts one component video pixel to rgb
 * Parameters: The current column and row, the original pixels array, 
               a pointer to the current element, and the pixmap
 * Returns: none
 */
void to_rgb(int col, int row, A2Methods_UArray2 u2, void *elem, void *pixmap)
{
    (void)u2;
    Pnm_ppm pix = (Pnm_ppm)pixmap; 
    A2Methods_T methods = uarray2_methods_blocked;

    Pnm_cv cv = (Pnm_cv)elem;
    float y = cv->y;
    float pb = cv->pb;
    float pr = cv->pr;

    float red = 1.0 * y + 0.0 * pb + 1.402 * pr;
    float green = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
    float blue = 1.0 * y + 1.772 * pb + 0.0 * pr;

    push_into_range(&red, 1.0, 0.0);
    push_into_range(&green, 1.0, 0.0);
    push_into_range(&blue, 1.0, 0.0);

    int denominator = pix->denominator;
    red *= denominator;
    green *= denominator;
    blue *= denominator;

    unsigned red_u = (unsigned)round((double)red);
    unsigned green_u = (unsigned)round((double)green);
    unsigned blue_u = (unsigned)round((double)blue);

    struct Pnm_rgb rgb;
    rgb.red = red_u;
    rgb.green = green_u;
    rgb.blue = blue_u;

    Pnm_rgb new_location = (Pnm_rgb)methods->at(pix->pixels, col, row);
    *new_location = rgb;
}

/*block_arith()
 * Purpose: Perform multiple arithmetic steps on each pixel of the pixmap
            in order to convert the data to 32-bit packed codewords
 * Parameters: The pixmap and the methods suite
 * Returns: The pixmap, which is now populated with bitpacked codewords
 */
Pnm_ppm block_arith(Pnm_ppm pixmap, A2Methods_T methods){
    assert(pixmap != NULL);
    assert(methods != NULL);

    pixmap = average2x2(pixmap, methods); 
    methods->map_block_major(pixmap->pixels, to_index, NULL); 
    methods->map_block_major(pixmap->pixels, abcd_to_index, NULL); 
    
    A2Methods_T new_methods = uarray2_methods_plain; 
    A2Methods_UArray2 new_array = new_methods->new(
                                  pixmap->width, pixmap->height, 
                                  sizeof(uint32_t));
    assert(new_array != NULL); 
    
    struct Image_data image_data;
    image_data.array = new_array; 
    image_data.methods = new_methods; 
    image_data.pb_sum = 0.0;
    image_data.pr_sum = 0.0;
    
    methods->map_block_major(pixmap->pixels, packing, &image_data); 
    print_image(pixmap, new_methods, image_data.array); 
    
    return pixmap;  
}

/* average2x2()
 * Purpose: Compute the average pb and pr values in a 2x2 block
 * Parameters: The pixmap and the methods suite
 * Returns: The pixmap, which is now populated with codeword structs
 * Notes: The Pb and Pr values of each pixel of the original image 
 *        are lost in this step. Only the average is now stored. 
 */
Pnm_ppm average2x2(Pnm_ppm pixmap, A2Methods_T methods)
{
    assert(pixmap != NULL);
    assert(methods != NULL);

    int new_width = pixmap->width / 2;
    int new_height = pixmap->height / 2;
    A2Methods_UArray2 old_array = pixmap->pixels; 
    A2Methods_UArray2 new_array = methods->new_with_blocksize(
                                  new_width, new_height, 
                                  sizeof(struct Codeword), 1);
    assert(new_array != NULL); 
    
    /* create a new struct to store the new array */ 
    struct Image_data image_data;  
    image_data.array = new_array; 
    image_data.methods = methods; 
    image_data.pb_sum = 0.0; 
    image_data.pr_sum = 0.0; 
    methods->map_block_major(old_array, populate_small, &image_data); 
    pixmap->pixels = image_data.array; 
    
    /* update the width and height of the pixmap */
    pixmap->width/=2;
    pixmap->height/=2;
    
    free_old_array(old_array, methods); 
    return pixmap; 
}

/* populate_small()
 * Purpose: Compute the average pb and pr values in a 2x2 block
 * Parameters: The pixmap and the methods suite
 * Returns: The pixmap, which is now populated with codeword structs
 * Notes: The Pb and Pr values of each pixel of the original image 
 *        are lost in this step. Only the average is now stored. 
 */
void populate_small(int col, int row, A2Methods_UArray2 u2, 
                    void *elem, void *Image_data) 
{
    (void)u2;
    
    Pnm_cv cv = (Pnm_cv)elem;
    struct Image_data *image = (struct Image_data*)Image_data;
    A2Methods_T methods = image->methods; 
   
    /*add this element's pb and pr to the running sum of values*/
    image->pb_sum += cv->pb;
    image->pr_sum += cv->pr;
 
    /*if we're at the top left of a block, grab the y values 
    of the entire block and store them for later */
    if(col % 2 == 0 && row % 2 == 0){
        image->y_array[0] = cv->y; 
        Pnm_cv cv2 = (Pnm_cv)methods->at(u2, col + 1, row); 
        image->y_array[1] = cv2->y;
        Pnm_cv cv3 = (Pnm_cv)methods->at(u2, col, row + 1); 
        image->y_array[2] = cv3->y;
        Pnm_cv cv4 = (Pnm_cv)methods->at(u2, col + 1, row + 1); 
        image->y_array[3] = cv4->y;
    }

    float avg_pb, avg_pr;
    /*if we're at the last element in a 2x2 block, compute the average 
    and add them to the compressed UArray */
    if (col % 2 != 0 && row % 2 != 0){
        avg_pb = image->pb_sum / 4.0;
        avg_pr = image->pr_sum / 4.0;
        push_into_range(&avg_pb, 0.5, -0.5);
        push_into_range(&avg_pr, 0.5, -0.5);

        /*find the index that we need in the compressed UArray*/
        int new_col = col / 2;
        int new_row = row / 2;
        struct Codeword codeword; 
        codeword.avg_pb = avg_pb; 
        codeword.avg_pr = avg_pr; 

        compute_dct(image->y_array, &codeword);
       
        struct Codeword *new_location = (struct Codeword*)methods->at(
                                        image->array, new_col, new_row);
        *new_location = codeword; 
        new_location->a = codeword.a;
        new_location->b = codeword.b;
        new_location->c = codeword.c;
        new_location->d = codeword.d;
        new_location->avg_pb = codeword.avg_pb;
        new_location->avg_pr = codeword.avg_pr;

        /*reset the values of the sums and the y-values for the next block*/
        image->pb_sum = 0;
        image->pr_sum = 0;
    }
}

/* compute_dct()
 * Purpose: Performs discrete cosine transformation on one 2x2 block of pixels
 * Parameters: An array of y values, a pointer to the codeword
 * Returns: None
 * Notes: Computes a, b, c, and d, and ensures that they are in range 
 */
void compute_dct(float array[], struct Codeword *codeword)
{

    float a = (array[3] + array[2] + array[1] + array[0]) / 4.0;
    float b = (array[3] + array[2] - array[1] - array[0]) / 4.0;
    float c = (array[3] - array[2] + array[1] - array[0]) / 4.0;
    float d = (array[3] - array[2] - array[1] + array[0]) / 4.0;

    push_into_range(&a, 1.0, 0.0);
    push_into_range(&b, 0.3, -0.3);
    push_into_range(&c, 0.3, -0.3);
    push_into_range(&d, 0.3, -0.3);

    codeword->a = a; 
    codeword->b = b; 
    codeword->c = c; 
    codeword->d = d; 
    
}

/* to_index()
 * Purpose: Converts the average pb and pr values of a 2x2 block to an index
 * Parameters: The current column, the current row, the UArray, a 
 *             pointer to the current element, and a closure (NULL)
 * Returns: None
 * Notes: The exact pb and pr values of the 2x2 block are lost in this 
 *        step, as we are quantizing the values in a range of -15 to 15 
 */
void to_index(int col, int row, A2Methods_UArray2 u2, void *elem, void *cl)
{
    (void)u2;
    (void)col;
    (void)row;
    (void)cl;
    
    struct Codeword *codeword = (struct Codeword*)elem; 
    assert(codeword != NULL);
    float avg_pb = codeword->avg_pb;
    float avg_pr = codeword->avg_pr;

    codeword->pb_index = Arith40_index_of_chroma(avg_pb); 
    codeword->pr_index = Arith40_index_of_chroma(avg_pr); 
}

/* unpack_code()
 * Purpose: Transforms an array of 32-bit words to an expanded 
            array which contains a, b, c, d, pb index, and pr index
 * Parameters: The pixmap and the methods suite
 * Returns: A Pnm_ppm, which contains the aforementioned pixel data
 */
Pnm_ppm unpack_code(Pnm_ppm pixmap, A2Methods_T methods)
{
    assert(pixmap != NULL);
    assert(methods != NULL);

    A2Methods_UArray2 new_array = methods->new_with_blocksize(pixmap->width, 
                                pixmap->height, sizeof(struct Codeword), 1);
    
    A2Methods_T new_methods = uarray2_methods_plain;
    new_methods->map_row_major(pixmap->pixels, get_bits, new_array);
    
    A2Methods_UArray2 old_array = pixmap->pixels; 
    pixmap->methods = uarray2_methods_blocked;
    pixmap->pixels = new_array; 
    
    methods->map_block_major(pixmap->pixels, index_to_abcd, NULL);
    methods->map_block_major(pixmap->pixels, to_chroma, NULL);

    pixmap = expand_pixmap(pixmap, methods); 

    free_old_array(old_array, new_methods); 
    return pixmap;  
}

/* get_bits()
 * Purpose: Unpack one 32-bit word to reveal a, b, c, d, pb, and pr index
 * Parameters: The current column, the current row, the UArray, a 
 *             pointer to the current element, the new array to be populated
 * Returns: None
 * Notes: Populates a new array
 */
void get_bits(int col, int row, A2Methods_UArray2 u2, void *elem, void *array)
{
    (void)u2; 
    
    uint32_t word = *(uint32_t *)elem;
    A2Methods_UArray2 new_array = (A2Methods_UArray2) array;
    A2Methods_T methods = uarray2_methods_blocked;

    unsigned a_int = Bitpack_getu(word, 6, 26);

    int b_int = Bitpack_gets(word, 6, 20);
    int c_int = Bitpack_gets(word, 6, 14);
    int d_int = Bitpack_gets(word, 6, 8);
    unsigned pb_index = Bitpack_getu(word, 4, 4);
    unsigned pr_index = Bitpack_getu(word, 4, 0);
    
    struct Codeword *location = (struct Codeword *)methods->at(
                                                   new_array, col, row);
    location->a_int = a_int; 
    location->b_int = b_int; 
    location->c_int = c_int; 
    location->d_int = d_int; 
    location->pb_index = pb_index; 
    location->pr_index = pr_index; 
}

/* to_chroma()
 * Purpose: Convert one pixel's pb and pr index to its chroma value
 * Parameters: The current column, the current row, the UArray, a 
 *             pointer to the current element, the new array to be populated
 * Returns: None
 * Notes: The pb and pr indices which are populated by this function 
 *        are the closest that we can achieve to the original value, since the 
 *        original value was quantized
 */
void to_chroma(int col, int row, A2Methods_UArray2 u2, void *elem, void*cl)
{
    (void)u2;
    (void)col;
    (void)row;
    (void)cl;

    struct Codeword *codeword = (struct Codeword*)elem; 
    assert(codeword != NULL);

    unsigned pb_index = codeword->pb_index; 
    unsigned pr_index = codeword->pr_index; 

    codeword->avg_pb = Arith40_chroma_of_index(pb_index); 
    codeword->avg_pr = Arith40_chroma_of_index(pr_index);  
}

/* expand_pixmap()
 * Purpose: Expand the compressed pixmap to be twice its size
 * Parameters: The Pnm_ppm and the methods suite
 * Returns: A Pnm_ppm, which has been expanded and will 
 *          now store component video data
 */
Pnm_ppm expand_pixmap(Pnm_ppm pixmap, A2Methods_T methods)
{
    assert(pixmap != NULL);
    assert(methods != NULL);

    int new_width = pixmap->width * 2;
    int new_height = pixmap->height * 2;

    A2Methods_UArray2 old_array = pixmap->pixels; 
    A2Methods_UArray2 new_array = methods->new_with_blocksize(
                                  new_width, new_height, 
                                  sizeof(struct Pnm_cv_T), 2);
    assert(new_array != NULL); 
    
    /*create a new struct to store the new array*/
    struct Image_data image; 
    image.array = new_array; 
    image.methods = uarray2_methods_blocked;
    methods->map_block_major(old_array, populate_big, &image); 
    pixmap->pixels = image.array;  
    
    /*update the width and height of the pixmap*/
    pixmap->width = new_width;
    pixmap->height = new_height;
    
    free_old_array(old_array, methods); 
    return pixmap; 
}

/* populate_big()
 * Purpose: an apply function that perform inverse of the discrete cosine
 *          transform and store 4 y value into a Pnm_cv struct. 
 *          Then populate a new 2d array with the Pnm_cv struct. 
 * Parameters: col of 2d array, row of 2d array, Uarray2b, codeword struct, 
 *             Image_data struct 
 * Returns: none
 */
void populate_big(int col, int row, A2Methods_UArray2 u2, void *elem, 
                                                   void *Image_data)
{
    (void)u2; 
    struct Codeword codeword = *(struct Codeword*)elem;
    struct Image_data image = *(struct Image_data*)Image_data;

    float avg_pb = codeword.avg_pb;
    float avg_pr = codeword.avg_pr;
    float a = codeword.a;
    float b = codeword.b;
    float c = codeword.c;
    float d = codeword.d;

    /* Inverse of the discrete cosine transform */
    float y1 = a - b - c + d;
    float y2 = a - b + c - d;
    float y3 = a + b - c - d;
    float y4 = a + b + c + d;

    push_into_range(&y1, 1.0, 0.0);
    push_into_range(&y2, 1.0, 0.0);
    push_into_range(&y3, 1.0, 0.0);
    push_into_range(&y4, 1.0, 0.0);

    int new_col = col * 2;
    int new_row = row * 2;
    
    Pnm_cv cv1 = (Pnm_cv)image.methods->at(image.array, new_col, new_row);
    Pnm_cv cv2 = (Pnm_cv)image.methods->at(image.array, new_col + 1, new_row);
    Pnm_cv cv3 = (Pnm_cv)image.methods->at(image.array, new_col, new_row + 1);
    Pnm_cv cv4 = (Pnm_cv)image.methods->at(image.array, new_col + 1, 
                                                        new_row + 1);

    init_cv(cv1, avg_pb, avg_pr, y1); 
    init_cv(cv2, avg_pb, avg_pr, y2);
    init_cv(cv3, avg_pb, avg_pr, y3);
    init_cv(cv4, avg_pb, avg_pr, y4);
}

/* init_cv()
 * Purpose: create a pnm_cv struct and populate the struct 
 * Parameters: Pnm_cv struct, average pb, average pr, y value 
 * Returns: none
 */
void init_cv(Pnm_cv cv, float avg_pb, float avg_pr, float y){
    struct Pnm_cv_T cv_T; 
    cv_T.y = y;
    cv_T.pb = avg_pb;
    cv_T.pr = avg_pr;
    *cv = cv_T; 
}


/* abcd_to_index()
 * Purpose: an apply function that covert a, b, c, d
 *          into a, b, c, d index 
 * Parameters: col of 2d array, row of 2d array, Uarray2b, codeword struct, 
 *             closure (NULL)
 * Returns: none
 */
void abcd_to_index(int col, int row, A2Methods_UArray2 u2, 
                   void *elem, void *cl)
{
    (void) col;
    (void) row;
    (void) u2;
    (void) cl;

    struct Codeword *codeword = (struct Codeword*)elem;
    unsigned a = round(codeword->a * 63.0);
    int b = round(codeword->b * 50.0);
    int c = round(codeword->c * 50.0);
    int d = round(codeword->d * 50.0);
   
    codeword->a_int = a; 
    codeword->b_int = b; 
    codeword->c_int = c; 
    codeword->d_int = d; 
}


/* index_to_abcd()
 * Purpose: an apply function that covert the a, b, c, d index 
 *          into a, b, c, d
 * Parameters: col of 2d array, row of 2d array, Uarray2b, codeword struct,
 *             closure (NULL)
 * Returns: none
 */
void index_to_abcd(int col, int row, A2Methods_UArray2 u2, 
                   void *elem, void *cl)
{
    (void) col;
    (void) row;
    (void) u2;
    (void) cl;
    struct Codeword *codeword = (struct Codeword*)elem;
    float a = (float)(codeword->a_int / 63.0);
    float b = (float)(codeword->b_int / 50.0);
    float c = (float)(codeword->c_int / 50.0);
    float d = (float)(codeword->d_int / 50.0);

    codeword->a = a; 
    codeword->b = b; 
    codeword->c = c; 
    codeword->d = d; 
}

/* packing()
 * Purpose: an apply function that packs a, b, c, d, average pb, 
 *          and average pr into A 32-bit words into a Uarray2
 * Parameters: col of 2d array, row of 2d array, Uarray2b, 32 bit word, 
 *             struct Image_data 
 * Returns: none
 */
void packing(int col, int row, A2Methods_UArray2 u2, void *elem, 
                                              void *Image_data)
{
    (void) col;
    (void) row;
    (void) u2;

    struct Image_data *image = (struct Image_data *)Image_data;
    struct Codeword codeword = *(struct Codeword *)elem;

    uint32_t word = 0;
    word = (uint32_t)Bitpack_newu(word, 6, 26, codeword.a_int);
    word = (uint32_t)Bitpack_news(word, 6, 20, codeword.b_int);
    word = (uint32_t)Bitpack_news(word, 6, 14, codeword.c_int);
    word = (uint32_t)Bitpack_news(word, 6, 8, codeword.d_int);
    word = (uint32_t)Bitpack_newu(word, 4, 4, codeword.pb_index);
    word = (uint32_t)Bitpack_newu(word, 4, 0, codeword.pr_index);

    uint32_t *location = image->methods->at(image->array, col, row);
    *location = word;  
}

/* print_image()
 * Purpose: write a compressed binary image to standard output
 * Parameters: pixmap, blocked methods, Uarray2
 * Returns: none
 */
void print_image(Pnm_ppm pixmap, A2Methods_T methods, 
                         A2Methods_UArray2 new_array)
{
    printf("COMP40 Compressed image format 2\n%u %u\n", pixmap->width, 
             pixmap->height); 
    methods->map_row_major(new_array, print_codeword, &pixmap->width);
    free_old_array (new_array, methods); 
}

/* print_codeword()
 * Purpose: an apply function that write out the each code words
 * Parameters: col of 2d array, row of 2d array, Uarray2, 32 bit word, 
 *             file pointer
 * Returns: none
 * Notes: putchar() write a single byte
 */
void print_codeword(int col, int row, A2Methods_UArray2 u2, 
                    void *elem, void *cl)
{
    (void) col;
    (void) row;
    (void) u2;
    (void) cl; 

    uint32_t codeword = *(uint32_t*)elem;

    uint32_t first_byte = codeword; 
    first_byte = first_byte >> 24;
    putchar(first_byte); 

    uint32_t second_byte = codeword;
    second_byte = second_byte << 8; 
    second_byte = second_byte >> 24;
    putchar(second_byte); 

    uint32_t third_byte = codeword;
    third_byte = third_byte << 16; 
    third_byte = third_byte >> 24;
    putchar(third_byte); 

    uint32_t last_byte = codeword;
    last_byte = last_byte << 24; 
    last_byte = last_byte >> 24;
    putchar(last_byte); 
}

/* read_compressed_file()
 * Purpose: Read in information from a compressed binary file. 
 *          Store file's information in a new Pnm_ppm. 
 * Parameters: file pointer
 * Returns: pixmap 
 */
Pnm_ppm read_compressed_file(FILE *fp)
{
    unsigned height, width; 
    int read = fscanf(fp, "COMP40 Compressed image format 2\n%u %u", 
                      &width, &height); 
    assert(read == 2); 
    int c = getc(fp); 
    assert(c == '\n'); 

    A2Methods_T methods = uarray2_methods_plain;
    A2Methods_UArray2 array = methods->new(width, height, sizeof(uint32_t));
    
    methods->map_row_major(array, read_32bit, fp);   

    Pnm_ppm pixmap = malloc(sizeof(struct Pnm_ppm));
    pixmap->width = width;
    pixmap->height = height;
    pixmap->denominator = 255;
    pixmap->pixels = array;
    pixmap->methods = methods;

    assert((unsigned)methods->width(array) == width); 
    assert((unsigned)methods->height(array) == height); 
    
    return pixmap;
}

/* read_32bit()
 * Purpose: an apply function that read in the 32-bit code words in 
 *          sequence and store each word in a 2D array. 
 * Parameters: col of 2d array, row of 2d array, Uarray2, 32 bit word, 
 *             file pointer
 * Returns: none
 * Notes: Since we write a single byte using putchar(), we read in 
 *        1 byte using getc().  
 */
void read_32bit (int col, int row, A2Methods_UArray2 u2, void *elem, void *fp)
{
    (void)col;
    (void)row;
    (void) u2;

    FILE *file_p = (FILE *)fp;
    assert(file_p != NULL);

    uint32_t *location = (uint32_t*)elem;
    uint32_t first_byte = (uint32_t)getc(file_p);
    uint32_t second_byte = (uint32_t)getc(file_p);
    uint32_t third_byte = (uint32_t)getc(file_p);
    uint32_t last_byte = (uint32_t)getc(file_p);

    uint32_t word = 0;
    word = Bitpack_newu(word, 8, 24, first_byte);
    word = Bitpack_newu(word, 8, 16, second_byte); 
    word = Bitpack_newu(word, 8, 8, third_byte);
    word = Bitpack_newu(word, 8, 0, last_byte);

    *location = word;
}

/* push_into_range()
 * Purpose: Given a value and its max and min, push it into its range
 * Parameters: The value, the value's max, and the value's min
 * Returns: none  
 */
void push_into_range(float *value, float max, float min)
{
    if (*value > max) {
        *value = max;
    }
    if (*value < min) {
        *value = min;
    }
}

/* free_old_array()
 * Purpose: Given the old array and free memory 
 * Parameters: Uarray2b, methods
 * Returns: none  
 */
void free_old_array (A2Methods_UArray2 old_array, A2Methods_T methods)
{
    if (old_array != NULL){
        methods->free(&old_array); 
    }
}