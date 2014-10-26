/*===- GCDAProfiling.c - Support library for GCDA file emission -----------===*\
|*
|*                     The LLVM Compiler Infrastructure
|*
|* This file is distributed under the University of Illinois Open Source
|* License. See LICENSE.TXT for details.
|*
|*===----------------------------------------------------------------------===*|
|*
|* This file implements the call back routines for the gcov profiling
|* instrumentation pass. Link against this library when running code through
|* the -insert-gcov-profiling LLVM pass.
|*
|* We emit files in a corrupt version of GCOV's "gcda" file format. These files
|* are only close enough that LCOV will happily parse them. Anything that lcov
|* ignores is missing.
|*
|* TODO: gcov is multi-process safe by having each exit open the existing file
|* and append to it. We'd like to achieve that and be thread-safe too.
|*
\*===----------------------------------------------------------------------===*/

#include <stdio.h>
#include <stdlib.h>
#include "../libsaio/io_inline.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

static void write_bytes(const char* data, uint32_t len);

#define COMMAND_OPEN_FILE       'o'
#define COMMAND_CLOSE_FILE      'c'
#define COMMAND_EMIT_ARCS       'a'
#define COMMAND_EMIT_FUNCS      'f'

typedef struct __attribute__((packed)) serial_command {
    uint8_t command;
    uint32_t length;
} serial_command_t;


/*** Local versions of std lib functions to ensure we don't prifile recursivly... ***/
#define UCHAR_MAX 255
#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

static size_t strlen(const char *s)
{
    const char *a = s;
    const size_t *w;
    for (; (uintptr_t)s % ALIGN; s++) if (!*s) return s-a;
    for (w = (const void *)s; !HASZERO(*w); w++);
    for (s = (const void *)w; *s; s++);
    return s-a;
}

/* #define DEBUG_GCDAPROFILING */

/*
 * --- LLVM line counter API ---
 */

/* A file in this case is a translation unit. Each .o file built with line
 * profiling enabled will emit to a different file. Only one file may be
 * started at a time.
 */
void llvm_gcda_start_file(const char *orig_filename) {
    serial_command_t command;
    command.command = COMMAND_OPEN_FILE;
    command.length = strlen(orig_filename);

    // write command.
    write_bytes((void*)&command, sizeof(command));
    // write orig_filename
    write_bytes(orig_filename, command.length);
}

/* Given an array of pointers to counters (counters), increment the n-th one,
 * where we're also given a pointer to n (predecessor).
 */
void llvm_gcda_increment_indirect_counter(uint32_t *predecessor,
                                          uint64_t **counters) {
  uint64_t *counter;
  uint32_t pred;

  pred = *predecessor;
  if (pred == 0xffffffff)
    return;
  counter = counters[pred];

  /* Don't crash if the pred# is out of sync. This can happen due to threads,
     or because of a TODO in GCOVProfiling.cpp buildEdgeLookupTable(). */
  if (counter)
    ++*counter;
}

void llvm_gcda_emit_function(uint32_t ident, const char *function_name) {
    serial_command_t command;
    command.command = COMMAND_EMIT_FUNCS;
    command.length = sizeof(ident) + strlen(function_name);

    // write command.
    write_bytes((void*)&command, sizeof(command));
    write_bytes((void*)&ident, sizeof(ident));
    write_bytes((void*)function_name, strlen(function_name));
}

void llvm_gcda_emit_arcs(uint32_t num_counters, uint64_t *counters) {
    serial_command_t command;
    command.command = COMMAND_EMIT_ARCS;
    command.length = num_counters * sizeof(uint64_t);

    // write command.
    write_bytes((void*)&command, sizeof(command));
    // write counters
    write_bytes((void*)counters, command.length);
}

void llvm_gcda_end_file() {
    serial_command_t command;
    command.command = COMMAND_CLOSE_FILE;
    command.length = 0;

    // write command.
    write_bytes((void*)&command, sizeof(command));
}




/***** Serial Port Routines *****/
// FIXME: Serial port init must also be done.
#define PORT 0x3f8   /* COM1 */

static int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

static void write_serial(char a) {
   while (is_transmit_empty() == 0);

   outb(PORT,a);
}

static void write_bytes(const char* data, uint32_t len)
{
    while(len--) write_serial(*data++);
}
