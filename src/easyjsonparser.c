#include <stdio.h>
#include <json-c/json.h>
#include <errno.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>

#include "config.h"
#include "easyjsonparser.h"


/// Local function declarations.

static int    parse (json_object * jobj, easyjsonparser_schema * js, void * cfg);
static int    rec_parse (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * c);
static int    rec_parse_obj (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * c);
static int    rec_parse_obj_varkeys (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * cfg);
static int    rec_parse_obj_fixedkeys (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * cfg);
static int    rec_parse_list (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * c);
static char * jobj_type_to_str (enum json_type jobj_type);
static char * stack_render_rec (easyjsonparser_stack * stack, char * buf, char * buf_of);
static int    error_handler (int err_code, const void * data, const char * reason, const char * errmsg_fmt, ...);

/// Logger, log level and error handler intialisation.

static int logger_loglevel = EASYJSONPARSER_LOG_LEVEL_ERROR;
static void (*alt_logger)(int level, const char * fmt) = NULL;
static int (*alt_errhandler)(int err_code, const void * data, const char * reason, const char * errmsg_fmt) = NULL;


/// Set the log level (used only by the default logger).

void easyjsonparser_set_loglevel (int loglevel)
{
  logger_loglevel = loglevel;
}


/// Replace the logger.

void easyjsonparser_set_logger (void (*logger)(int, const char *))
{
  alt_logger = logger;
}


/// Replace the error handler.

void easyjsonparser_set_errhandler (int (*handler)(int, const void *, const char *, const char *))
{
  alt_errhandler = handler;
}


/// Open and parse the JSON file.

int easyjsonparser_parse_file (const char * filename, easyjsonparser_schema * js, void * cfg)
{
  FILE * fh = fopen(filename, "r");
  if (fh == NULL)
    return error_handler(EASYJSONPARSER_ERROR_FILEOPEN, filename, strerror(errno), "error opening config file (%s)", strerror(errno));

  char * buf = NULL;
  int buf_size = 0, buf_used = 0;
  for (int read_len = -1; read_len != 0;) {
    if (buf_size - buf_used == 0) {
      buf_size += 1024;
      buf = (char *) realloc(buf, buf_size);
    }
    read_len = fread(buf, 1, buf_size - buf_used, fh);
    buf_used += read_len;
  }
  fclose(fh);

  int retval = easyjsonparser_parse_string(buf, js, cfg);
  free(buf);
  return retval;
}


/// Parse the zero byte terminated JSON string.

int easyjsonparser_parse_string (const char * input_string, easyjsonparser_schema * js, void * cfg)
{
  struct json_tokener * parser = json_tokener_new();

  struct json_object * jobj = json_tokener_parse_ex(parser, input_string, strlen(input_string));

  if (jobj == NULL) {
    enum json_tokener_error libjsonc_err = json_tokener_get_error(parser);
    return error_handler(EASYJSONPARSER_ERROR_LIBJSONC_PARSE, &libjsonc_err,
                         "json_tokener_parse_ex() returned error",
                         "could not parse JSON (json_tokener_parse_ex() returned error)");
  }

  int retval = parse(jobj, js, cfg);

  json_tokener_free(parser);

  return retval;
}


/// Parse the JSON. Called from \ref easyjsonparser_parse_file or
/// \ref easyjsonparser_parse_string to complete the parsing of the source.

int parse (json_object * jobj, easyjsonparser_schema * js, void * cfg)
{
  easyjsonparser_log(EASYJSONPARSER_LOG_LEVEL_TRACE, "JSON root processing");

  easyjsonparser_stack stack;
  stack.key  = NULL;
  stack.prev = NULL;

  if ((js->type & EASYJSONPARSER_SCHEMA_TYPE_BITS) == EASYJSONPARSER_SCHEMA_MAP)
    return rec_parse_obj(jobj, js + 1, &stack, cfg);
  else if ((js->type & EASYJSONPARSER_SCHEMA_TYPE_BITS) == EASYJSONPARSER_SCHEMA_LST)
    return rec_parse_list(jobj, js + 1, &stack, cfg);
  else
    return rec_parse(jobj, js, &stack, cfg);
}


/// Recursive parse of a JSON map/object.

int rec_parse_obj (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * cfg)
{
  easyjsonparser_log(EASYJSONPARSER_LOG_LEVEL_TRACE, "JSON object processing");

  return js[0].type != EASYJSONPARSER_SCHEMA_END && js[1].type == EASYJSONPARSER_SCHEMA_END && js[0].key == NULL
    ? rec_parse_obj_varkeys (jobj, js, stack, cfg)
    : rec_parse_obj_fixedkeys (jobj, js, stack, cfg);
}


/// Recursive parse JSON map variable keys.

