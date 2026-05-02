#include "sds.h"
#include "testhelp.h"

#include <stdlib.h>
#include <string.h>

void sds_test(void) {
    sds x = sdsnew(""), y;
    test_cond("Create an empty string",
        sdslen(x) == 0 && memcmp(x, "\0", 1) == 0);

    sdsfree(x);
    x = sdsnew("foo");
    test_cond("Create a string and obtain the length",
        sdslen(x) == 3 && memcmp(x, "foo\0", 4) == 0);
    test_cond("sds capacity tracks used length",
        sdsalloc(x) == 3 && sdsavail(x) == 0);

    sdsfree(x);
    x = sdsnewfmt("%d", 1234567890);
    test_cond("sdsnewfmt() seems working in the base case",
        sdslen(x) == 10 && memcmp(x, "1234567890\0", 11) == 0);

    sdsfree(x);
    x = sdsnewlen("foo", 2);
    test_cond("Create a string with specified length",
        sdslen(x) == 2 && memcmp(x, "fo\0", 3) == 0);

    sdsfree(x);
    x = sdsncat(NULL, "foobar", 3);
    test_cond("sdsncat() creates a string from NULL",
        sdslen(x) == 3 && memcmp(x, "foo\0", 4) == 0);

    y = sdsdup(x);
    test_cond("sdsdup() copies string contents",
        y != NULL && y != x && sdslen(y) == 3 && memcmp(y, "foo\0", 4) == 0);
    sdsfree(y);
    y = NULL;

    x = sdscat(x, "bar");
    test_cond("Strings concatenation",
        sdslen(x) == 6 && memcmp(x, "foobar\0", 7) == 0);

    x = sdsreserve(x, 32);
    test_cond("sdsreserve() grows capacity",
        x != NULL && sdsalloc(x) >= 32 && sdslen(x) == 6);
    sdsclear(x);
    test_cond("sdsclear() resets logical length",
        sdslen(x) == 0 && memcmp(x, "\0", 1) == 0);

    sdsfree(x);
    x = sdsnew(" x ");
    sdstrim(x, " x");
    test_cond("sdstrim() works when all chars match",
        sdslen(x) == 0);

    sdsfree(x);
    x = sdsnew(" x ");
    sdstrim(x, " ");
    test_cond("sdstrim() works when a single char remains",
        sdslen(x) == 1 && x[0] == 'x');

    sdsfree(x);
    x = sdsnew("xxciaoyyy");
    sdstrim(x, "xy");
    test_cond("sdstrim() correctly trims characters",
        sdslen(x) == 4 && memcmp(x, "ciao\0", 5) == 0);

    sdsfree(x);
    x = sdsnew("");
    sdstrim(x, " ");
    test_cond("sdstrim() keeps empty strings stable",
        sdslen(x) == 0 && memcmp(x, "\0", 1) == 0);

    sdsfree(x);
    x = sdsnew("foo");
    y = sdsnew("foa");
    test_cond("sdscmp(foo, foa)", sdscmp(x, y) > 0);

    sdsfree(y);
    sdsfree(x);
    x = sdsnew("bar");
    y = sdsnew("bar");
    test_cond("sdscmp(bar, bar)", sdscmp(x, y) == 0);

    sdsfree(y);
    sdsfree(x);
    x = sdsnew("bar");
    y = sdsnew("bara");
    test_cond("sdscmp(bar, bara)", sdscmp(x, y) < 0);

    sdsfree(x);
    x = sdsnew("hellowhat");
    x = sdsreplace(x, "what", "world");
    test_cond("sdsreplace() seems working in the base case",
        sdslen(x) == 10 && memcmp(x, "helloworld\0", 11) == 0);

    x = sdsreplace(x, "world", "earth");
    test_cond("sdsreplace() works with same-length replacement",
        sdslen(x) == 10 && memcmp(x, "helloearth\0", 11) == 0);

    x = sdsreplace(x, "earth", "go");
    test_cond("sdsreplace() works with shorter replacement",
        sdslen(x) == 7 && memcmp(x, "hellogo\0", 8) == 0);

    x = sdsreplace(x, "go", "worldwide");
    test_cond("sdsreplace() works with longer replacement",
        sdslen(x) == 14 && memcmp(x, "helloworldwide\0", 15) == 0);

    y = x;
    x = sdsreplace(x, "missing", "noop");
    test_cond("sdsreplace() keeps string when no match exists",
        x == y && sdslen(x) == 14 && memcmp(x, "helloworldwide\0", 15) == 0);

    y = x;
    x = sdsreplace(x, "", "noop");
    test_cond("sdsreplace() ignores empty replace pattern",
        x == y && sdslen(x) == 14 && memcmp(x, "helloworldwide\0", 15) == 0);

    sdsfree(x);
    y = NULL;
    x = sdsnewfmt("%s:%d", "value", 12345);
    test_cond("sdsnewfmt() grows for longer formatted strings",
        sdslen(x) == 11 && memcmp(x, "value:12345\0", 12) == 0);

    sdsfree(x);
    size_t count = 0;
    sds *parts = calloc(2, sizeof(*parts));
    x = sdsnew("a-b-c");
    y = sdsnew("tail");
    test_cond("sdsjoin() init parts", parts != NULL);
    parts[0] = x;
    parts[1] = y;
    sds joined = sdsjoin(parts, 2, "::");
    test_cond("sdsjoin() joins array elements",
        joined != NULL && sdslen(joined) == 11 && memcmp(joined, "a-b-c::tail\0", 12) == 0);
    sdsfree(joined);
    sdsfree(y);
    sdsfree(x);
    y = NULL;
    x = NULL;
    free(parts);

    joined = sdsjoin(NULL, 0, ",");
    test_cond("sdsjoin() returns empty string for empty input",
        joined != NULL && sdslen(joined) == 0 && memcmp(joined, "\0", 1) == 0);
    sdsfree(joined);

    parts = sdssplit("alpha,beta,,gamma", ",", &count);
    test_cond("sdssplit() creates an array", parts != NULL);
    test_cond("sdssplit() keeps empty tokens", parts != NULL && count == 4);
    test_cond("sdssplit() token 0",
        parts != NULL && strcmp(parts[0], "alpha") == 0);
    test_cond("sdssplit() token 1",
        parts != NULL && strcmp(parts[1], "beta") == 0);
    test_cond("sdssplit() token 2",
        parts != NULL && strcmp(parts[2], "") == 0);
    test_cond("sdssplit() token 3",
        parts != NULL && strcmp(parts[3], "gamma") == 0);
    sdssplitfree(parts, count);

    parts = sdssplit("", ",", &count);
    test_cond("sdssplit() keeps empty input as one token",
        parts != NULL && count == 1 && strcmp(parts[0], "") == 0);
    sdssplitfree(parts, count);

    parts = sdssplit("abc", "", &count);
    test_cond("sdssplit() rejects empty separator", parts == NULL);

    {
        const char raw[] = {'a', '\0', 'b', '\0', 'c'};
        const char sep_raw[] = {'\0', 'b'};
        const char rep_raw[] = {'\0', 'b'};
        const char with_raw[] = {'X', 'Y', 'Z'};
        const char expected_raw[] = {'a', 'X', 'Y', 'Z', '\0', 'c'};
        sds raw_s = sdsnewlen(raw, sizeof(raw));
        sds binary_a = sdsnewlen(raw, 2);
        sds binary_b = sdsnewlen(raw + 2, 3);
        sds join_parts[2];
        sds binary_joined;
        sds replaced;
        sds *binary_parts;
        size_t binary_count = 0;

        test_cond("binary sds init", raw_s != NULL && binary_a != NULL && binary_b != NULL);
        join_parts[0] = binary_a;
        join_parts[1] = binary_b;

        binary_joined = sdsjoinlen(join_parts, 2, sep_raw, sizeof(sep_raw));
        test_cond("sdsjoinlen() keeps embedded nul bytes",
            binary_joined != NULL &&
            sdslen(binary_joined) == 7 &&
            memcmp(binary_joined, (char[]){'a','\0','\0','b','b','\0','c'}, 7) == 0);

        replaced = sdsreplacelen(raw_s, rep_raw, sizeof(rep_raw), with_raw, sizeof(with_raw));
        test_cond("sdsreplacelen() replaces binary subsequences",
            replaced != NULL &&
            sdslen(replaced) == sizeof(expected_raw) &&
            memcmp(replaced, expected_raw, sizeof(expected_raw)) == 0);

        binary_parts = sdssplitlen(raw, sizeof(raw), sep_raw, sizeof(sep_raw), &binary_count);
        test_cond("sdssplitlen() splits binary payload",
            binary_parts != NULL && binary_count == 2 &&
            sdslen(binary_parts[0]) == 1 && memcmp(binary_parts[0], "a", 1) == 0 &&
            sdslen(binary_parts[1]) == 2 && memcmp(binary_parts[1], "\0c", 2) == 0);

        sdssplitfree(binary_parts, binary_count);
        sdsfree(replaced);
        sdsfree(binary_joined);
        sdsfree(binary_b);
        sdsfree(binary_a);
    }

    sdsfree(y);
    sdsfree(x);
    test_report();
}

int main(void) {
    sds_test();
}
