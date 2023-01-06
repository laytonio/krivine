// ----------------------------------------------------------------------------
// This software is available under 2 licenses -- choose whichever you prefer.
// ----------------------------------------------------------------------------
// ALTERNATIVE A - MIT License
// Copyright (c) 2023 Layton Kifer
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------
// ALTERNATIVE B - Public Domain (www.unlicense.org)
// This is free and unencumbered software released into the public domain.
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute
// this software, either in source code form or as a compiled binary, for any
// purpose, commercial or non-commercial, and by any means.
// In jurisdictions that recognize copyright laws, the author or authors of
// this software dedicate any and all copyright interest in the software to the
// public domain. We make this dedication for the benefit of the public at
// large and to the detriment of our heirs and successors. We intend this
// dedication to be an overt act of relinquishment in perpetuity of all present
// and future rights to this software under copyright law.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "krivine.h"

void print(kn_term t) {
    if (kn_is_index(t)) {
        printf("%d", kn_to_index(t).n);
    } else if (kn_is_abstraction(t)) {
        putchar('[');
        print(kn_to_abstraction(t).b);
        putchar(']');
    } else {
        kn_application app = kn_to_application(t);
        print(app.m);
        putchar(' ');
        if (kn_is_application(app.n)) {
            putchar('(');
            print(app.n);
            putchar(')');
        } else {
            print(app.n);
        }
    }
}

kn_term parse(kn_buffer* b, char** str){
    kn_term res;
    for (bool first = true; true; first = false) {
        while (**str == ' ') (*str)++;

        kn_term previous = res;
        if (**str >= '0' && **str <= '9') {
            int sym = 0;
            for (; **str >= '0' && **str <= '9'; (*str)++) 
                sym = (sym * 10) + **str - '0';
            res = kn_add_index(b, sym);
        } else if (**str == '[') {
            (*str)++;
            kn_term body = parse(b, str);
            if (**str != ']') {
                (*str)--;
                break;
            }
            (*str)++;
            res = kn_add_abstraction(b, body);
        } else if (**str == '(') {
            (*str)++;
            res = parse(b, str);
            if (**str != ')') {
                (*str)--;
                break;
            }
            (*str)++;
        } else {
            break;
        }

        if (first) continue;
        res = kn_add_application(b, previous, res);
    }
    return res;
}

int main(int argc, char** argv) {
    char line[256];
    kn_buffer b = { NULL, 0, 0 };
    while (true) {
        b.c = 0;
        if (!fgets(line, 256, stdin)) break;
        line[strcspn(line, "\n")] = 0;

        char* ptr = line; 
        kn_term t = parse(&b, &ptr);
        if (*ptr || !b.c) {
            fprintf(stderr, "Parse failed at '%s'\n", ptr);
        } else {
            print(kn_eval(t));
            putchar('\n');
        }
    }
    return 0;
}
