// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: copy infile outfile\n");
        return 1;
    }

    // remember filenames
    float f = atof(argv[1]);
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    printf("Difference between bfSize and biSizeIamge: %i\n",bf.bfSize - bi.biSizeImage); //this is 54
    printf("Width before multiplying by f: %i\n", bi.biWidth); //width works

    //padding for original image
    int padding1 = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4; //works, 3

    //changes to the headers
    bi.biWidth = bi.biWidth*f; //changed bi.biWidth by factor of f
    bi.biHeight = bi.biHeight*f; //changed bi.biHeight by factor of f

    printf("Width after multipying by f: %i\n", bi.biWidth);
    printf("Height after multiplying by f: %i\n", bi.biHeight);

     // determine padding for scanlines
    int padding2 = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4; // we need second mod 4 in case of 0!, works, 2

    printf("padding1: %i, padding 2: %i\n", padding1, padding2);
    printf("size of RGB: %lu\n", sizeof(RGBTRIPLE));

    bi.biSizeImage = (sizeof(RGBTRIPLE) * (bi.biWidth * abs(bi.biHeight))) + (padding2 * abs(bi.biHeight)); //do we malloc this?
    printf("biSizeImage: %i\n", bi.biSizeImage); //works, 120
    bf.bfSize = bi.biSizeImage + 54;

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    printf("Location of pointer: %ld", ftell(inptr));


    // iterate over infile's scanlines
    for (int i = 0, biHeight = (abs(bi.biHeight))/f; i < biHeight; i++)
    {
        //mulitplies the rows by f
        for (int j = 0; j < f; j++)
        {
            if (j > 0)
            {
            int x = (sizeof(RGBTRIPLE) * (bi.biWidth/f)) + padding1;
            //printf("Size of x: %i", x);
            fseek(inptr,-x, SEEK_CUR);
            //printf("Location of pointer: %ld", ftell(inptr));
            }
            // iterate over pixels in scanline
            for (int k = 0; k < ((bi.biWidth)/f); k++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr); //how does this file move along the inptr automatically through a for loop? How does it keep track? How do we know where it is?

                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);

                //multiplies the amount of things in each row by f
                for (int l = 1; l < f; l++)
                {
                    fseek(inptr, -3, SEEK_CUR); //move back one RGBTRIPLE; check if you can do -sizeof(RGBTRIPLE)

                    fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }

            }

            // skip over padding, if any
            fseek(inptr, padding1, SEEK_CUR);

            // then add it back (to demonstrate how)
            for (int m = 0; m < padding2; m++)
            {
                fputc(0x00, outptr);
            }

            //printf("Location of pointer at the end of loop: %ld", ftell(inptr));
        }
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
