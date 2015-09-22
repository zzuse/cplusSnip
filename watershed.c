/*---------------------------------------------------------------------------*/
/* Program:  watershed.c                                                    */
/*                                                                          */
/* Purpose:  This program calculates the watershed regions of an image.      */
/*          Some presmoothing should be performed to reduce the effects    */
/*          of plateaus in the image.                                      */
/*                                                                          */
/* Author:  John Gauch                                                      */
/*                                                                          */
/* Date:    April 27, 1994    - Original program.                          */
/*          February 10, 1995 - Added gradient watersheds                  */
/*                                                                          */
/* Note:    Copyright (C) The University of Kansas, 1994-1995              */
/*---------------------------------------------------------------------------*/
#include <IM.h>

/*---------------------------------------------------------------------------*/
/* Purpose:  This is the main program.                                      */
/*---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    /* Image variables */
    char Name1[50];
    char Name2[50];
    IM_TYPE *Image1;
    IM_TYPE *Image2;
    IM_TYPE *Image3;
    FLOAT_TYPE **Data1;
    SHORT_TYPE **Data2;
    FLOAT_TYPE **Data3;
    int PixType, Xdim, Ydim, Zdim, DimCnt;

    /* Program variables */
    int Debug = FALSE;
    int Invert = FALSE;
    int Gradient = FALSE;
    int i = 0, x, y, X, Y;
    int MinCount;
    int MinIndex;
    float MinValue;
    int Count;
    float TotalCnt = 0;
    float Dx, Dy;

    /* Interpret program options */
    printf("WATERSHED Program - KUIM Version 2.0\n\n");
    while ((++i < argc) && (argv[i][0] == '-'))
        switch (argv[i][1])
        {
            case 'i':
                Invert = TRUE;
                break;
            case 'g':
                Gradient = TRUE;
                break;
            case 'd':
                Debug = TRUE;
                break;
            default:
                Error("Invalid option encountered");
                break;
        }

    /* Check number of file names */
    if (argc - i != 2)
    {
        fprintf(stderr, "Usage: watershed [options] infile outfile\n");
        fprintf(stderr, "      [-d]  Print debugging information\n");
        fprintf(stderr, "      [-i]  Invert image before finding watersheds\n");
        fprintf(stderr, "      [-g]  Calculate gradient watersheds\n");
        exit(1);
    }

    /* Get image file names from argument list */
    if (sscanf(argv[i++], "%s", Name1) == 0)
        Error("Could not get input file name");
    if (sscanf(argv[i++], "%s", Name2) == 0)
        Error("Could not get output file name");

    /* Read input image */
    Image1 = im_open(Name1, &PixType, &Xdim, &Ydim, &Zdim, &DimCnt);
    if (DimCnt != 2)
        Error("Can not process 1D or 3D images");
    Data1 = (FLOAT_TYPE **) im_alloc2D(Image1, FLOAT);
    im_read(Image1, FLOAT, (char *) &(Data1[0][0]));

    /* Create output image */
    Image2 = im_create(Name2, SHORT, Xdim, Ydim, Zdim);
    Data2 = (SHORT_TYPE **) im_alloc2D(Image2, SHORT);
    Image3 = im_create("/dev/null", FLOAT, Xdim, Ydim, Zdim);
    Data3 = (FLOAT_TYPE **) im_alloc2D(Image3, FLOAT);

    /* Initialize output image */
    for (y = 0; y < Ydim; y++)
        for (x = 0; x < Xdim; x++)
            Data2[y][x] = 0;

    /* Invert input image data */
    if (Invert == TRUE)
        for (y = 0; y < Ydim; y++)
            for (x = 0; x < Xdim; x++)
                Data1[y][x] = -Data1[y][x];

    /* Calculate gradient magnitude image */
    if (Gradient == TRUE)
    {
        for (y = 1; y < (Ydim - 1); y++)
            for (x = 1; x < (Xdim - 1); x++)
            {
                Dx = (Data1[y + 1][x + 1] + 2 * Data1[y][x + 1] + Data1[y - 1][x + 1]
                        - Data1[y + 1][x - 1] - 2 * Data1[y][x - 1] - Data1[y - 1][x - 1]) / 8;
                Dy = (Data1[y + 1][x + 1] + 2 * Data1[y + 1][x] + Data1[y + 1][x - 1]
                        - Data1[y - 1][x + 1] - 2 * Data1[y - 1][x] - Data1[y - 1][x - 1]) / 8;
                Data3[y][x] = (float) sqrt((double) (Dx * Dx + Dy * Dy));
            }
        for (y = 1; y < (Ydim - 1); y++)
            for (x = 1; x < (Xdim - 1); x++)
                Data1[y][x] = Data3[y][x];
        for (x = 0; x < Xdim; x++)
        {
            Data1[0][x] = Data1[1][x];
            Data1[Ydim - 1][x] = Data1[Ydim - 2][x];
        }
        for (y = 0; y < Ydim; y++)
        {
            Data1[y][0] = Data1[y][1];
            Data1[y][Xdim - 1] = Data1[y][Xdim - 2];
        }
    }

    /* Mark boundary as pseudo-minima */
    MinCount = 1;
    for (y = 0; y < Ydim; y++)
        Data2[y][0] = Data2[y][Xdim - 1] = MinCount;
    for (x = 0; x < Xdim; x++)
        Data2[0][x] = Data2[Ydim - 1][x] = MinCount;

    /* Find local minima and gradient for each pixel */
    for (y = 1; y < (Ydim - 1); y++)
        for (x = 1; x < (Xdim - 1); x++)
        {
            MinValue = Data1[y][x];
            MinIndex = 4;
            for (Y = 0; Y < 3; Y++)
                for (X = 0; X < 3; X++)
                    if (Data1[y + Y - 1][x + X - 1] <= MinValue)
                    {
                        MinValue = Data1[y + Y - 1][x + X - 1];
                        MinIndex = Y * 3 + X;
                    }
            if (MinIndex == 4)
                Data2[y][x] = (++MinCount);
            else
                Data2[y][x] = -MinIndex;
        }

    /* Follow gradient downhill for each pixel */
    for (y = 1; y < (Ydim - 1); y++)
        for (x = 1; x < (Xdim - 1); x++)
        {
            X = x;
            Y = y;
            Count = 0;
            while (Data2[Y][X] <= 0)
            {
                switch (Data2[Y][X])
                {
                    case 0:
                        X--;
                        Y--;
                        break;
                    case -1:
                        Y--;
                        break;
                    case -2:
                        X++;
                        Y--;
                        break;
                    case -3:
                        X--;
                        break;
                    case -5:
                        X++;
                        break;
                    case -6:
                        X--;
                        Y++;
                        break;
                    case -7:
                        Y++;
                        break;
                    case -8:
                        X++;
                        Y++;
                        break;
                }
                Count++;
            }
            Data2[y][x] = Data2[Y][X];
            TotalCnt += Count;
        }

    /* Write information to output image */
    if (Debug == TRUE)
        printf("mincount = %d\n", MinCount);
    if (Debug == TRUE)
        printf("ave count = %f\n", TotalCnt / ((Xdim - 1) * (Ydim - 1)));
    im_write(Image2, SHORT, (char *) &(Data2[0][0]));
    im_free2D((char **) Data1);
    im_free2D((char **) Data2);
    im_free2D((char **) Data3);
    return (0);
}


