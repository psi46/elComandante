/**
 * convert.h
 * function template for conversion from char* to generic type
 * Dennis Terhorst
 * 2008-06-29
 */
#ifndef CONVERT_H
#define CONVERT_H

#include <stdlib.h>

int convert(char* parm,char*             value );
int convert(char* parm,unsigned char*    value );
int convert(char* parm,char**            value );
int convert(char* parm,int*              value );
int convert(char* parm,long int*         value );
int convert(char* parm,unsigned long int*value );
int convert(char* parm,long long int*    value );
int convert(char* parm,float*            value );
int convert(char* parm,double*           value );
int convert(char* parm,long double*      value );

#endif //ndef CONVERT_H
