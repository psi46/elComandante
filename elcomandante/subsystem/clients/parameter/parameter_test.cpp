#include "parameter_t.h"
#include <stdlib.h>
#include <stdio.h>


int main(){
   int test1 = 0;
   int test2 = 0;
   float test3 = -8.5;
   double test4 = -6.999;
   
   printf("test1 %d, test2 %d\n", test1, test2);
   printf("test3 %f, test4 %lf\n", test3, test4);
   
   parameter_t* myparm = new parameter_t("test1", &test1, TYPE_INT);
//    parameter_t* myparm2 = new parameter_t("test2", &test2, TYPE_INT);
   myparm -> add("test2", &test2, TYPE_INT);
   
   myparm -> add("test3", &test3, TYPE_FLOAT);
   myparm -> add("test4", &test4, TYPE_DOUBLE);
   
   int value = 5;
   myparm -> setValue("test67", &value);
   printf("test1 %d, test2 %d\n", test1, test2);
   myparm -> setValue("test2", &value);
   printf("test1 %d, test2 %d\n", test1, test2);
   
   float valuef = -4.999;
   double valued = -4.999;
   myparm -> setValue("test3", &valuef);
   printf("test3 %f, test4 %lf\n", test3, test4);
   myparm -> setValue("test4", &valued);
   printf("test3 %f, test4 %lf\n", test3, test4);
   
   return 0;
}
