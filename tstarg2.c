#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
 if (argc != 4) return 2;
 if (strcmp(argv[0],"tstarg2")) return 3;
 if (strcmp(argv[1],"Argument1")) return 4;
 if (strcmp(argv[2],"Argument2")) return 5;
 if (strcmp(argv[3],"Argument3")) return 6;
 return 1; /* 1 = OK - this tests that rc is returned ok */
}
