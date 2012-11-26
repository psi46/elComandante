#ifndef PARAMETER_T_H
#define PARAMETER_T_H

#define MAX_NAMELENGTH	64

#define TYPE_DOUBLE	1
#define TYPE_FLOAT	2
#define TYPE_INT	3

class parameter_t {
   private:
      static int no_params;
      static parameter_t* start;
      static parameter_t* end;
      
      parameter_t* next;
      char name[MAX_NAMELENGTH];
      
      void* variable;
      int type;
      
   public:
      parameter_t(char* name, void* var, int type);
      ~parameter_t();
      
      int add(char* buffer, void* var, int ptype);
      
      int setValue(char* name, void* value);
      int set_Value(char* buffer, void* value);
};
#endif//ndef PARAMETER_H
