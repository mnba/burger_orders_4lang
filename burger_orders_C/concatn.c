/** == A generic function ==
    a useful variadic function to concatenate any number of strings:
    From: http://stackoverflow.com/questions/8465006/how-to-concatenate-2-strings-in-c
 */

#include <stdlib.h>       // calloc
#include <stdarg.h>       // va_*
#include <string.h>       // strlen, strcpy
#include <stdbool.h> //for true and false

char* concatn(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

// Usage:

#include <stdio.h> // printf

void println(char *line)
{
    printf("%s\n", line);
}

bool test2()
{
    char* str;

    str = concatn(0);             println(str); free(str);
    str = concatn(1,"a");         println(str); free(str);
    str = concatn(2,"a","b");     println(str); free(str);
    str = concatn(3,"a","b","c"); println(str); free(str);

    return true;
}

#if 0
int main()
{
    test2();
    return 0;
}
#endif
