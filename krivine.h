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

// ----------------------------------------------------------------------------
//
//      A ::= n | y.A | A A                     terms with de Bruijn indices
//      C ::= A[p] | _n_ | \A/                  closures
//      p ::= E | C : p                         environment stack
//      S ::= E | A[p] : S | y : S | \A/ : S    continuation stack
//
// (1)                                 T -> (           T[E],        E, 0     )
// (2)  ( (n + 1)[C : p],         S, l ) -> (           n[p],        S, l     )
// (3)  (       0[C : p],         S, l ) -> (              C,        S, l     )
// (A)  (           n[E],         S, l ) -> (  \n - (0 - l)/,        S, l     )
// (4)  (       (M N)[p],         S, l ) -> (           M[p], N[p] : S, l     )
// (5)  (       (y.B)[p], N[p'] : S, l ) -> (   B[N[p'] : p],        S, l     )
// (6)  (       (y.B)[p],         S, l ) -> ( B[_l + 1_ : p],    y : S, l + 1 )
// (7)  (            _n_,         S, l ) -> (        \l - n/,        S, l     )
// (8)  (            \M/,  N[p] : S, l ) -> (           N[p],  \M/ : S, l     )
// (9)  (            \B/,     y : S, l ) -> (          \y.B/,        S, l - 1 )
// (10) (            \N/,   \M/ : S, l ) -> (          \M N/,        S, l     )
// (11) (            \T/,         E, l ) -> T
//
// ----------------------------------------------------------------------------
// Fig. 6. An improved version of KN that works with closures with open terms
//
//   from  GARCIA-PEREZ, A, & NOGUEIRA, P. (2019)
//         The full-reducing Krivine abstract machine KN
//         simulates pure normal-order reduction in
//         lockstep: A proof via corresponding calculus
//         Journal of Functional Programming, 29, E7.
//         doi:10.1017/S0956796819000017
//

#ifndef KRIVINE_H
#define KRIVINE_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------- 
//
typedef struct {
    int* b;
    int c;
    int s;
} kn_buffer;

// ---------------------------------------------------------------------------- 
// A ::= n | y.A | A A
//
typedef struct {
    kn_buffer* b;
    int i;
} kn_term;

// ---------------------------------------------------------------------------- 
//
static kn_term kn_buffer_add(kn_buffer* b, int i) {
    if (b->c >= b->s) {
        b->s += b->s ? b->s : 256;
        b->b = (int*) realloc(b->b, b->s * sizeof(int));
    }

    b->b[b->c] = i;
    b->c++;

    kn_term t = { b, b->c - 1 };
    return t;
}

// ---------------------------------------------------------------------------- 
// n
//                       t.i
//                        v
//      ==========================
//      |    |    |    |  3 |    |
//      ==========================
//
typedef struct {
    int n;
} kn_index;

static bool kn_is_index(kn_term t) {
    return t.b->b[t.i] >= 0;
}

static kn_index kn_to_index(kn_term t) {
    kn_index i = { t.b->b[t.i] };
    return i;
}

static kn_term kn_add_index(kn_buffer* b, int n) {
    for (int i = 0; i < b->c && i < 32; i++) {
        if (b->b[b->c - i - 1] != n) continue;
        kn_term t = { b, b->c - i - 1 };
        return t;
    }

    return kn_buffer_add(b, n);
}

// ---------------------------------------------------------------------------- 
// y.A
//               abs.b.i t.i
//                   v    v
//      ==========================
//      |    |    |  2 | -1 |    |
//      ==========================
//                   ^
//                 t.i-1
//
typedef struct {
    kn_term b;
} kn_abstraction;

static bool kn_is_abstraction(kn_term t) {
    return t.b->b[t.i] < 0 && t.b->b[t.i - 1] >= 0;
}

static kn_abstraction kn_to_abstraction(kn_term t) {
    t.i += t.b->b[t.i];
    kn_abstraction a = { t };
    return a;
}

static kn_term kn_add_abstraction(kn_buffer* buf, kn_term b) {
    if (buf->b[buf->c - 1] < 0 || !buf->c) kn_buffer_add(buf, 0);
    return kn_buffer_add(buf, b.i - buf->c);
}

