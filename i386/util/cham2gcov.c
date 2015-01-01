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

void llvm_gcda_start_file(const char *orig_filename, const char version[4], uint32_t checksum);
void llvm_gcda_end_file();
void llvm_gcda_emit_arcs(uint32_t num_counters, uint64_t *counters);
void llvm_gcda_emit_function(uint32_t ident,
                             const char *function_name,
                             uint32_t func_checksum,
                             uint8_t use_extra_checksum,
                             uint32_t cfg_checksum);

typedef struct __attribute__((packed)) serial_command {
    uint8_t command;
    uint32_t length;
} serial_command_t;

typedef struct __attribute__((packed)) open_command {
    serial_command_t header;
    const char version[4];
    uint32_t checksum;
    // const char[lenght] filename
} open_command_t;

typedef struct __attribute__((packed)) emit_function_command {
    serial_command_t header;
    uint32_t ident;
    uint32_t func_checksum;
    uint32_t cfg_checksum;
    uint8_t use_extra_checksum;
    // const char[lenght] function_name
} emit_function_command_t;


typedef struct __attribute__((packed)) emit_arcs_command {
    serial_command_t header;
} emit_arcs_command_t;


static int command_to_length(serial_command_t* cmd)
{
    switch(cmd->command)
    {
        case COMMAND_OPEN_FILE:
            return sizeof(open_command_t) + cmd->length;
	    
        case COMMAND_CLOSE_FILE:
            return sizeof(serial_command_t);
	    
        case COMMAND_EMIT_FUNCS:
            return sizeof(emit_function_command_t) + cmd->length;
	    
        case COMMAND_EMIT_ARCS:
            return sizeof(emit_arcs_command_t) + cmd->length;

        default:
            printf("Unknown header cmd '%c'\n", cmd->command);
            exit(-2);
    }
    return sizeof(serial_command_t);
}


/*
 * The current file we're outputting.
 */
static FILE *output_file = NULL;


int help(int argc, const char* argv[]);
void handle_command(uint8_t command, void* data, uint32_t len);

int main(int argc, const char* argv[])
{
    serial_command_t command;
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
    	int length = command_to_length(&command);
        void* data = malloc(length +1);
	memcpy(data, &command, sizeof(command));

        if((length - sizeof(command)) && !fread(data + sizeof(command), length - sizeof(command), 1, chamcov))
        {
            printf("Unable to read in command data (%lu bytes for cmd %c).\n", length - sizeof(command), command.command);
        }
        else
        {
            // Add null terminator in case of string.
            ((char*)data)[length] = 0;
        }

        handle_command(command.command, data, command.length);

        if(data) free(data);
    }

    return 0;
}

void handle_command(uint8_t command, void* data, uint32_t len)
{
    switch(command)
    {
        case COMMAND_OPEN_FILE:
        {
            open_command_t* cmd = data;
            char* filename = data + sizeof(open_command_t);
            printf("Start file %s\n", filename);
            llvm_gcda_start_file(filename, cmd->version, cmd->checksum);
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
            emit_arcs_command_t* cmd = data;
	    uint32_t num_counters = cmd->header.length / sizeof(uint64_t);
            uint64_t *counters = data + sizeof(emit_arcs_command_t);

            //printf("Emit Arcs\n");
	    printf("%c: (%d)\n", cmd->header.command, cmd->header.length);
	    printf("  counters = %x\n", num_counters);

            llvm_gcda_emit_arcs(num_counters, counters);
            break;
        }
        case COMMAND_EMIT_FUNCS:
        {
            emit_function_command_t* cmd = data;
            char* function_name = (data + sizeof(emit_function_command_t));
	    printf("%c: (%d)\n", cmd->header.command, cmd->header.length);
	    printf("  func_checksum = %x\n", cmd->func_checksum);
	    printf("  cfg_checksum = %x\n", cmd->cfg_checksum);
	    printf("  use_extra_checksum = %d\n", cmd->use_extra_checksum);
	    printf("  ident = %x\n", cmd->ident);
            printf("  function %s\n", strlen(function_name) ? function_name  : "(NULL)");

            //llvm_gcda_emit_function(ident, function_name, 0);
            llvm_gcda_emit_function(cmd->ident,
	    				strlen(function_name) ? function_name : NULL,
					cmd->func_checksum, 
					cmd->use_extra_checksum,
					cmd->cfg_checksum);
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

void llvm_gcda_start_file(const char *orig_filename, const char version[4],
                          uint32_t checksum) {
  char *filename = mangle_filename(orig_filename);
  printf("Opening output file %s\n", filename);
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

        return;
      }
    }
  }

  /* gcda file, version, stamp LLVM. */
  fwrite("adcg", 4, 1, output_file);
  fwrite(version, 4, 1, output_file);
  fwrite(&checksum, 4, 1, output_file);

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
                             uint32_t func_checksum, uint8_t use_extra_checksum,
                             uint32_t cfg_checksum) {
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
  if (function_name && strlen(function_name))
    len += 1 + length_of_string(function_name);
  write_int32(len);
  write_int32(ident);
  write_int32(func_checksum);
  if (use_extra_checksum)
    write_int32(cfg_checksum);
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
  char *new_filename;
  size_t filename_len, prefix_len;
  int prefix_strip;
  int level = 0;
  const char *fname, *ptr;
  const char *prefix = getenv("GCOV_PREFIX");
  const char *prefix_strip_str = getenv("GCOV_PREFIX_STRIP");

  if (prefix == NULL || prefix[0] == '\0')
    return strdup(orig_filename);

  if (prefix_strip_str) {
    prefix_strip = atoi(prefix_strip_str);

    /* Negative GCOV_PREFIX_STRIP values are ignored */
    if (prefix_strip < 0)
      prefix_strip = 0;
  } else {
    prefix_strip = 0;
  }

  fname = orig_filename;
  for (level = 0, ptr = fname + 1; level < prefix_strip; ++ptr) {
    if (*ptr == '\0')
      break;
    if (*ptr != '/')
      continue;
    fname = ptr;
    ++level;
  }

  filename_len = strlen(fname);
  prefix_len = strlen(prefix);
  new_filename = malloc(prefix_len + 1 + filename_len + 1);
  memcpy(new_filename, prefix, prefix_len);

  if (prefix[prefix_len - 1] != '/')
    new_filename[prefix_len++] = '/';
  memcpy(new_filename + prefix_len, fname, filename_len + 1);

  return new_filename;
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
