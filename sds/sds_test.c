#include "sds.h"
#include "../testhelp.h"

#include <string.h>

void sds_test(void) {
    sds x = sdsnew(""), y;
    test_cond("Create an empty string",
        sdslen(x) == 0 && memcmp(x, "\0", 1) == 0)

    sdsfree(x);
    x = sdsnew("foo");
    test_cond("Create a string and obtain the length",
        sdslen(x) == 3 && memcmp(x, "foo\0", 4) == 0)

    sdsfree(x);
    x = sdsnewfmt("%d", 1234567890);
    test_cond("sdsnewfmt() seems working in the base case",
        sdslen(x) == 10 && memcmp(x, "1234567890\0", 11) == 0)

    sdsfree(x);
    x = sdsnewlen("foo", 2);
    test_cond("Create a string with specified length",
        sdslen(x) == 2 && memcmp(x, "fo\0", 3) == 0)

    x = sdscat(x, "bar");
    test_cond("Strings concatenation",
        sdslen(x) == 5 && memcmp(x, "fobar\0", 6) == 0);

    sdsfree(x);
    x = sdsnew(" x ");
    sdstrim(x, " x");
    test_cond("sdstrim() works when all chars match",
        sdslen(x) == 0)

    sdsfree(x);
    x = sdsnew(" x ");
    sdstrim(x, " ");
    test_cond("sdstrim() works when a single char remains",
        sdslen(x) == 1 && x[0] == 'x')

    sdsfree(x);
    x = sdsnew("xxciaoyyy");
    sdstrim(x, "xy");
    test_cond("sdstrim() correctly trims characters",
        sdslen(x) == 4 && memcmp(x, "ciao\0", 5) == 0)

    sdsfree(x);
    x = sdsnew("foo");
    y = sdsnew("foa");
    test_cond("sdscmp(foo, foa)", sdscmp(x, y) > 0)

    sdsfree(y);
    sdsfree(x);
    x = sdsnew("bar");
    y = sdsnew("bar");
    test_cond("sdscmp(bar, bar)", sdscmp(x, y) == 0)

    sdsfree(y);
    sdsfree(x);
    x = sdsnew("bar");
    y = sdsnew("bara");
    test_cond("sdscmp(bar, bara)", sdscmp(x, y) < 0)

    sdsfree(x);
    x = sdsnew("hellowhat");
    x = sdsreplace(x, "what", "world");
    test_cond("sdsreplace() seems working in the base case",
        sdslen(x) == 10 && memcmp(x, "helloworld\0", 11) == 0)

    sdsfree(y);
    sdsfree(x);
    test_report()
}

int main(void) {
    sds_test();
}
