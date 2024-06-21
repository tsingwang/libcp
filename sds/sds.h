#ifndef __SDS_H
#define __SDS_H

#include <stddef.h>
#include <stdint.h>

#define SDS_MAX_PREALLOC 1024

typedef char *sds;

sds sdsnewlen(const char *init, uint32_t initlen);
sds sdsnew(const char *init);
void sdsfree(sds s);
uint32_t sdslen(const sds s);
sds sdsnewfmt(const char *fmt, ...);
sds sdsncat(sds s, const char *t, size_t n);
sds sdscat(sds s, const char *t);
sds sdstrim(sds s, const char *cset);
int sdscmp(const sds s1, const sds s2);
sds sdsreplace(sds s, const char *replace, const char *with);

#endif
