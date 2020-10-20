/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2014 Sylwester Wysocki <sw143@wp.pl>                         */
/*                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */
/*                                                                            */
/******************************************************************************/

#ifndef Debug_H
#define Debug_H

#include <cstdio>

#define DEBUG

#ifdef DEBUG
# define DBG_MSG(...) fprintf(stderr, __VA_ARGS__)
#else
# define DBG_MSG(...) fprintf(stderr, __VA_ARGS__)
#endif

#define DBG_ENTER(X) DBG_MSG("-> " X "()...\n")
#define DBG_LEAVE(X) DBG_MSG("<- " X "()...\n")

#define FAIL(X) if (X) goto fail

#define FAILEX(X, ...) if (X) {fprintf(stderr, __VA_ARGS__); goto fail;}

#endif /* Debug_H */
