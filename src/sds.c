#include "sds.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* +--------+-------------------------------+-----------+
 * | Header | Binary safe C alike string... | Null term |
 * +--------+-------------------------------+-----------+
 *          |
 *          `-> Pointer returned to the user.
 */
struct sdshdr {
    size_t len;   /* used */
    size_t alloc; /* excluding the header and null terminator */
    char buf[];
};

#define SDS_HDR(s) ((struct sdshdr *)((s) - sizeof(struct sdshdr)))
#define SDS_BYTES(n) ((n) + sizeof(struct sdshdr) + 1)

static sds sds_reserve_for_additional(sds s, size_t addlen) {
    assert(s != NULL);
    struct sdshdr *sh = SDS_HDR(s);

    if (sh->alloc - sh->len >= addlen) return s;

    size_t newlen = sh->len + addlen;
    assert(newlen >= sh->len); /* Catch size_t overflow */
    if (newlen < SDS_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += SDS_MAX_PREALLOC;
    assert(newlen >= sh->len + addlen);

    struct sdshdr *newsh = realloc(sh, SDS_BYTES(newlen));
    if (newsh == NULL) return NULL;
    newsh->alloc = newlen;
    return newsh->buf;
}

static const unsigned char *sds_find_bytes(const unsigned char *haystack,
                                           size_t haystack_len,
                                           const unsigned char *needle,
                                           size_t needle_len) {
    if (needle_len == 0 || haystack_len < needle_len) return NULL;

    for (size_t i = 0; i + needle_len <= haystack_len; i++) {
        if (memcmp(haystack + i, needle, needle_len) == 0) {
            return haystack + i;
        }
    }

    return NULL;
}

sds sdsnewlen(const char *init, size_t initlen) {
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
    return sdsnewlen(init, strlen(init));
}

sds sdsdup(const sds s) {
    if (s == NULL) return NULL;
    return sdsnewlen(s, sdslen(s));
}

void sdsfree(sds s) {
    if (s == NULL) return;
    free(SDS_HDR(s));
}

size_t sdslen(const sds s) {
    assert(s != NULL);
    return SDS_HDR(s)->len;
}

size_t sdsavail(const sds s) {
    assert(s != NULL);
    return SDS_HDR(s)->alloc - SDS_HDR(s)->len;
}

size_t sdsalloc(const sds s) {
    assert(s != NULL);
    return SDS_HDR(s)->alloc;
}

sds sdsreserve(sds s, size_t capacity) {
    if (s == NULL) return NULL;
    if (capacity <= sdsalloc(s)) return s;

    sds grown = sds_reserve_for_additional(s, capacity - sdslen(s));
    return grown;
}

void sdsclear(sds s) {
    if (s == NULL) return;
    struct sdshdr *sh = SDS_HDR(s);
    sh->len = 0;
    s[0] = '\0';
}

static sds sdsnewfmtva(const char *fmt, va_list ap) {
    size_t buflen = strlen(fmt) * 2;
    int bufstrlen = 0;
    sds s = sdsnew("");

    if (s == NULL) return NULL;
    sds grown = sds_reserve_for_additional(s, buflen);
    if (grown == NULL) {
        sdsfree(s);
        return NULL;
    }
    s = grown;

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
            grown = sds_reserve_for_additional(s, buflen);
            if (grown == NULL) {
                sdsfree(s);
                return NULL;
            }
            s = grown;
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
    if (s == NULL) return sdsnewlen(t, n);
    sds grown = sds_reserve_for_additional(s, n);
    if (grown == NULL) return NULL;
    s = grown;
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
    size_t slen = sdslen(s);

    if (slen == 0) return s;

    char *sp = s;
    char *end = s + slen - 1;
    char *ep = end;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > sp && strchr(cset, *ep)) ep--;
    size_t len = (ep - sp) + 1;
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

sds sdsreplacelen(sds s, const void *replace, size_t replace_len,
                  const void *with, size_t with_len) {
    size_t count = 0;
    const unsigned char *source = (const unsigned char *)s;

    if (s == NULL || replace == NULL || with == NULL) return NULL;
    if (replace_len == 0) return s;

    struct sdshdr *sh = SDS_HDR(s);
    size_t source_len = sh->len;
    const unsigned char *cursor = source;

    while (1) {
        size_t remaining_len = source_len - (size_t)(cursor - source);
        const unsigned char *match =
            sds_find_bytes(cursor, remaining_len, replace, replace_len);
        if (match == NULL) break;
        count++;
        cursor = match + replace_len;
    }

    if (count == 0) return s;

    size_t newlen = with_len >= replace_len
                        ? source_len + (with_len - replace_len) * count
                        : source_len - (replace_len - with_len) * count;
    struct sdshdr *newsh = malloc(SDS_BYTES(newlen));
    if (newsh == NULL) return NULL;
    newsh->len = newlen;
    newsh->alloc = newlen;

    cursor = source;
    char *dest = newsh->buf;
    while (1) {
        size_t remaining_len = source_len - (size_t)(cursor - source);
        const unsigned char *match =
            sds_find_bytes(cursor, remaining_len, replace, replace_len);
        size_t unmatched_len;

        if (match == NULL) break;

        unmatched_len = (size_t)(match - cursor);
        memcpy(dest, cursor, unmatched_len);
        dest += unmatched_len;
        memcpy(dest, with, with_len);
        dest += with_len;
        cursor = match + replace_len;
    }

    memcpy(dest, cursor, source_len - (size_t)(cursor - source));
    newsh->buf[newlen] = '\0';

    sdsfree(s);
    return newsh->buf;
}

sds sdsreplace(sds s, const char *replace, const char *with) {
    assert(replace != NULL && with != NULL);
    return sdsreplacelen(s, replace, strlen(replace), with, strlen(with));
}

sds sdsjoinlen(const sds *parts, size_t count, const void *sep, size_t sep_len) {
    size_t total_len = 0;

    if ((parts == NULL && count > 0) || (sep == NULL && sep_len > 0)) return NULL;

    for (size_t i = 0; i < count; i++) {
        if (parts[i] == NULL) return NULL;
        total_len += sdslen(parts[i]);
        if (i + 1 < count) total_len += sep_len;
    }

    sds joined = sdsnew("");
    if (joined == NULL) return NULL;

    sds grown = sds_reserve_for_additional(joined, total_len);
    if (grown == NULL) {
        sdsfree(joined);
        return NULL;
    }
    joined = grown;

    char *dest = joined;
    for (size_t i = 0; i < count; i++) {
        size_t part_len = sdslen(parts[i]);
        memcpy(dest, parts[i], part_len);
        dest += part_len;
        if (i + 1 < count) {
            memcpy(dest, sep, sep_len);
            dest += sep_len;
        }
    }

    *dest = '\0';
    SDS_HDR(joined)->len = total_len;
    return joined;
}

sds sdsjoin(const sds *parts, size_t count, const char *sep) {
    if (sep == NULL) return NULL;
    return sdsjoinlen(parts, count, sep, strlen(sep));
}

sds *sdssplitlen(const void *s, size_t len, const void *sep, size_t sep_len,
                 size_t *count) {
    size_t part_count = 1;
    size_t index = 0;
    const unsigned char *source = s;

    if (count != NULL) *count = 0;
    if (s == NULL || sep == NULL || count == NULL) return NULL;
    if (sep_len == 0) return NULL;

    const unsigned char *cursor = source;
    while (1) {
        size_t remaining_len = len - (size_t)(cursor - source);
        const unsigned char *match =
            sds_find_bytes(cursor, remaining_len, sep, sep_len);
        if (match == NULL) break;
        part_count++;
        cursor = match + sep_len;
    }

    sds *parts = calloc(part_count, sizeof(*parts));
    if (parts == NULL) return NULL;

    cursor = source;
    while (1) {
        size_t remaining_len = len - (size_t)(cursor - source);
        const unsigned char *match =
            sds_find_bytes(cursor, remaining_len, sep, sep_len);
        size_t part_len = match == NULL ? remaining_len : (size_t)(match - cursor);
        sds part = sdsnewlen((const char *)cursor, part_len);
        if (part == NULL) {
            sdssplitfree(parts, index);
            return NULL;
        }

        parts[index++] = part;
        if (match == NULL) break;
        cursor = match + sep_len;
    }

    *count = index;
    return parts;
}

sds *sdssplit(const char *s, const char *sep, size_t *count) {
    if (s == NULL || sep == NULL) {
        if (count != NULL) *count = 0;
        return NULL;
    }

    return sdssplitlen(s, strlen(s), sep, strlen(sep), count);
}

void sdssplitfree(sds *parts, size_t count) {
    if (parts == NULL) return;
    for (size_t i = 0; i < count; i++) {
        sdsfree(parts[i]);
    }
    free(parts);
}
