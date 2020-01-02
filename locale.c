/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  locale.c - implementation of stuff in locale.h                   */
/*                                                                   */
/*********************************************************************/

#include "locale.h"
#include "limits.h"
#include "string.h"

static struct lconv thislocale = {
    ".",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX,
    CHAR_MAX
};

char *setlocale(int category, const char *locale)
{
    (void)category;
    if (locale == NULL)
    {
        return ("C");
    }
    else if ((strcmp(locale, "C") == 0)
             || (strcmp(locale, "") == 0))
    {
        return ("C");
    }
    else
    {
        return (NULL);
    }
}

struct lconv *localeconv(void)
{
    return (&thislocale);
}
