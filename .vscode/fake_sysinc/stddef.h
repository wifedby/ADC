#ifndef _FAKE_STDDEF_H_
#define _FAKE_STDDEF_H_

#include <stdint.h>

typedef unsigned long size_t;
typedef signed long   ptrdiff_t;
typedef signed long   intptr_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define offsetof(s, m) ((size_t) &(((s *)0)->m))

#endif /* _FAKE_STDDEF_H_ */