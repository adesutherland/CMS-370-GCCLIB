/**************************************************************************************************/
/* GCCLIB Test Suite - Header                                                                     */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/

/* TEST INFRASTRUCTURE */
extern int LEVEL1;
extern int LEVEL2;
extern int LEVEL3;
extern char *LEVEL1_D;
extern char *LEVEL2_D;
extern char *LEVEL3_D;
extern int ERRORS;
extern int OK;

void REPORT();

void SEC_STRT(char *title);

void SUB_STRT(char *title);

void TST_STRT(char *title);

void TST_OK();

void _TST_ERR(char *s, char *file, int line);

void _TST_PERR(char *file, int line);

#define TST_ERR(s) _TST_ERR((s), __FILENAME__, __LINE__)
#define TST_PERR() _TST_PERR(__FILENAME__, __LINE__)

void MAKEFILE(char *id);

void DELFILE(char *id);

char *TRIM(char *str);

#define ASSERTNOTNULL(msg, init, t, clean) {TST_STRT((msg)); init; if (!(t)) TST_ERR("Value is NULL"); else TST_OK(); clean;}
#define ASSERTNOTNULLP(msg, init, t, clean) {TST_STRT((msg)); init; if (!(t)) TST_PERR(); else TST_OK(); clean;}
#define ASSERTNULL(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)) TST_ERR("Value is not NULL"); else TST_OK(); clean;}
#define ASSERTNULLP(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)) TST_PERR(); else TST_OK(); clean;}

#define ASSERTTRUE(msg, init, t, clean) {TST_STRT((msg)); init; if (!(t)) TST_ERR("Value is False"); else TST_OK(); clean;}
#define ASSERTFALSE(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)) TST_ERR("Value is True"); else TST_OK(); clean;}

#define ASSERTEOF(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)!=EOF) TST_ERR("Not EOF"); else TST_OK(); clean;}
#define ASSERTEOFP(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)!=EOF) TST_PERR(); else TST_OK(); clean;}
#define ASSERTNOTEOF(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)==EOF) TST_ERR("Is EOF"); else TST_OK(); clean;}
#define ASSERTNOTEOFP(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)==EOF) TST_PERR(); else TST_OK(); clean;}

#define ASSERTZERO(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)) TST_ERR("Not Zero"); else TST_OK(); clean;}
#define ASSERTZEROP(msg, init, t, clean) {TST_STRT((msg)); init; if ((t)) TST_PERR(); else TST_OK(); clean;}
#define ASSERTNOTZERO(msg, init, t, clean) {TST_STRT((msg)); init; if (!(t)) TST_ERR("Is Zero"); else TST_OK(); clean;}
#define ASSERTNOTZEROP(msg, init, t, clean) {TST_STRT((msg)); init; if (!(t)) TST_PERR(); else TST_OK(); clean;}




/* IO TESTS ASSETS */
#define TEST_STRING "123456789+123456789+123456789+123456789+123456789+123456789+123456789+123456789+123456789+123456789+"
#define TEST_STRING_NL TEST_STRING "\n"
#define TEST_SHORT_STRING "123456789+123456789+"
#define TEST_SHORT_STRING_NL TEST_SHORT_STRING "\n"

char *FGETC_RD(char *in, int len, FILE *test);

int FPUTC_WR(char *out, FILE *test);

extern char *S_INPUT[];
extern char *L_INPUT;

int FGETS_T(FILE *test, int lrecl);

int FPUTS_T(FILE *test);

int FPUTC_T(FILE *test);

void CMSSYS_T();

void IO_CON_T();

void IO_PUN_T();

void IO_PRT_T();

void IO_RDR_T();

void IO_VFL_T();

void IO_FFL_T();

void IO_COM_T();