// ---------------------------------------------------------------------------- 
// A A
//           app.n.i     t.i
//              v         v
//      ==========================
//      |    |    | -2 | -2 |    |
//      ==========================
//         ^         ^
//      app.m.i    t.i-1
//
typedef struct {
    kn_term m;
    kn_term n;
} kn_application;

static bool kn_is_application(kn_term t) {
    return t.b->b[t.i] < 0 && t.b->b[t.i - 1]  < 0;
}

static kn_application kn_to_application(kn_term t) {
    kn_application app = { t, t };
    app.m.i += t.b->b[t.i - 1] - 1;
    app.n.i += t.b->b[t.i];
    return app;
}

static kn_term kn_add_application(kn_buffer* b, kn_term m, kn_term n) {
    kn_buffer_add(b, m.i - b->c);
    return kn_buffer_add(b, n.i - b->c);
}

// ---------------------------------------------------------------------------- 
// p
// S
//
typedef struct kn_stack kn_stack;

// ---------------------------------------------------------------------------- 
// A[p]
//
typedef struct {
    kn_term t;
    kn_stack* p;
} kn_value;

// ---------------------------------------------------------------------------- 
// _n_
//
typedef struct {
    kn_buffer* b;
    int n;
} kn_level;

// ---------------------------------------------------------------------------- 
// \A/
//
typedef struct {
    kn_term t;
} kn_ground_term;

// ---------------------------------------------------------------------------- 
// C ::= A[p] | _n_ | \A/
// y
typedef struct {
    union {
        kn_value v;       // type == 0
        kn_level l;       // type == 1
        kn_ground_term g; // type == 2
        // 'y' control char  type == 3
    };
    char type;
} kn_closure;

// ---------------------------------------------------------------------------- 
// A[p]
static bool kn_is_value(kn_closure c) {
    return c.type == 0;
}

static kn_value kn_to_value(kn_closure c) {
    return c.v;
}

static kn_closure kn_from_value(kn_term t, kn_stack* p) {
    kn_closure c;
    c.type = 0;
    c.v = (kn_value) { t, p };
    return c;
}

// ---------------------------------------------------------------------------- 
// _n_
//
static bool kn_is_level(kn_closure c) {
    return c.type == 1;
}

static kn_level kn_to_level(kn_closure c) {
    return c.l;
}

static kn_closure kn_from_level(kn_buffer* b, int n) {
    kn_closure c;
    c.type = 1;
    c.l = (kn_level) { b, n };
    return c;
}

// ---------------------------------------------------------------------------- 
// \A/
//
static bool kn_is_ground_term(kn_closure c) {
    return c.type == 2;
}

static kn_ground_term kn_to_ground_term(kn_closure c) {
    return c.g;
}

static kn_closure kn_from_ground_term(kn_term t) {
    kn_closure c;
    c.type = 2;
    c.g = (kn_ground_term) { t };
    return c;
}

// ---------------------------------------------------------------------------- 
// y
//
static bool kn_is_control(kn_closure c) {
    return c.type == 3;
}

static kn_closure kn_from_control() {
    kn_closure c;
    c.type = 3;
    return c;
}

// ---------------------------------------------------------------------------- 
// p ::= E | C : p
// S ::= E | A[p] : S | y : S | \A/ : S
//
struct kn_stack {
    kn_closure c;
    kn_stack* p;
};

