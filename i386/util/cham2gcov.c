/*
 * Copyright (c) 2014 xZenue LLC. All Rights Reserved.
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define COMMAND_OPEN_FILE       'o'
#define COMMAND_CLOSE_FILE      'c'
#define COMMAND_EMIT_ARCS       'a'
#define COMMAND_EMIT_FUNCS      'f'

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long  uint64_t;
typedef signed char	    	sint8_t;
typedef signed short		sint16_t;
typedef signed int	     	sint32_t;
typedef signed long long    sint64_t;

static char *mangle_filename(const char *orig_filename);
static void recursive_mkdir(char *filename);
static uint64_t read_into_buffer(uint64_t *buffer, size_t size);
static uint32_t read_int32();
static void write_int32(uint32_t i);
static uint64_t write_from_buffer(uint64_t *buffer, size_t size);
static uint32_t length_of_string(const char *s);
static void write_string(const char *s);

void llvm_gcda_start_file(const char *orig_filename, const char version[4]);
void llvm_gcda_end_file();
void llvm_gcda_emit_arcs(uint32_t num_counters, uint64_t *counters);
void llvm_gcda_emit_function(uint32_t ident, const char *function_name,
                             uint8_t use_extra_checksum);

typedef struct __attribute__((packed)) cov_command {
    uint8_t command;
    uint32_t length;
} cov_command_t;

/*
 * The current file we're outputting.
 */
static FILE *output_file = NULL;


int help(int argc, const char* argv[]);
void handle_command(uint8_t command, void* data, uint32_t len);

int main(int argc, const char* argv[])
{
    cov_command_t command;
	FILE* chamcov;

	if(argc < 2)
	{
		exit(help(argc, argv));
	}

    chamcov = fopen(argv[1], "r");
    if(!chamcov)
    {
        printf("ERROR: unable to open input file %s with error '%s'\n", argv[1], strerror(errno));
        exit(errno);
    }

    while(fread(&command, sizeof(command), 1, chamcov))
    {
        void* data = command.length ? malloc(command.length + 1) : NULL;

        if(command.length && !fread(data, command.length, 1, chamcov))
        {
            printf("Unable read in command data.\n");
        }
        else if(command.length)
        {
            // Add null terminator in case of string.
            ((char*)data)[command.length] = 0;
        }

        handle_command(command.command, data, command.length);

        free(data);
    }

	return 0;
}

void handle_command(uint8_t command, void* data, uint32_t len)
{
    switch(command)
    {
        case COMMAND_OPEN_FILE:
        {
            char* filename = data;
            printf("Start file %s\n", filename);
            llvm_gcda_start_file(filename, "*704");
            break;
        }
        case COMMAND_CLOSE_FILE:
        {
            printf("Close file\n");

            llvm_gcda_end_file();
            break;
        }
        case COMMAND_EMIT_ARCS:
        {
            printf("Emit Arcs\n");
            uint32_t num_counters = len / sizeof(uint64_t);
            uint64_t *counters = data;
            llvm_gcda_emit_arcs(num_counters, counters);
            break;
        }
        case COMMAND_EMIT_FUNCS:
        {
            uint32_t ident = ((uint32_t*)data)[0];
            char* function_name = (data + sizeof(ident));
            printf("Emit function %s\n", function_name);

            //llvm_gcda_emit_function(ident, function_name, 0);
            llvm_gcda_emit_function(ident, NULL, 1); // gcov compat
            break;
        }
    }
}


int help(int argc, const char* argv[])
{
	printf("%s filename\n", argv[0]);
	printf("	filename	The input coverage information file.\n");
	return -1;
}


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

void llvm_gcda_start_file(const char *orig_filename, const char version[4]) {
  char *filename = mangle_filename(orig_filename);

  /* Try just opening the file. */
  output_file = fopen(filename, "r+b");

  if (!output_file) {
    /* Try opening the file, creating it if necessary. */
    output_file = fopen(filename, "w+b");
    if (!output_file) {
      /* Try creating the directories first then opening the file. */
      recursive_mkdir(filename);
      output_file = fopen(filename, "w+b");
      if (!output_file) {
        /* Bah! It's hopeless. */
        fprintf(stderr, "profiling:%s: cannot open\n", filename);
        free(filename);
        return;
      }
    }
  }

  /* gcda file, version, stamp LLVM. */
  fwrite("adcg", 4, 1, output_file);
  fwrite(version, 4, 1, output_file);
  fwrite("MVLL", 4, 1, output_file);
  free(filename);

#ifdef DEBUG_GCDAPROFILING
  fprintf(stderr, "llvmgcda: [%s]\n", orig_filename);
#endif
}

void llvm_gcda_end_file() {
  /* Write out EOF record. */
  if (!output_file) return;
  fwrite("\0\0\0\0\0\0\0\0", 8, 1, output_file);
  fclose(output_file);
  output_file = NULL;

#ifdef DEBUG_GCDAPROFILING
  fprintf(stderr, "llvmgcda: -----\n");
#endif
}

