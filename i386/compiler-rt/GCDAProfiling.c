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
extern void register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*));

/*
 * zalloc.c
 */
#define malloc(size) safe_malloc(size, __FILE__, __LINE__)
extern void   malloc_init(char * start, int size, int nodes, void (*malloc_error)(char *, size_t, const char *, int));
extern void * safe_malloc(size_t size,const char *file, int line);
extern void   free(void * start);
extern void * realloc(void * ptr, size_t size);



typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

/*
 * A list of functions to write out the data.
 */
typedef void (*writeout_fn)();

struct writeout_fn_node {
  writeout_fn fn;
  struct writeout_fn_node *next;
};

static struct writeout_fn_node *writeout_fn_head = NULL;
static struct writeout_fn_node *writeout_fn_tail = NULL;

/*
 *  A list of flush functions that our __gcov_flush() function should call.
 */
typedef void (*flush_fn)();

struct flush_fn_node {
  flush_fn fn;
  struct flush_fn_node *next;
};

static struct flush_fn_node *flush_fn_head = NULL;
static struct flush_fn_node *flush_fn_tail = NULL;




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
#define PORT 0x3f8   /* COM1 */

void llvm_register_writeout_function(writeout_fn fn) {
  struct writeout_fn_node *new_node = malloc(sizeof(struct writeout_fn_node));
  new_node->fn = fn;
  new_node->next = NULL;

  if (!writeout_fn_head) {
    writeout_fn_head = writeout_fn_tail = new_node;
  } else {
    writeout_fn_tail->next = new_node;
    writeout_fn_tail = new_node;
  }
}

void llvm_writeout_files() {
  struct writeout_fn_node *curr = writeout_fn_head;

  while (curr) {
    curr->fn();
    curr = curr->next;
  }
}

void llvm_delete_writeout_function_list() {
  while (writeout_fn_head) {
    struct writeout_fn_node *node = writeout_fn_head;
    writeout_fn_head = writeout_fn_head->next;
    free(node);
  }

  writeout_fn_head = writeout_fn_tail = NULL;
}

void llvm_register_flush_function(flush_fn fn) {
  struct flush_fn_node *new_node = malloc(sizeof(struct flush_fn_node));
  new_node->fn = fn;
  new_node->next = NULL;

  if (!flush_fn_head) {
    flush_fn_head = flush_fn_tail = new_node;
  } else {
    flush_fn_tail->next = new_node;
    flush_fn_tail = new_node;
  }
}

void __gcov_flush() {
  struct flush_fn_node *curr = flush_fn_head;

  while (curr) {
    curr->fn();
    curr = curr->next;
  }
}

void llvm_delete_flush_function_list() {
  while (flush_fn_head) {
    struct flush_fn_node *node = flush_fn_head;
    flush_fn_head = flush_fn_head->next;
    free(node);
  }

  flush_fn_head = flush_fn_tail = NULL;
}


void llvm_do_exit(void* arg0, void* arg1, void* arg2, void* arg3)
{
    llvm_writeout_files();
}

void llvm_gcov_init(writeout_fn wfn, flush_fn ffn) {
  static int atexit_ran = 0;

  if (wfn)
    llvm_register_writeout_function(wfn);

  if (ffn)
    llvm_register_flush_function(ffn);

  if (atexit_ran == 0) {
    atexit_ran = 1;

    /* Make sure we write out the data and delete the data structures. */
    //atexit(llvm_delete_flush_function_list);
    //atexit(llvm_delete_writeout_function_list);
    //atexit(llvm_writeout_files);
    register_hook_callback("Exit", &llvm_do_exit);

    // Init serial port.
    outb(PORT + 1, 0x00);    // Disable all interrupts
    outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00);    //                  (hi byte)
    outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
  }
}


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

void llvm_gcda_summary_info()
{
}
