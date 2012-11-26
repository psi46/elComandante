#!/usr/bin/perl


foreach $efield (50,100,150,200,250,300,350,400,450,500) {

   $file_out = sprintf("magboltz_%i.out",$efield);
   open(MAGBOLTZ,"|./magboltz > $file_out");
   select(MAGBOLTZ);
   printf("%10i%10i%10.5f\n",3,10,0.0);
   printf("%5i%5i%5i%5i%5i\n",2,1,11,77,77,77);
   printf("%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f\n",95.0,3.0,,2.0,0.0,0.0,0.0,20.0,760.0);
   printf("%10.3f%10.3f%10.3f\n",$efield,0.2,0.0);
   printf("%1i\n",0);
   close(MAGBOLTZ);
   select(STDOUT);


}
