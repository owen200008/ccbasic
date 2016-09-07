/////////////////////////////////////////////////////////////////////////////////////
#if defined(LINUX) || defined(__MAC) || defined(__ANDROID)
#include "../../../def/types/tchar.h"
/////////////////////////////////////////////////////////////////////////////////////

char * __cdecl _strlwr (
        char * string
        )
{
#if defined (_WIN32)

        int dstlen;                 /* len of dst string, with null  */
        unsigned char *dst = NULL;  /* destination string */
#if defined (_MT)
        int local_lock_flag;
#endif  /* defined (_MT) */

        if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
        {
        char *cp;               /* traverses string for C locale conversion */

                for (cp=string; *cp; ++cp)
                {
                        if ('A' <= *cp && *cp <= 'Z')
                                *cp += 'a' - 'A';
                }

                return(string);
        } /* C locale */

        _lock_locale( local_lock_flag )

#if defined (_MT)

        if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
        {
        char *cp;               /* traverses string for C locale conversion */
                _unlock_locale( local_lock_flag )

                for (cp=string; *cp; ++cp)
                {
                        if ('A' <= *cp && *cp <= 'Z')
                                *cp += 'a' - 'A';
                }

                return(string);
        } /* C locale */
#endif  /* defined (_MT) */
f (0 == (dstlen = __crtLCMapStringA(__lc_handle[LC_CTYPE], LCMAP_LOWERCASE,
            string, -1, NULL, 0, 0, TRUE)))
                goto error_cleanup;

        /* Allocate space for dst */
        if (NULL == (dst = (unsigned char *) _malloc_crt(dstlen*sizeof(unsigned char))))
                goto error_cleanup;

        /* Map src string to dst string in alternate case */
        if (0 == __crtLCMapStringA(__lc_handle[LC_CTYPE], LCMAP_LOWERCASE,
            string, -1, dst, dstlen, 0, TRUE))
                goto error_cleanup;

        /* copy dst string to return string */
        strcpy(string, dst);

error_cleanup:
        _unlock_locale( local_lock_flag )
        _free_crt (dst);

        return (string);

#else  /* defined (_WIN32) */

        char * cp;

        for (cp=string; *cp; ++cp)
        {
                if ('A' <= *cp && *cp <= 'Z')
                        *cp += 'a' - 'A';
        }

        return(string);


#endif  /* defined (_WIN32) */
}

char * __cdecl _strupr (
    char * string
    )
{
#if defined (_WIN32)

    int dstlen;                 /* len of dst string, with null  */
    unsigned char *dst = NULL;  /* wide version of string in alternate case */
#if defined (_MT)
    int local_lock_flag;
#endif  /* defined (_MT) */

    if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
    {
        char *cp;       /* traverses string for C locale conversion */
        for (cp = string; *cp; ++cp)
        {
            if ('a' <= *cp && *cp <= 'z')
                *cp += 'A' - 'a';
        }
        return(string);
    } /* C locale */

    _lock_locale( local_lock_flag )

#if defined (_MT)
    if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
    {
        char *cp;       /* traverses string for C locale conversion */
        _unlock_locale( local_lock_flag )

        for (cp=string; *cp; ++cp)
        {
            if ('a' <= *cp && *cp <= 'z')
                *cp += 'A' - 'a';
        }

        return(string);
    } /* C locale */
#endif  /* defined (_MT) */
f (0 == (dstlen =__crtLCMapStringA(__lc_handle[LC_CTYPE],
                                       LCMAP_UPPERCASE, string, -1,
                                       NULL, 0, 0, TRUE)))
        goto error_cleanup;

    /* Allocate space for dst */
    if (NULL == (dst = (unsigned char *)
                             _malloc_crt(dstlen * sizeof(unsigned char))))
        goto error_cleanup;

    /* Map src string to dst string in alternate case */
    if (0 == __crtLCMapStringA(__lc_handle[LC_CTYPE], LCMAP_UPPERCASE,
            string, -1, dst, dstlen, 0, TRUE))
        goto error_cleanup;

    /* copy dst string to return string */
    strcpy(string, dst);

error_cleanup:
    _unlock_locale( local_lock_flag )
    _free_crt (dst);
    return (string);

#else  /* defined (_WIN32) */

    char * cp;

    for (cp=string; *cp; ++cp)
    {
        if ('a' <= *cp && *cp <= 'z')
            *cp += 'A' - 'a';
    }

    return(string);

#endif  /* defined (_WIN32) */
}
                                                        
char * __cdecl _strrev (
        char * string
        )
{
        char *start = string;
        char *left = string;
        char ch;

        while (*string++)                 /* find end of string */
                ;
        string -= 2;

        while (left < string)
        {
                ch = *left;
                *left++ = *string;
                *string-- = ch;
        }

        return(start);
}


#endif


