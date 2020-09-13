/// \file
/// \brief Simple 'hello world' use example for libeasyjson.
///
/// This example shows an absolute bare bones starter JSON file parse
/// example.
///
/// The JSON file to be parsed will have a single value called 'count'
/// which will take an integer value, so it will look like:
///
/// {"count": 4}
///
/// For a more realistic example where a JSON file involving different
/// data types is parsed into a structure, see 'ey_hello_universe'.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "easyjsonparser.h"


/// Callback to set the count.

static void set_count (easyjsonparser_stack * stack, int val, int * countp)
{
  *countp = val;
}


/// Parse the file and return the resultant configuration structure.

int main (int argc, char ** argv)
{
  // easyjsonparser_set_loglevel(EASYJSONPARSER_LOG_LEVEL_TRACE);

  int count;

  // Define the schema, which is a JSON document with just one item called 'count'.
  static EASYJSONPARSER_SCHEMA(schema, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_INT("count", &set_count, "Count"),
    EASYJSONPARSER_END();

  // For unit tests.
  char * filename = argv[1];
  if (filename == NULL) {
    filename = (char *) malloc(strlen(argv[0]) + strlen(".json") + 1);
    sprintf(filename, "%s.json", argv[0]);
    printf("%s\n", filename);
  }

  // Parse the JSON file, passing the filename, the schema, and
  // a pointer to p where the result will be parsed to.
  if (easyjsonparser_parse_file(filename, schema, &count) != EASYJSONPARSER_SUCCESS) {
    fprintf(stderr, "ey_hello_world: JSON read/parse failed, sorry\n");
    exit(1);
  }

  printf("ey_hello_world: count = %d\n", count);

  exit(0);
}