int rec_parse_obj_varkeys (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * cfg)
{
  easyjsonparser_log(EASYJSONPARSER_LOG_LEVEL_TRACE, "JSON variable key object processing");

  int retval = EASYJSONPARSER_SUCCESS;

  json_object_object_foreach(jobj, key, jobj2) {
    easyjsonparser_stack stack2;
    stack2.key  = (char *) key;
    stack2.prev = stack;

    if ((retval = rec_parse(jobj2, js, &stack2, cfg)) != EASYJSONPARSER_SUCCESS)
      break;
  }

  return retval;
}


/// Recursive parse JSON map fixed keys.

int rec_parse_obj_fixedkeys (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * cfg)
{
  easyjsonparser_log(EASYJSONPARSER_LOG_LEVEL_TRACE, "JSON fixed key object processing");

  int retval = EASYJSONPARSER_SUCCESS;

  json_object_object_foreach(jobj, key, jobj2) {
    easyjsonparser_schema * js2 = js;
    while (js2->type != EASYJSONPARSER_SCHEMA_END) {
      if (strcmp(js2->key, (char *) key) == 0) {
        easyjsonparser_stack stack2;
        stack2.key  = key;
        stack2.prev = stack;

        retval = rec_parse(jobj2, js2, &stack2, cfg);
        break;
      }

      js2++;
    }

    if (js2->type == EASYJSONPARSER_SCHEMA_END) {
      char * stack_path = easyjsonparser_stack_path(stack);
      void * data[3] = {js, stack_path, key};
      retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_UNEXPECTED_KEY, data,
                                 "unexpected key",
                                 "key %s unexpected while parsing map at %s",
                                 key, stack_path);
    }

    if (retval != EASYJSONPARSER_SUCCESS)
      break;
  }

  return retval;
}


/// Recursive parse of a JSON list.

int rec_parse_list (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * cfg)
{
  easyjsonparser_log(EASYJSONPARSER_LOG_LEVEL_TRACE, "JSON list/array processing");

  for (int i = 0; i < json_object_array_length(jobj); i++) {
    for (easyjsonparser_schema * js2 = js; js2->type != EASYJSONPARSER_SCHEMA_END; js2++) {
      int rec_result = rec_parse(json_object_array_get_idx(jobj, i), js2, stack, cfg);
      if (rec_result != EASYJSONPARSER_SUCCESS)
        return rec_result;
    }
  }

  return EASYJSONPARSER_SUCCESS;
}


/// Recursive parse of a JSON something (could be anything in this context).

int rec_parse (json_object * jobj, easyjsonparser_schema * js, easyjsonparser_stack * stack, void * cfg)
{
  enum json_type jobj_type = json_object_get_type(jobj);
  char * stack_path = easyjsonparser_stack_path(stack);
  char * jobj_type_str = jobj_type_to_str(jobj_type);
  void * data[3] = {js, stack_path, jobj_type_str};

  easyjsonparser_log(EASYJSONPARSER_LOG_LEVEL_TRACE, "JSON scalar processing, found %s", jobj_type_str);

  if (js->type == EASYJSONPARSER_SCHEMA_STR) {
    if (jobj_type == json_type_string) {
      if (js->data != NULL)
        ((void (*)(easyjsonparser_stack *, char *, void *)) js->data)(stack, (char *) json_object_get_string(jobj), cfg);
    } else {
      int retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_MANDATES_STRING, data,
                                 "string mandated by schema",
                                 "%s (%s) must be a string at %s",
                                 js->key, js->descr, stack_path);
      if (retval != EASYJSONPARSER_SUCCESS)
        return retval;
    }
  } else if (js->type == EASYJSONPARSER_SCHEMA_INT) {
    if (jobj_type == json_type_int) {
      if (js->data != NULL)
        ((void (*)(easyjsonparser_stack *, int, void *)) js->data)(stack, json_object_get_int(jobj), cfg);
    } else {
      int retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_MANDATES_INT, data,
                                 "integer mandated by schema",
                                 "%s (%s) must be an integer at %s",
                                 js->key, js->descr, stack_path);
      if (retval != EASYJSONPARSER_SUCCESS)
        return retval;
    }
  } else if (js->type == EASYJSONPARSER_SCHEMA_DBL) {
    if (jobj_type == json_type_double) {
      if (js->data != NULL)
        ((void (*)(easyjsonparser_stack *, double, void *)) js->data)(stack, json_object_get_double(jobj), cfg);
    } else {
      int retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_MANDATES_DOUBLE, data,
                                 "double mandated by schema",
                                 "%s (%s) must be a double/float at %s",
                                 js->key, js->descr, stack_path);
      if (retval != EASYJSONPARSER_SUCCESS)
        return retval;
    }
  } else if (js->type == EASYJSONPARSER_SCHEMA_BOO) {
    if (jobj_type == json_type_boolean) {
      if (js->data != NULL)
        ((void (*)(easyjsonparser_stack *, int, void *)) js->data)(stack, json_object_get_boolean(jobj), cfg);
    } else {
      int retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_MANDATES_BOOL, data,
                                 "boolean mandated by schema",
                                 "%s (%s) must be a boolean at %s",
                                 js->key, js->descr, stack_path);
      if (retval != EASYJSONPARSER_SUCCESS)
        return retval;
    }
  } else if (js->type == EASYJSONPARSER_SCHEMA_MAP) {
    if (jobj_type == json_type_object) {
      return rec_parse_obj(jobj, js->data, stack, cfg);
    } else {
      int retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_MANDATES_MAP, data,
                                 "map/object mandated by schema",
                                 "%s (%s) must be a map/object at %s",
                                 js->key, js->descr, stack_path);
      if (retval != EASYJSONPARSER_SUCCESS)
        return retval;
    }
  } else if (js->type == EASYJSONPARSER_SCHEMA_LST) {
    if (jobj_type == json_type_array) {
      return rec_parse_list(jobj, js->data, stack, cfg);
    } else {
      int retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_MANDATES_LIST, data,
                                 "list/array mandated by schema",
                                 "%s (%s) must be a list/array at %s",
                                 js->key, js->descr, stack_path);
      if (retval != EASYJSONPARSER_SUCCESS)
        return retval;
    }
  } else if (js->type == EASYJSONPARSER_SCHEMA_NUL) {
    if (jobj_type == json_type_null) {
      if (js->data != NULL)
        ((void (*)(easyjsonparser_stack *, void *)) js->data)(stack, cfg);
    } else {
      int retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_MANDATES_NULL, data,
                                 "null mandated by schema",
                                 "%s (%s) must be a null at %s",
                                 js->key, js->descr, stack_path);
      if (retval != EASYJSONPARSER_SUCCESS)
        return retval;
    }
  } else {
    int retval = error_handler(EASYJSONPARSER_ERROR_SCHEMA_INVALID, data,
                               "schema invalid",
                               "schema has invalid/corrupt type %d at %s",
                               js->type, stack_path);
    if (retval != EASYJSONPARSER_SUCCESS)
      return retval;
  }

  return EASYJSONPARSER_SUCCESS;
}


