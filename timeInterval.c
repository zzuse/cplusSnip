#include <time.h>
{
    struct timeval lt1;
    gettimeofday(&lt1, NULL);
    stucssubsfreeze.Select( table_name, 'U'  );
    struct timeval lt2;
    gettimeofday(&lt2, NULL);
    freezetime_S += (lt2.tv_sec - lt1.tv_sec)*1.0 + (lt2.tv_usec - lt1.tv_usec)/1000000.0;
    char buffer[30];
    struct timeval tv;
    time_t curtime;
    gettimeofday(&tv, NULL);
    curtime=tv.tv_sec;
    strftime(buffer,30,"%m-%d-%Y  %T.",localtime(&curtime));
    printf("%s%ld\n",buffer,tv.tv_usec);
}
