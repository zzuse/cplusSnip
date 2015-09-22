/*
LONG LONG
http://h21007.www2.hp.com/portal/site/dspp/menuitem.863c3e4cbcdc3f3515b49c108973a801?ciid=4708852bcbe02110852bcbe02110275d6e10RCRD

There are a few standard things you want to be able to do with long long datatypes. One of them is to convert a value from an ASCII string. Unfortunately, there is currently no atoll or strtoll or their reverse counterparts. But here is a possible solution.

To use the functionality described below, you'll have to use the -Ae (Ansi with hp extensions) compiler switch.

 *      A possible version of the library routine atoll
 *                for illustration purposes.
 *                  */
#define MAX_LEN 10
#include <stdio.h>
#ifdef TEST
main()
{
    char      inp[MAX_LEN], *cp;
    int       c;
    long long nbr;
    long long atoll(char *);  /* Needs char pointer
                                 as arg */
    /* cp points to start of array  */
    cp=inp;
    printf("Please enter a number: ");
    while (( c=getchar()) != '\n')
        *cp++ = c;      /* read in number as string */
    *cp = '\0';
    /* call atoll to convert to long long */
    nbr = atoll (inp);
    printf("The number entered is %lld\n", nbr);
}
#endif
long long atoll( char *ca )
{
    long long ig=0;
    int       sign=1;
    /* test for prefixing white space */
    while (*ca == ' ' || *ca == '\t' )
        ca++;
    /* Check sign entered or no */
    if ( *ca == '-' )
        sign = -1;
    /* convert string to int */
    while (*ca != '\0')
        if (*ca >= '0' && *ca <= '9')
            ig = ig * 10LL + *ca++ - '0';
        else
            ca++;
    return (ig*(LL)sign);
}