/// Return a string representing the given libjsonparser token.

char * easyjsonparser_stack_path (easyjsonparser_stack * stack)
{
  static char buf[MAX_STACKPATH_LEN];

  if (stack->key == NULL)
    snprintf(buf, MAX_STACKPATH_LEN, "/");
  else
    stack_render_rec(stack, buf, buf + MAX_STACKPATH_LEN);

  return buf;
}


/// Recursively render the stack, called by \ref easyjsonparser_stack_path.

char * stack_render_rec (easyjsonparser_stack * stack, char * buf, char * buf_of)
{
  if (stack->key == NULL)
    return buf;

  snprintf(stack_render_rec(stack->prev, buf, buf_of), buf_of - buf, "/%s", stack->key);

  return buf + strlen(buf);
}


/// Return a string representing the given libjsonparser token.

char * jobj_type_to_str (enum json_type jobj_type)
{
  switch (jobj_type) {
  case json_type_object:
    return "object";
    break;

  case json_type_array:
    return "array";
    break;

  case json_type_null:
    return "null";
    break;

  case json_type_boolean:
    return "boolean";
    break;

  case json_type_double:
    return "double";
    break;

  case json_type_int:
    return "int";
    break;

  case json_type_string:
    return "string";
    break;

  default:
    return "unknown";
    break;
  }
}


/// Log something (the library logs at the 'error' and 'trace' levels).

void easyjsonparser_log (int level, const char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  if (alt_logger == NULL) {
    if (level > logger_loglevel)
      return;

    fprintf(stderr, "libeasyjsonparser: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    return;
  }

  char msg[MAX_LOGMSG_LEN];
  vsnprintf(msg, MAX_LOGMSG_LEN, fmt, args);

  alt_logger(level, msg);
}


/// Handle an error.

int error_handler (int err_code, const void * data, const char * reason, const char * errmsg_fmt, ...)
{
  va_list args;
  va_start(args, errmsg_fmt);
  char errmsg[MAX_LOGMSG_LEN];
  vsnprintf(errmsg, MAX_LOGMSG_LEN, errmsg_fmt, args);

  if (alt_errhandler == NULL) {
    easyjsonparser_log(EASYJSONPARSER_LOG_LEVEL_ERROR, errmsg);
    return err_code;
  }

  if ((err_code & EASYJSONPARSER_ERROR_FATAL_BITS) == 0)
    return alt_errhandler(err_code, data, reason, errmsg);

  alt_errhandler(err_code, data, reason, errmsg);
  return err_code;
}
