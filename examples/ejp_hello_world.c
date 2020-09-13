/// \file
/// \brief Simple 'hello universe' use example for libeasyjsonparser.
///
/// This example shows how to parse a simple JSON config file for a
/// mythical RESTful API service, and the config will look like this:
///
/// {
///   "restapi": {
///     "base-path": "/api",
///     "port": 80,
///     "ssl": false
///   },
///   "version": "1.5.4"
/// }
///
/// In this case the version is handled as a string so that it can be
/// a 3 part semantic version, but you could parse it as an integer or
/// float too if it suited.


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "easyjsonparser.h"


/// The structure the JSON file will be parsed into.

struct hello_config {
  int    valid;
  char * version;
  int    svr_port;
  int    svr_ssl;
  char * svr_base_path;
};


/// Declare callbacks.

static void ey_handle_version (easyjsonparser_stack * stack, char * val, struct hello_config * cfg);
static void ey_handle_svr_port (easyjsonparser_stack * stack, int val, struct hello_config * cfg);
static void ey_handle_svr_ssl (easyjsonparser_stack * stack, int val, struct hello_config * cfg);
static void ey_handle_svr_base_path (easyjsonparser_stack * stack, char * val, struct hello_config * cfg);


/// JSON schema.

static easyjsonparser_schema * schema ()
{
  static EASYJSONPARSER_SUBSCHEMA(restapi_js)
    EASYJSONPARSER_INT("port",      &ey_handle_svr_port,      "TCP port"     ),
    EASYJSONPARSER_BOO("ssl",       &ey_handle_svr_ssl,       "SSL enabled"  ),
    EASYJSONPARSER_STR("base-path", &ey_handle_svr_base_path, "URL base path"),
    EASYJSONPARSER_END();

  static EASYJSONPARSER_SCHEMA(js, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("version", &ey_handle_version, "Configuration version"),
    EASYJSONPARSER_MAP("restapi", restapi_js,         "RESTful API restapi"  ),
    EASYJSONPARSER_END();

  return js;
}


/// Schema parsing callbacks.

static void ey_handle_version (easyjsonparser_stack * stack, char * val, struct hello_config * cfg)
{
  cfg->version = (char *) malloc(strlen(val) + 1);
  strcpy(cfg->version, val);
}

static void ey_handle_svr_port (easyjsonparser_stack * stack, int val, struct hello_config * cfg)
{
  cfg->svr_port = val;
}

static void ey_handle_svr_ssl (easyjsonparser_stack * stack, int val, struct hello_config * cfg)
{
  cfg->svr_ssl = val;
}

static void ey_handle_svr_base_path (easyjsonparser_stack * stack, char * val, struct hello_config * cfg)
{
  cfg->svr_base_path = (char *) malloc(strlen(val) + 1);
  strcpy(cfg->svr_base_path, val);
}


/// Parse the file and return the resultant configuration structure.

int main (int argc, char ** argv)
{
  // easyjsonparser_set_loglevel(EASYJSONPARSER_LOG_LEVEL_TRACE);

  struct hello_config * cfg = malloc(sizeof(struct hello_config));

  cfg->valid = 1;

  // For unit tests.
  char * filename = argv[1];
  if (filename == NULL) {
    filename = (char *) malloc(strlen(argv[0]) + strlen(".json") + 1);
    sprintf(filename, "%s.json", argv[0]);
    printf("%s\n", filename);
  }

  printf("loading %s\n", filename);
  if (easyjsonparser_parse_file(filename, schema(), cfg) != EASYJSONPARSER_SUCCESS || !cfg->valid) {
    fprintf(stderr, "ey_hello_universe: reading configuration failed, sorry\n");
    exit(1);
  }

  printf("ey_hello_universe: success, version %s config (restapi SSL=%s, port=%d, URL=%s)\n",
         cfg->version, (cfg->svr_ssl ? "true" : "false"), cfg->svr_port, cfg->svr_base_path
         );

  exit(0);
}
