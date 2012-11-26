#include "parameter_t.h"
#include <stdio.h>
#include <string.h>

int parameter_t::no_params = 0;
parameter_t* parameter_t::end = NULL;
parameter_t* parameter_t::start = NULL;


parameter_t::parameter_t(char* buffer, void* var, int ptype){
   if(end != NULL){
      end -> next = this;
      end = this;
   }else{
      end = this;
   }
   if(start == NULL){
      start = this;
   }
   no_params ++;
   sprintf(name, "%s", buffer);
   variable = var;
   type = ptype;
   next = NULL;
   printf("New parameter @ %p with name %s and type %d\n", variable, name, type);
}
int parameter_t::add(char* buffer, void* var, int ptype){
   new parameter_t(buffer, var, ptype);
}

parameter_t::~parameter_t(){
   if(this == start){
      start = next;
   }
   no_params--;
}
int parameter_t::setValue(char* name, void* value){
   return start->set_Value(name, value);
}
int parameter_t::set_Value(char* buffer, void* value){
   if(strcmp(name, buffer)==0){
      switch(type){
	 case TYPE_INT:
	    *(int *)variable = *(int *)value;
	    break;
	 case TYPE_FLOAT:
	    *(float *)variable = *(float *)value;
	    break;
	 case TYPE_DOUBLE:
	    *(double *)variable = *(double *)value;
	    break;
	 default:
	    printf("Type %d not implemented\n", type);
	    return -2;
      }
      return 0;
   }else if(next!=NULL){
      return next->set_Value(buffer, value);
   }else{
      return -1;
   }
}