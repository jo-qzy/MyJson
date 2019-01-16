/*************************************************************************
	> File Name: test.c
	> Author: 
	> Mail: 
	> Created Time: Tue 15 Jan 2019 02:52:34 PM CST
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>

int main()
{
    const char* end = NULL;
    printf("%lf", strtod("1.0", &end));
    return 0;
}