// ----------------------------------------------------------------------------
//
static kn_term kn_step(kn_closure clo, kn_stack* s, int l) {
    if (kn_is_level(clo)) {
// ----------------------------------------------------------------------------
// (7)  (            _n_,         S, l ) -> (        \l - n/,        S, l     )
//
        kn_level lev = kn_to_level(clo);
        kn_term t = kn_add_index(lev.b, l - lev.n);
        kn_closure g = kn_from_ground_term(t);
        return kn_step(g, s, l);
    } else if (kn_is_ground_term(clo)) {
        kn_ground_term gro = kn_to_ground_term(clo);
        if (s == NULL) {
// ----------------------------------------------------------------------------
// (11) (            \T/,         E, l ) -> T
//
            return gro.t;
        } else if (kn_is_control(s->c)) {
// ----------------------------------------------------------------------------
// (9)  (            \B/,     y : S, l ) -> (          \y.B/,        S, l - 1 )
//
            kn_term yb = kn_add_abstraction(gro.t.b, gro.t);
            kn_closure ybg = kn_from_ground_term(yb);
            return kn_step(ybg, s->p, l - 1);
        } else if (kn_is_ground_term(s->c)) {
// ----------------------------------------------------------------------------
// (10) (            \N/,   \M/ : S, l ) -> (          \M N/,        S, l     )
//
            kn_ground_term svg = kn_to_ground_term(s->c);
            kn_term n = gro.t;
            kn_term m = svg.t;
            kn_term mn = kn_add_application(n.b, m, n);
            kn_closure mng = kn_from_ground_term(mn);
            return kn_step(mng, s->p, l);
        } else if (kn_is_value(s->c)) {
// ----------------------------------------------------------------------------
// (8)  (            \M/,  N[p] : S, l ) -> (           N[p],  \M/ : S, l     )
//
            kn_stack ms = { clo, s->p };
            return kn_step(s->c, &ms, l);
        }
    } else if (kn_is_value(clo)) {
        kn_value val = kn_to_value(clo);
        if (kn_is_application(val.t)){
// ----------------------------------------------------------------------------
// (4)  (       (M N)[p],         S, l ) -> (           M[p], N[p] : S, l     )
//
            kn_application app = kn_to_application(val.t);
            kn_closure mp = kn_from_value(app.m, val.p);
            kn_closure np = kn_from_value(app.n, val.p);
            kn_stack e = { np, s };
            return kn_step(mp, &e, l);
        } else if (kn_is_abstraction(val.t)) {
            kn_abstraction abs = kn_to_abstraction(val.t);
            if (s && kn_is_value(s->c)) {
// ----------------------------------------------------------------------------
// (5)  (       (y.B)[p], N[p'] : S, l ) -> (   B[N[p'] : p],        S, l     )
//
                kn_stack npp = { s->c, val.p };
                kn_closure bnpp = kn_from_value(abs.b, &npp);
                return kn_step(bnpp, s->p, l);
            } else {
// ----------------------------------------------------------------------------
// (6)  (       (y.B)[p],         S, l ) -> ( B[_l + 1_ : p],    y : S, l + 1 )
//
                kn_closure l1 = kn_from_level(abs.b.b, l + 1);
                kn_stack l1p = { l1, val.p };
                kn_closure bl1p = kn_from_value( abs.b, &l1p );
                kn_closure y = kn_from_control();
                kn_stack ys = { y, s };
                return kn_step(bl1p, &ys, l + 1);
            }
        } else if (kn_is_index(val.t)) {
            kn_index ind = kn_to_index(val.t);
            if (val.p == NULL) {
// ----------------------------------------------------------------------------
// (A)  (           n[E],         S, l ) -> (  \n - (0 - l)/,        S, l     )
//
                kn_term n0l = kn_add_index(val.t.b, ind.n - (0 - l));
                kn_closure n01g = kn_from_ground_term(n0l);
                return kn_step(n01g, s, l);
            } else if (ind.n == 0) {
// ----------------------------------------------------------------------------
// (3)  (       0[C : p],         S, l ) -> (              C,        S, l     )
//
                return kn_step(val.p->c, s, l);
            } else {
// ----------------------------------------------------------------------------
// (2)  ( (n + 1)[C : p],         S, l ) -> (           n[p],        S, l     )
//
                kn_term n = kn_add_index(val.t.b, ind.n - 1);
                kn_closure np = kn_from_value(n, val.p->p);
                return kn_step(np, s, l);
            }
        }
    }
    assert(false);
}

static kn_term kn_eval(kn_term t) {
// ----------------------------------------------------------------------------
// (1)                                 T -> (           T[E],        E, 0     )
//
    kn_closure c = kn_from_value(t, NULL);
    return kn_step(c, NULL, 0);
}

#endif // KRIVINE_H
