/*
 * Simple single threaded Malloc benchmark test based on the more complex one @
 * https://github.com/emeryberger/Malloc-Implementations/blob/master/allocators/ptmalloc/ptmalloc3/t-test1.c
 * by Wolfram Gloger 1996-1999, 2001, 2004, 2006
 *
 *
 * Adrian Sutherland
 */

#include <stdlib.h>
#include <stdio.h>

#define LOOPS        10
#define BINS        100
#define SIZE        10000
#define ACTIONS_MAX    1000
#define TEST    3

#define RANDOM(s)    (rand() % (s))
#define EXIT(c) exit(c)

struct bin {
    unsigned char *ptr;
    unsigned long size;
};

#if TEST > 0

static void
mem_init(unsigned char *ptr, unsigned long size) {
    unsigned long i, j;

    if (size == 0) return;
    for (i = 0; i < size; i += 2047) {
        j = (unsigned long) ptr ^ i;
        ptr[i] = ((j ^ (j >> 8)) & 0xFF);
    }
    j = (unsigned long) ptr ^ (size - 1);
    ptr[size - 1] = ((j ^ (j >> 8)) & 0xFF);
}

static int
mem_check(unsigned char *ptr, unsigned long size) {
    unsigned long i, j;

    if (size == 0) return 0;
    for (i = 0; i < size; i += 2047) {
        j = (unsigned long) ptr ^ i;
        if (ptr[i] != ((j ^ (j >> 8)) & 0xFF)) return 1;
    }
    j = (unsigned long) ptr ^ (size - 1);
    if (ptr[size - 1] != ((j ^ (j >> 8)) & 0xFF)) return 2;
    return 0;
}

#endif /* TEST > 0 */

/* Allocate a bin with malloc() */
static void
bin_alloc(struct bin *m, unsigned long size) {
#if TEST > 0
    if (mem_check(m->ptr, m->size)) {
        printf("memory corrupt!\n");
        EXIT(1);
    }
#endif
    if (m->size > 0) free(m->ptr);
    m->ptr = (unsigned char *) malloc(size);

    if (!m->ptr) {
        printf("out of memory (SIZE=%ld)!\n", (long) size);
        EXIT(1);
    }
    m->size = size;
#if TEST > 0
    mem_init(m->ptr, m->size);
#endif
}

/* Free a bin. */
static void
bin_free(struct bin *m) {
    if (m->size == 0) return;
#if TEST > 0
    if (mem_check(m->ptr, m->size)) {
        printf("memory corrupt!\n");
        EXIT(1);
    }
#endif
    free(m->ptr);
    m->size = 0;
}

void
malloc_test() {
    int b, j, actions;
    struct bin *m;

    m = (struct bin *) malloc(BINS * sizeof(struct bin));
    if (!m) {
        printf("failed to allocate bin!\n");
        EXIT(1);
    }

#if TEST > 2
    int i = 0;
#endif
    for (b = 0; b < BINS; b++) {
        m[b].size = 0;
        m[b].ptr = NULL;
        if (RANDOM(2) == 0) {
            bin_alloc(&m[b], RANDOM(SIZE) + 1);
#if TEST > 2
            i++;
#endif
        }
    }
#if TEST > 2
    printf("%d allocate out of %d\n", i, BINS);
#endif

#if TEST > 1
    for (b = 0; b < BINS; b++) {
        if (mem_check(m[b].ptr, m[b].size)) {
            printf("memory corrupt!\n");
            EXIT(1);
        }
    }
#endif

    actions = RANDOM(ACTIONS_MAX);
    for (j = 0; j < actions; j++) {
        b = RANDOM(BINS);
        bin_free(&m[b]);
    }

    actions = RANDOM(ACTIONS_MAX);
    for (j = 0; j < actions; j++) {
        b = RANDOM(BINS);
        bin_alloc(&m[b], RANDOM(SIZE) + 1);
#if TEST > 2
        for (b = 0; b < BINS; b++) {
            if (mem_check(m[b].ptr, m[b].size)) {
                printf("memory corrupt!\n");
                EXIT(1);
            }
        }
#endif
    }

    for (b = 0; b < BINS; b++)
        bin_free(&m[b]);
    free(m);
}

int
main(int argc, char *argv[]) {
    int i;

    for (i = 0; i < LOOPS; i++) {
        printf("Starting Loop %d\n", i);
        malloc_test();
    }

    printf("Done.\n");
    return 0;
}