void llvm_gcda_emit_arcs(uint32_t num_counters, uint64_t *counters) {
  uint32_t i;
  uint64_t *old_ctrs = NULL;
  uint32_t val = 0;
  long pos = 0;

  if (!output_file) return;

  pos = ftell(output_file);
  val = read_int32();

  if (val != (uint32_t)-1) {
    /* There are counters present in the file. Merge them. */
    if (val != 0x01a10000) {
      fprintf(stderr, "profiling:invalid magic number (0x%08x)\n", val);
      return;
    }

    val = read_int32();
    if (val == (uint32_t)-1 || val / 2 != num_counters) {
      fprintf(stderr, "profiling:invalid number of counters (%d)\n", val);
      return;
    }

    old_ctrs = malloc(sizeof(uint64_t) * num_counters);

    if (read_into_buffer(old_ctrs, num_counters) == (uint64_t)-1) {
      fprintf(stderr, "profiling:invalid number of counters (%d)\n",
              num_counters);
      return;
    }
  }

  /* Reset for writing. */
  fseek(output_file, pos, SEEK_SET);

  /* Counter #1 (arcs) tag */
  fwrite("\0\0\xa1\1", 4, 1, output_file);
  write_int32(num_counters * 2);
  for (i = 0; i < num_counters; ++i)
    counters[i] += (old_ctrs ? old_ctrs[i] : 0);

  if (write_from_buffer(counters, num_counters) == (uint64_t)-1) {
    fprintf(stderr, "profiling:cannot write to output file\n");
    return;
  }

  free(old_ctrs);

#ifdef DEBUG_GCDAPROFILING
  fprintf(stderr, "llvmgcda:   %u arcs\n", num_counters);
  for (i = 0; i < num_counters; ++i)
    fprintf(stderr, "llvmgcda:   %llu\n", (unsigned long long)counters[i]);
#endif
}

void llvm_gcda_emit_function(uint32_t ident, const char *function_name,
                             uint8_t use_extra_checksum) {
  uint32_t len = 2;
  if (use_extra_checksum)
    len++;
#ifdef DEBUG_GCDAPROFILING
  fprintf(stderr, "llvmgcda: function id=0x%08x name=%s\n", ident,
          function_name ? function_name : "NULL");
#endif
  if (!output_file) return;

  /* function tag */
  fwrite("\0\0\0\1", 4, 1, output_file);
  if (function_name)
    len += 1 + length_of_string(function_name);
  write_int32(len);
  write_int32(ident);
  write_int32(0);
  if (use_extra_checksum)
    write_int32(0);
  if (function_name)
    write_string(function_name);
}



static void recursive_mkdir(char *filename) {
  int i;

  for (i = 1; filename[i] != '\0'; ++i) {
    if (filename[i] != '/') continue;
    filename[i] = '\0';
#ifdef _WIN32
    _mkdir(filename);
#else
    mkdir(filename, 0755);  /* Some of these will fail, ignore it. */
#endif
    filename[i] = '/';
  }
}


static char *mangle_filename(const char *orig_filename) {
  char *filename = 0;
  int prefix_len = 0;
  int prefix_strip = 0;
  int level = 0;
  const char *fname = orig_filename, *ptr = NULL;
  const char *prefix = getenv("GCOV_PREFIX");
  const char *tmp = getenv("GCOV_PREFIX_STRIP");

  if (!prefix)
    return strdup(orig_filename);

  if (tmp) {
    prefix_strip = atoi(tmp);

    /* Negative GCOV_PREFIX_STRIP values are ignored */
    if (prefix_strip < 0)
      prefix_strip = 0;
  }

  prefix_len = strlen(prefix);
  filename = malloc(prefix_len + 1 + strlen(orig_filename) + 1);
  strcpy(filename, prefix);

  if (prefix[prefix_len - 1] != '/')
    strcat(filename, "/");

  for (ptr = fname + 1; *ptr != '\0' && level < prefix_strip; ++ptr) {
    if (*ptr != '/') continue;
    fname = ptr;
    ++level;
  }

  strcat(filename, fname);

  return filename;
}

static uint32_t read_int32() {
  uint32_t tmp;

  if (fread(&tmp, 1, 4, output_file) != 4)
    return (uint32_t)-1;

  return tmp;
}

static uint64_t read_into_buffer(uint64_t *buffer, size_t size) {
  if (fread(buffer, 8, size, output_file) != size)
    return (uint64_t)-1;

  return 0;
}

static void write_int32(uint32_t i) {
  fwrite(&i, 4, 1, output_file);
}

static uint64_t write_from_buffer(uint64_t *buffer, size_t size) {
  if (fwrite(buffer, 8, size, output_file) != size)
    return (uint64_t)-1;

  return 0;
}

static uint32_t length_of_string(const char *s) {
  return (strlen(s) / 4) + 1;
}

static void write_string(const char *s) {
  uint32_t len = length_of_string(s);
  write_int32(len);
  fwrite(s, strlen(s), 1, output_file);
  fwrite("\0\0\0\0", 4 - (strlen(s) % 4), 1, output_file);
}
