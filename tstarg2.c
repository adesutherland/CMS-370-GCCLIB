#include <stdio.h>
#include <string.h>
#include <cmssys.h>

int main(int argc, char *argv[]) {
 if (argc != 4) return 2;
 if (CMScalltype() == 0) {
   if (strcmp(argv[0],"TSTARG2")) return 3;
   if (strcmp(argv[1],"ARGUMENT")) return 4;
   if (strcmp(argv[2],"ARGUMENT")) return 5;
   if (strcmp(argv[3],"ARGUMENT")) return 6;
 }
 else if (CMScalltype() == 5) {
   if (strcmp(argv[1],"Argument 1")) return 7;
   if (strcmp(argv[2],"Argument 2")) return 8;
   if (strcmp(argv[3],"Argument 3")) return 9;
   if (!strcmp(argv[0],"stringreturn")) {
     CMSreturnvalue("text return value");
   }
   else if (!strcmp(argv[0],"isproc")) {
      if (!CMSisproc()) return 20;
   }
   else if (!strcmp(argv[0],"isfunc")) {
      if (CMSisproc()) return 21;
   }
   else if (strcmp(argv[0],"tstarg2")) return 10;
 }
 else {
   if (strcmp(argv[0],"tstarg2")) return 11;
   if (strcmp(argv[1],"Argument1")) return 12;
   if (strcmp(argv[2],"Argument2")) return 13;
   if (strcmp(argv[3],"Argument3")) return 14;
 }

 if (CMSargc() != argc) return 15;

 if (CMSargv() != argv) return 16;

 if (strncmp(CMSplist(),"TSTARG2 ",8)) return 17;

 if (CMScalltype() != 0) {
   if ( strncmp(CMSeplist()->Command,"TSTARG2",7) &&
        strncmp(CMSeplist()->Command,"tstarg2",7) )
        return 19;
 }

 return 1; /* 1 = OK - this tests that rc is returned ok */
}
