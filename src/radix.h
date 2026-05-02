#ifndef __RADIX_H
#define __RADIX_H

#include "radixmap.h"

typedef struct {
    radixmap map;
} radix;

#define radix_empty(set) radixmap_empty(&(set)->map)
#define radix_size(set) radixmap_size(&(set)->map)

/* Example:
 *   radix_add(set, "foo");
 *   radix_contains(set, "foo") == 1;
 */

/* Stack usage:
 *   radix set;
 *   radix_init(&set);
 *   radix_destroy(&set);
 */
int radix_init(radix *set);
void radix_destroy(radix *set);

/* Heap usage:
 *   radix *set = radix_new();
 *   radix_free(set);
 */
radix *radix_new(void);
void radix_free(radix *set);

int radix_copy(radix *dst, const radix *src);
radix *radix_clone(const radix *src);

int radix_add(radix *set, const char *key);
int radix_contains(const radix *set, const char *key);
int radix_delete(radix *set, const char *key);
void radix_clear(radix *set);

/* has_prefix(...) reports whether any stored key starts with prefix. */
int radix_has_prefix(const radix *set, const char *prefix);

#endif
