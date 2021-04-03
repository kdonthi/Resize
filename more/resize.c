// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

    //printf("Difference between bfSize and biSizeIamge: %i\n",bf.bfSize - bi.biSizeImage); //this is 54
    printf("Width before multiplying by f: %i\n", bi.biWidth); //width works

    //padding for original image
    int padding1 = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4; //works, 3

    float decimaloffactor = fmod((f * 10), 10);

    if (f > 1) //rounds float
    {
        if (decimaloffactor >= 5) //for greater than 1
        {
            f = ceil(f);
        }

        if (decimaloffactor < 5 && decimaloffactor > 0)

        {
            f = floor(f);
        }
    }
    //changes to the headers
    float biWidthf = (float) bi.biWidth;
    float biHeightf = (float) bi.biHeight;

    float originalwid = bi.biWidth;
    float originalhei = bi.biHeight;

    float o = ceil(biWidthf * f);
    float p = floor(biHeightf * f);

    bi.biWidth = (long) o; //changed bi.biWidth by factor of f
    bi.biHeight = (long) p; //changed bi.biHeight by factor of f


    //printf("Width after multipying by f: %i\n", bi.biWidth);
    //printf("Height after multiplying by f: %i\n", bi.biHeight);

    // determine padding for scanlines for f < 1
    int padding2 = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4; // we need second mod 4 in case of 0, works, 2

    int skip = (round(1 / f) - 1); //how much we skip, set for every float! (multiplied by 3 because we have to do 3 at a time)
    int timestoskip = bi.biWidth; //amount of times we want to skip in a line, can I change this to bi.biWidth?
    int toofar = (skip + 1) - (fmod(originalwid, (skip + 1)));
    int goback = fmod(toofar, (skip + 1)); //amount we went too far CHECK, repetitive
    int padding3 = (4 - (timestoskip * sizeof(RGBTRIPLE)) % 4) % 4; //perfect

    //printf("padding1: %i, padding 2: %i\n", padding1, padding2); //works
    //printf("size of RGB: %lu\n", sizeof(RGBTRIPLE)); //works

    if (f > 1)
    {
        bi.biSizeImage = (sizeof(RGBTRIPLE) * (bi.biWidth * abs(bi.biHeight))) + (padding2 * abs(bi.biHeight)); //do we malloc this?
    }

    if (f < 1)
    {
        bi.biSizeImage = (sizeof(RGBTRIPLE) * (bi.biWidth * abs(bi.biHeight))) + (padding3 * abs(bi.biHeight));
    }

    //printf("biSizeImage: %i\n", bi.biSizeImage); //works, 120
    bf.bfSize = bi.biSizeImage + 54;

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    //printf("Location of pointer: %ld", ftell(inptr));


    // iterate over infile's scanlines
    if (f >= 1)
    {
        for (int i = 0, biHeight = fabsf(originalhei); i < biHeight; i++) //multiplied by rounded float!
        {
            //repeats rows vertically
            for (int j = 0; j < f; j++) //f has been rounded already!
            {
                if (j > 0) //rewinds pointer to beggining of line
                {
                    int x = sizeof(RGBTRIPLE) * (originalwid) + padding1;
                    fseek(inptr,-x, SEEK_CUR);
                    //printf("Location of pointer: %ld", ftell(inptr));
                }
                // repeats pixels horizontally
                for (int k = 0; k < (originalwid); k++)
                {
                    // temporary storage
                    RGBTRIPLE triple;

                    // read RGB triple from infile
                    fread(&triple, sizeof(RGBTRIPLE), 1,
                    inptr); //how does this file move along the inptr automatically through a for loop? How does it keep track? How do we know where it is?

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
    }
    //DO WE NEED TO WORRY ABOUT THE BI.WIDTH?

    printf("bi.biWidth: %f", originalwid);

    if (f < 1)
    {
        printf("Location of pointer at the begginning of loop: %ld", ftell(inptr));
        for (int i = 0, biHeight = timestoskip; i < biHeight;
        i++) //we basically skip the same amount of pixels vertically as we do horizontally
        {
            for (int k = 0; k < timestoskip; k++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1,
                inptr); //how does this file move along the inptr automatically through a for loop? How does it keep track? How do we know where it is?

                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);

                fseek(inptr, skip*3, SEEK_CUR);

            }

            printf("Location of pointer before going back: %ld\n", ftell(inptr));

            fseek(inptr, -(goback*3), SEEK_CUR); //go back after skipping too much

            printf("Location of pointer after going back: %ld\n", ftell(inptr));

            // skip over padding, if any
            fseek(inptr, padding1, SEEK_CUR);

            // then add it back (to demonstrate how)
            for (int m = 0; m < padding3; m++)
            {
                fputc(0x00, outptr);
            }

            printf("Location of pointer at the end of loop: %ld", ftell(inptr));

            fseek(inptr, skip*((originalwid*3)+padding1), SEEK_CUR); //skip the amount of lines vertically as we do pixels horizontally

        }
    }
    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
