#ifndef __SDS_H
#define __SDS_H

#include <stddef.h>

#define SDS_MAX_PREALLOC 1024

typedef char *sds;

sds sdsnewlen(const char *init, size_t initlen);
sds sdsnew(const char *init);
/* Duplicate an sds including embedded '\0' bytes. */
sds sdsdup(const sds s);
void sdsfree(sds s);
size_t sdslen(const sds s);
size_t sdsavail(const sds s);
size_t sdsalloc(const sds s);
/* Ensure total payload capacity is at least capacity bytes. */
sds sdsreserve(sds s, size_t capacity);
void sdsclear(sds s);
sds sdsnewfmt(const char *fmt, ...);
sds sdsncat(sds s, const char *t, size_t n);
sds sdscat(sds s, const char *t);
sds sdstrim(sds s, const char *cset);
int sdscmp(const sds s1, const sds s2);
sds sdsreplacelen(sds s, const void *replace, size_t replace_len,
                  const void *with, size_t with_len);
sds sdsreplace(sds s, const char *replace, const char *with);
sds sdsjoinlen(const sds *parts, size_t count, const void *sep, size_t sep_len);
sds sdsjoin(const sds *parts, size_t count, const char *sep);
sds *sdssplitlen(const void *s, size_t len, const void *sep, size_t sep_len,
                 size_t *count);
sds *sdssplit(const char *s, const char *sep, size_t *count);
void sdssplitfree(sds *parts, size_t count);

#endif
