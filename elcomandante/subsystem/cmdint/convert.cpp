/**
 * convert.h
 * function template for conversion from char* to generic type
 * Dennis Terhorst
 * 2008-06-29
 */
#include "convert.h"
#include <stdlib.h>
//#include <stdio.h>

int convert(char* parm, char             *ret )
{
	int tmp = strtol (parm, NULL, 0);
	*ret = tmp & 0xFF;
	//printf("convert<char>(\"%s\", %p) *ret=0x%x, tmp=0x%x\n", parm, ret, *ret, tmp );
	return 0;
}

int convert(char* parm, unsigned char    *ret )
{
	int tmp = strtol (parm, NULL, 0);
	*ret = tmp & 0xFF;
	//printf("convert<char>(\"%s\", %p) *ret=0x%x, tmp=0x%x\n", parm, ret, *ret, tmp );
	return 0;
}

int convert(char* parm, char*            *ret )
{
	*ret = parm;
	return 0;
}	// FIXME: makes const char* impossible

int convert(char* parm, int              *ret )
{
	*ret = strtol (parm, NULL, 0); 
	return 0;  
}

int convert(char* parm, long int         *ret )
{
	*ret = strtol (parm, NULL, 0);
	return 0; 
}

int convert(char* parm, unsigned long int*ret )
{
	*ret = strtoul(parm, NULL, 0);
	return 0; 
}

int convert(char* parm, long long int    *ret )
{
	*ret = strtoll(parm, NULL, 0);
	return 0; 
}

int convert(char* parm, float            *ret )
{
	*ret = strtof(parm, NULL);
	return 0; 
}

int convert(char* parm, double           *ret )
{
	*ret = strtod (parm, NULL);
	return 0; 
}

int convert(char* parm, long double      *ret )
{
	*ret = strtold(parm, NULL);
	return 0; 
}

