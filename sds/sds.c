#include "sds.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sdshdr {
    uint32_t len;   /* used */
    uint32_t alloc; /* excluding the header and null terminator */
    char buf[];
};

#define SDS_HDR(s) ((struct sdshdr *)((s)-(sizeof(struct sdshdr))))
#define SDS_BYTES(n) ((n) + sizeof(struct sdshdr) + 1)

static sds sdsMakeRoomFor(sds s, size_t addlen) {
    assert(s != NULL);
    struct sdshdr *sh = SDS_HDR(s);

    if (sh->alloc - sh->len >= addlen) return s;

    size_t newlen = sh->len + addlen;
    if (newlen < SDS_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += SDS_MAX_PREALLOC;

    assert(newlen > sh->len + addlen); /* Catch size_t overflow */

    struct sdshdr *newsh = realloc(sh, SDS_BYTES(newlen));
    if (newsh == NULL) return NULL;
    newsh->alloc = newlen;
    return newsh->buf;
}

sds sdsnewlen(const char *init, uint32_t initlen) {
    if (init == NULL) return NULL;

    struct sdshdr *sh = malloc(SDS_BYTES(initlen));
    if (sh == NULL) return NULL;

    sh->len = initlen;
    sh->alloc = initlen;
    if (initlen)
        memcpy(sh->buf, init, initlen);
    sh->buf[initlen] = '\0';
    return sh->buf;
}

sds sdsnew(const char *init) {
    if (init == NULL) return NULL;
    return sdsnewlen(init, (uint32_t) strlen(init));
}

void sdsfree(sds s) {
    if (s == NULL) return;
    free(SDS_HDR(s));
}

uint32_t sdslen(const sds s) {
    assert(s != NULL);
    return SDS_HDR(s)->len;
}

static sds sdsnewfmtva(const char *fmt, va_list ap) {
    size_t buflen = strlen(fmt) * 2;
    int bufstrlen;
    sds s = sdsnew("");
    s = sdsMakeRoomFor(s, buflen);
    if (s == NULL) return NULL;

    while(1) {
        va_list cpy;
        va_copy(cpy, ap);
        bufstrlen = vsnprintf(s, buflen, fmt, cpy);
        va_end(cpy);
        if (bufstrlen < 0) {
            sdsfree(s);
            return NULL;
        }
        if (((size_t)bufstrlen) >= buflen) {
            buflen = ((size_t)bufstrlen) + 1;
            s = sdsMakeRoomFor(s, buflen);
            if (s == NULL) return NULL;
            continue;
        }
        break;
    }
    SDS_HDR(s)->len = bufstrlen;
    return s;
}

sds sdsnewfmt(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    sds s = sdsnewfmtva(fmt, ap);
    va_end(ap);
    return s;
}

sds sdsncat(sds s, const char *t, size_t n) {
    s = sdsMakeRoomFor(s, n);
    if (s == NULL) return NULL;
    memcpy(s + sdslen(s), t, n);
    struct sdshdr *sh = SDS_HDR(s);
    sh->len += n;
    s[sh->len] = '\0';
    return s;
}

sds sdscat(sds s, const char *t) {
    if (s == NULL) return sdsnew(t);
    return sdsncat(s, t, strlen(t));
}

sds sdstrim(sds s, const char *cset) {
    char *end, *sp, *ep;
    size_t len;

    sp = s;
    ep = end = s + sdslen(s) - 1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > sp && strchr(cset, *ep)) ep--;
    len = (ep - sp) + 1;
    if (s != sp) memmove(s, sp, len);
    s[len] = '\0';
    SDS_HDR(s)->len = len;
    return s;
}

int sdscmp(const sds s1, const sds s2) {
    size_t l1 = sdslen(s1), l2 = sdslen(s2);
    size_t minlen = (l1 < l2) ? l1 : l2;
    int cmp = memcmp(s1, s2, minlen);
    if (cmp == 0) return l1 > l2 ? 1 : (l1 < l2 ? -1 : 0);
    return cmp;
}

sds sdsreplace(sds s, const char *replace, const char *with) {
    assert(s != NULL && replace != NULL && with != NULL);

    struct sdshdr *sh = SDS_HDR(s);
    size_t rep_len = strlen(replace);
    size_t with_len = strlen(with);
    char *t = s;

    // Fast path, same size replacement.
    if (rep_len == with_len) {
        while ((t = strstr(t, replace)) != NULL) {
            memcpy(t, with, rep_len);
            t += rep_len;
        }
        return s;
    }

    // Calculate new string size.
    int cnt = 0;
    while ((t = strstr(t, replace)) != NULL) {
        t += rep_len;
        cnt++;
    }

    // No match.
    if (cnt == 0) return s;

    int newlen = sh->len + (with_len - rep_len)*cnt;
    struct sdshdr *newsh = malloc(SDS_BYTES(newlen));
    if (newsh == NULL) return NULL;
    newsh->len = newlen;
    newsh->alloc = newlen;

    t = s;
    char *dest = newsh->buf;
    while (cnt--) {
        int len_unmatch = strstr(t, replace) - t;
        memcpy(dest, t, len_unmatch);
        dest += len_unmatch;
        memcpy(dest, with, with_len);
        dest += with_len;
        t += len_unmatch + rep_len;
    }
    memcpy(dest, t, s + sh->len - t + 1);   /* include '\0' */

    sdsfree(s);
    return newsh->buf;
}
