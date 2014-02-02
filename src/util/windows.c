#ifdef WIN32

#include <stdlib.h>
#include <string.h>

// pulled from glibc, strsep.c
char *strsep(char **stringp, const char *delim)
{
    char *begin, *end;

    begin = *stringp;
    if (begin == NULL) {
        return NULL;
    }

    /* A frequent case is when the delimiter string contains only one
        character.  Here we don't need to call the expensive `strpbrk'
        function and instead work using `strchr'.  */
    if (delim[0] == '\0' || delim[1] == '\0'){
        char ch = delim[0];

        if (ch == '\0') {
	    end = NULL;
        } else {
	    if (*begin == ch) {
	        end = begin;
	    } else if (*begin == '\0') {
	        end = NULL;
            } else {
	        end = strchr (begin + 1, ch);
            }
	}
    }
    else {
        /* Find the end of the token.  */
        end = strpbrk (begin, delim);
    }

    if (end){
        /* Terminate the token and set *STRINGP past NUL character.  */
        *end++ = '\0';
        *stringp = end;
    } else {
        /* No more delimiters; this is the last token.  */
        *stringp = NULL;
    }

  return begin;
}

// http://stackoverflow.com/a/12979321
char *strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;

    if (str == NULL) {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0') {
        return NULL;
    }

    ret = str;
    str += strcspn(str, delim);

    if (*str) {
        *str++ = '\0';
    }

    *nextp = str;
    return ret;
}

size_t strnlen(const char *str, size_t maxlen) // ripped off from glibc, stripped down significantly
{
    const char *char_ptr, *end_ptr = str + maxlen;
    if (maxlen == 0)
        return 0;
    if (end_ptr < str)
        end_ptr = (const char *) ~0UL;
    for (char_ptr = str; ; ++char_ptr) {
        if (*char_ptr == '\0') {
            if (char_ptr > end_ptr)
                char_ptr = end_ptr;
            return char_ptr - str;
        }
    }
    return end_ptr - str;
}

char *strdup(const char* str)
{
    char *new = malloc(strlen(str) + 1);
    strcpy(new, str);
    return new;
}

char *strndup(const char *str, size_t len)
{
    char *new = malloc(strnlen(str, len) + 1);
    strncpy(new, str, len);
    return new;
}

/*
* Copyright (c) 2004 Darren Tucker.
*
* Based originally on asprintf.c from OpenBSD:
* Copyright (c) 1997 Todd C. Miller <Todd.Miller AT courtesan.com>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

extern int vsnprintf();

#ifndef HAVE_VASPRINTF

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef VA_COPY
# ifdef HAVE_VA_COPY
#  define VA_COPY(dest, src) va_copy(dest, src)
# else
#  ifdef HAVE___VA_COPY
#   define VA_COPY(dest, src) __va_copy(dest, src)
#  else
#   define VA_COPY(dest, src) (dest) = (src)
#  endif
# endif
#endif

#define INIT_SZ 128

int
vasprintf(char **str, const char *fmt, va_list ap)
{
    int ret = -1;
    va_list ap2;
    char *string, *newstr;
    size_t len;

    VA_COPY(ap2, ap);
    if ((string = calloc(1, INIT_SZ)) == NULL)
        goto fail;

    ret = vsnprintf(string, INIT_SZ, fmt, ap2);
    if (ret >= 0 && ret < INIT_SZ) { /* succeeded with initial alloc */
        *str = string;
    } else if (ret == INT_MAX || ret < 0) { /* Bad length */
        goto fail;
    } else {        /* bigger than initial, realloc allowing for nul */
        len = (size_t)ret + 1;
        if ((newstr = realloc(string, len)) == NULL) {
            free(string);
            goto fail;
        } else {
            va_end(ap2);
            VA_COPY(ap2, ap);
            ret = vsnprintf(newstr, len, fmt, ap2);
            if (ret >= 0 && (size_t)ret < len) {
                *str = newstr;
            } else { /* failed with realloc'ed string, give up */
                free(newstr);
                goto fail;
            }
        }
    }
    va_end(ap2);
    return (ret);

fail:
    *str = NULL;
    errno = ENOMEM;
    va_end(ap2);
    return (-1);
}
#endif

/* Include asprintf() if not on your OS. */
#ifndef HAVE_ASPRINTF
int asprintf(char **str, const char *fmt, ...)
{
    va_list ap;
    int ret;

    *str = NULL;
    va_start(ap, fmt);
    ret = vasprintf(str, fmt, ap);
    va_end(ap);

    return ret;
}
#endif

#endif

