#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "easyjsonparser_check.h"

#include "easyjsonparser.h"


#define SHOW_LOG_OUTPUT 1


// Globals set by setup and teardown functions.

int g_log_count_all    = 0;
int g_log_count_errs   = 0;
int g_errhandler_count = 0;


// Logger callback.

void test_logger (int level, const char * msg)
{
  g_log_count_all++;
  if (level == EASYJSONPARSER_LOG_LEVEL_ERROR)
    g_log_count_errs++;

  if (SHOW_LOG_OUTPUT) {
    char * level_str = level == EASYJSONPARSER_LOG_LEVEL_ERROR
      ? "error"
      : "trace";
    fprintf(stderr, "test: %s: %s\n", level_str, msg);
  }
}

// Error handler callbacks.

int test_passthrough_errhandler (int err_code, const void * data, const char * reason, const char * errmsg)
{
  g_errhandler_count++;
  easyjsonparser_log(EASYJSONPARSER_LOG_LEVEL_ERROR, errmsg);

  return err_code;
}

int test_quashing_errhandler (int err_code, const void * data, const char * reason, const char * errmsg)
{
  g_errhandler_count++;

  return EASYJSONPARSER_SUCCESS;
}


// Tests.

START_TEST (parse_kvp_str_success)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"foo\": \"fooval\"}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_kvp_str_success_with_trace_log)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_END();

  easyjsonparser_set_loglevel(EASYJSONPARSER_LOG_LEVEL_TRACE);
  ck_assert_int_eq(easyjsonparser_parse_string("{\"foo\": \"fooval\"}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_kvp_int_success)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_INT("bar", NULL, "bar test kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"bar\": 21}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_kvp_double_success)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_DBL("doo", NULL, "doo test kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"doo\": 21.21}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_kvp_boolean_success)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_BOO("dah", NULL, "dah test kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"dah\": true}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_sub_kvp_str_success)
{
  static EASYJSONPARSER_SUBSCHEMA(sub_ys)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_MAP("sub", sub_ys, "sub data"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":{\"foo\":\"fooval\"}}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_sub_kvp_int_success)
{
  static EASYJSONPARSER_SUBSCHEMA(sub_ys)
    EASYJSONPARSER_INT("bar", NULL, "sub int"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_MAP("sub", sub_ys, "sub list"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":{\"bar\":21}}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_sub_map_str_success)
{
  static EASYJSONPARSER_SUBSCHEMA(sub_sub_ys)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SUBSCHEMA(sub_ys)
    EASYJSONPARSER_MAP("bar", sub_sub_ys, "sub obj"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_MAP("sub", sub_ys, "sub obj"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":{\"bar\":{\"foo\":\"fooval\"}}}\n", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_sub_freekey_success)
{
  static EASYJSONPARSER_SUBSCHEMA(sub_ys)
    EASYJSONPARSER_STR(NULL, NULL, "sub flexible key"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_MAP("sub", sub_ys, "sub obj"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":{\"bar\":\"123\", \"foo\":\"fooval\"}}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_sublist_str_success)
{
  static EASYJSONPARSER_SUBSCHEMA(sub_ys)
    EASYJSONPARSER_STR(NULL, NULL, "foo test kvp"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_LST("sub", sub_ys, "sub data"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":[\"foo1\",\"foo2\"]}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_sublist_int_success)
{
  static EASYJSONPARSER_SUBSCHEMA(sub_ys)
    EASYJSONPARSER_INT(NULL, NULL, "bar test kvp"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_LST("sub", sub_ys, "sub data"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":[1,2]}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_sublist_map_success)
{
  static EASYJSONPARSER_SUBSCHEMA(subobj_ys)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_INT("bar", NULL, "bar test kvp"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SUBSCHEMA(sub_ys)
    EASYJSONPARSER_MAP(NULL, subobj_ys, "sub list entry"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_LST("sub", sub_ys, "sub data"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":[{\"foo\": \"fooval\",\"bar\": 123}]}\n", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_unknown_key_somekeys_fails)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("nice", NULL, "nice kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"naughty2\":\"intruder\"}", ys, NULL), EASYJSONPARSER_ERROR_SCHEMA_UNEXPECTED_KEY);
}
END_TEST

START_TEST (parse_unknown_key_somekeys_fails_errlogs)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("nice", NULL, "nice kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"naughty2\":\"intruder\"}", ys, NULL), EASYJSONPARSER_ERROR_SCHEMA_UNEXPECTED_KEY);
  ck_assert_int_eq(g_log_count_errs, 1);
}
END_TEST

START_TEST (parse_unknown_key_nokeys_success)
{
  static EASYJSONPARSER_SCHEMA(empty_ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"naughty1\":\"intruder\"}", empty_ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

START_TEST (parse_unknown_key_somekeys_success)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("nice", NULL, "nice kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"naughty2\": \"intruder\"}", ys, NULL), EASYJSONPARSER_SUCCESS);
}
END_TEST

int calls_string_handler_callback_handler_callcount = 0;

void calls_string_handler_callback_handler (easyjsonparser_stack * stack, char * val, void * extra)
{
  calls_string_handler_callback_handler_callcount++;

  ck_assert_int_eq(strcmp(val, "fooval"), 0);
}

START_TEST (calls_string_handler_callback)
{
  static EASYJSONPARSER_SCHEMA(empty_ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("foo", calls_string_handler_callback_handler, "foo test kvp"),
    EASYJSONPARSER_END();

  calls_string_handler_callback_handler_callcount = 0;
  ck_assert_int_eq(easyjsonparser_parse_string("{\"foo\": \"fooval\"}", empty_ys, NULL), EASYJSONPARSER_SUCCESS);
  ck_assert_int_eq(calls_string_handler_callback_handler_callcount, 1);
}
END_TEST

int calls_int_handler_callback_handler_callcount = 0;

void calls_int_handler_callback_handler (easyjsonparser_stack * stack, int val, void * extra)
{
  calls_int_handler_callback_handler_callcount++;

  ck_assert_int_eq(val, 123);
}

START_TEST (calls_int_handler_callback)
{
  static EASYJSONPARSER_SCHEMA(empty_ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_INT("bar", calls_int_handler_callback_handler, "bar test kvp"),
    EASYJSONPARSER_END();

  calls_int_handler_callback_handler_callcount = 0;
  ck_assert_int_eq(easyjsonparser_parse_string("{\"bar\": 123}", empty_ys, NULL), EASYJSONPARSER_SUCCESS);
  ck_assert_int_eq(calls_int_handler_callback_handler_callcount, 1);
}
END_TEST

int calls_double_handler_callback_handler_callcount = 0;

void calls_double_handler_callback_handler (easyjsonparser_stack * stack, double val, void * extra)
{
  calls_double_handler_callback_handler_callcount++;

  ck_assert(val == 1.23);
}

START_TEST (calls_double_handler_callback)
{
  static EASYJSONPARSER_SCHEMA(empty_ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_DBL("doo", calls_double_handler_callback_handler, "doo test kvp"),
    EASYJSONPARSER_END();

  calls_double_handler_callback_handler_callcount = 0;
  ck_assert_int_eq(easyjsonparser_parse_string("{\"doo\": 1.23}", empty_ys, NULL), EASYJSONPARSER_SUCCESS);
  ck_assert_int_eq(calls_double_handler_callback_handler_callcount, 1);
}
END_TEST

int calls_boolean_handler_callback_handler_callcount = 0;

void calls_boolean_handler_callback_handler (easyjsonparser_stack * stack, int val, void * extra)
{
  calls_boolean_handler_callback_handler_callcount++;

  ck_assert_int_gt(val, 0);
}

START_TEST (calls_boolean_handler_callback)
{
  static EASYJSONPARSER_SCHEMA(empty_ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_BOO("dah", calls_boolean_handler_callback_handler, "dah test kvp"),
    EASYJSONPARSER_END();

  calls_boolean_handler_callback_handler_callcount = 0;
  ck_assert_int_eq(easyjsonparser_parse_string("{\"dah\": true}", empty_ys, NULL), EASYJSONPARSER_SUCCESS);
  ck_assert_int_eq(calls_boolean_handler_callback_handler_callcount, 1);
}
END_TEST

int handler_callback_stack_traces_path_foo_callback_handler_callcount = 0;

void handler_callback_stack_traces_path_foo_callback_handler (easyjsonparser_stack * stack, char * val, void * extra)
{
  handler_callback_stack_traces_path_foo_callback_handler_callcount++;

  ck_assert_int_eq(strcmp(stack->key, "foo"), 0);
  ck_assert_int_eq(strcmp(stack->prev->key, "abc"), 0);
  ck_assert_int_eq(strcmp(stack->prev->prev->key, "sub"), 0);
  ck_assert_ptr_eq(stack->prev->prev->prev->key, NULL);
}

int handler_callback_stack_traces_path_bar_callback_handler_callcount = 0;

void handler_callback_stack_traces_path_bar_callback_handler (easyjsonparser_stack * stack, int val, void * extra)
{
  handler_callback_stack_traces_path_bar_callback_handler_callcount++;

  ck_assert_int_eq(strcmp(stack->key, "bar"), 0);
  ck_assert_int_eq(strcmp(stack->prev->key, "abc"), 0);
  ck_assert_int_eq(strcmp(stack->prev->prev->key, "sub"), 0);
  ck_assert_ptr_eq(stack->prev->prev->prev->key, NULL);
}

START_TEST (handler_callback_stack_traces_path)
{
  static EASYJSONPARSER_SUBSCHEMA(sub_sub_ys)
    EASYJSONPARSER_STR("foo", handler_callback_stack_traces_path_foo_callback_handler, "foo test kvp"),
    EASYJSONPARSER_INT("bar", handler_callback_stack_traces_path_bar_callback_handler, "bar test kvp"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SUBSCHEMA(sub_ys)
    EASYJSONPARSER_MAP(NULL, sub_sub_ys, "sub sub obj"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_MAP("sub", sub_ys, "sub obj"),
    EASYJSONPARSER_END();

  handler_callback_stack_traces_path_foo_callback_handler_callcount = 0;
  handler_callback_stack_traces_path_bar_callback_handler_callcount = 0;
  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":{\"abc\":{\"foo\":\"yadda\",\"bar\":123}}}", ys, NULL), EASYJSONPARSER_SUCCESS);
  ck_assert_int_eq(handler_callback_stack_traces_path_foo_callback_handler_callcount, 1);
  ck_assert_int_eq(handler_callback_stack_traces_path_bar_callback_handler_callcount, 1);
}
END_TEST

START_TEST (parse_badschema_fails_errlogs)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_END();

  ys[0].type = 123;

  ck_assert_int_eq(easyjsonparser_parse_string("{\"foo\": \"fooval\"}\n", ys, NULL), EASYJSONPARSER_ERROR_SCHEMA_INVALID);
  ck_assert_int_eq(g_log_count_errs, 1);
}
END_TEST

START_TEST (parse_file_success)
{
  static EASYJSONPARSER_SCHEMA(empty_ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_END();

  int fd = open("check_json_test_input_file.json", O_CREAT | O_WRONLY | O_TRUNC, 0666);
  ck_assert_int_ge(fd, 0);
  ck_assert_int_eq(write(fd, "{\"foo\": \"fooval\"}\n", strlen("{\"foo\": \"fooval\"}\n")), strlen("{\"foo\": \"fooval\"}\n"));
  close(fd);

  ck_assert_int_eq(easyjsonparser_parse_file("check_json_test_input_file.json", empty_ys, NULL), EASYJSONPARSER_SUCCESS);

  unlink("check_json_test_input_file.json");
}
END_TEST

START_TEST (parse_file_nonexisting_fails_errlogs)
{
  static EASYJSONPARSER_SCHEMA(empty_ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_file("check_json_test_input_nonexisting_file.json", empty_ys, NULL), EASYJSONPARSER_ERROR_FILEOPEN);
  ck_assert_int_eq(g_log_count_errs, 1);
}
END_TEST

START_TEST (parse_expected_list_fails_errlogs)
{
  static EASYJSONPARSER_SUBSCHEMA(sublist_ys)
    EASYJSONPARSER_STR(NULL, NULL, "foo test kvp"),
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_LST("sub", sublist_ys, "sub data"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":{\"deeper\":{\"foo\":\"fooval\"}}}\n", ys, NULL), EASYJSONPARSER_ERROR_SCHEMA_MANDATES_LIST);
  ck_assert_int_eq(g_log_count_errs, 1);
}
END_TEST

START_TEST (parse_expected_map_fails_errlogs)
{
  static EASYJSONPARSER_SUBSCHEMA(subobj_ys)
    EASYJSONPARSER_END();
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_MAP("sub", subobj_ys, "sub data"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"sub\":\"unexpectedval\"}\n", ys, NULL), EASYJSONPARSER_ERROR_SCHEMA_MANDATES_MAP);
  ck_assert_int_eq(g_log_count_errs, 1);
}
END_TEST

START_TEST (parse_expected_str_fails_errlogs)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("foo", NULL, "foo test kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"foo\":{\"deeper\":{}}}\n", ys, NULL), EASYJSONPARSER_ERROR_SCHEMA_MANDATES_STRING);
  ck_assert_int_eq(g_log_count_errs, 1);
}
END_TEST

START_TEST (parse_expected_int_fails_errlogs)
{
  static EASYJSONPARSER_SCHEMA(ys, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_INT("bar", NULL, "bar test kvp"),
    EASYJSONPARSER_END();

  ck_assert_int_eq(easyjsonparser_parse_string("{\"bar\":{\"deeper\":{}}}\n", ys, NULL), EASYJSONPARSER_ERROR_SCHEMA_MANDATES_INT);
  ck_assert_int_eq(g_log_count_errs, 1);
}
END_TEST

START_TEST (stack_path_renders_empty_stack)
{
  easyjsonparser_stack stack1;
  stack1.key  = NULL;
  stack1.prev = NULL;

  ck_assert_int_eq(strcmp(easyjsonparser_stack_path(&stack1), "/"), 0);
}
END_TEST

START_TEST (stack_path_renders_nonempty_stack)
{
  easyjsonparser_stack stack1;
  stack1.key  = NULL;
  stack1.prev = NULL;
  easyjsonparser_stack stack2;
  stack2.key  = "foo";
  stack2.prev = &stack1;

  ck_assert_int_eq(strcmp(easyjsonparser_stack_path(&stack2), "/foo"), 0);

  easyjsonparser_stack stack3;
  stack3.key  = "bar";
  stack3.prev = &stack2;

  ck_assert_int_eq(strcmp(easyjsonparser_stack_path(&stack3), "/foo/bar"), 0);
}
END_TEST


// Fixtures.

void setup_logger (void)
{
  g_log_count_all  = 0;
  g_log_count_errs = 0;

  easyjsonparser_set_logger(test_logger);
}

void teardown_logger (void)
{
  easyjsonparser_set_logger(NULL);
}

void setup_default_logger (void)
{
  easyjsonparser_set_loglevel(EASYJSONPARSER_LOG_LEVEL_ERROR);
  easyjsonparser_set_logger(NULL);
}

void teardown_default_logger (void)
{
  easyjsonparser_set_logger(NULL);
}

void setup_passthrough_errhandler (void)
{
  easyjsonparser_set_errhandler(test_passthrough_errhandler);
}

void teardown_passthrough_errhandler (void)
{
  easyjsonparser_set_errhandler(NULL);
}

void setup_quashing_errhandler (void)
{
  easyjsonparser_set_errhandler(test_quashing_errhandler);
}

void teardown_quashing_errhandler (void)
{
  easyjsonparser_set_errhandler(NULL);
}


// Suite.

void parse_success_tests (TCase * tc, Suite * s, char ** tags, void (**fixtures)(), void * extra)
{
  tcase_add_test(tc, parse_kvp_str_success);
  tcase_add_test(tc, parse_kvp_int_success);
  tcase_add_test(tc, parse_kvp_double_success);
  tcase_add_test(tc, parse_kvp_boolean_success);
  tcase_add_test(tc, parse_sub_kvp_str_success);
  tcase_add_test(tc, parse_sub_kvp_int_success);
  tcase_add_test(tc, parse_sub_map_str_success);
  tcase_add_test(tc, parse_sub_freekey_success);
  tcase_add_test(tc, parse_sublist_str_success);
  tcase_add_test(tc, parse_sublist_int_success);
  tcase_add_test(tc, parse_sublist_map_success);
  tcase_add_test(tc, calls_string_handler_callback);
  tcase_add_test(tc, calls_int_handler_callback);
  tcase_add_test(tc, calls_double_handler_callback);
  tcase_add_test(tc, calls_boolean_handler_callback);
  tcase_add_test(tc, handler_callback_stack_traces_path);
  tcase_add_test(tc, parse_file_success);
  tcase_add_test(tc, stack_path_renders_empty_stack);
  tcase_add_test(tc, stack_path_renders_nonempty_stack);
}

void parse_failure_tests (TCase * tc, Suite * s, char ** tags, void (**fixtures)(), void * extra)
{
  tcase_add_test(tc, parse_unknown_key_somekeys_fails_errlogs);
  tcase_add_test(tc, parse_badschema_fails_errlogs);
  tcase_add_test(tc, parse_file_nonexisting_fails_errlogs);
  tcase_add_test(tc, parse_expected_list_fails_errlogs);
  tcase_add_test(tc, parse_expected_map_fails_errlogs);
  tcase_add_test(tc, parse_expected_str_fails_errlogs);
  tcase_add_test(tc, parse_expected_int_fails_errlogs);
}

void parse_tests (TCase * tc, Suite * s, char ** tags, void (**fixtures)(), void * extra)
{
  parse_success_tests(tc, s, tags, fixtures, extra);
  parse_failure_tests(tc, s, tags, fixtures, extra);

  build_suite(add_tag(tags, "alt_errhandler"),
              add_fixture(fixtures, setup_passthrough_errhandler, teardown_passthrough_errhandler),
              parse_failure_tests,
              s, NULL);
}

void default_logger_tests (TCase * tc, Suite * s, char ** tags, void (**fixtures)(), void * extra)
{
  tcase_add_test(tc, parse_kvp_str_success);
  tcase_add_test(tc, parse_kvp_str_success_with_trace_log);
  tcase_add_test(tc, parse_unknown_key_somekeys_fails);
}

void errhandler_override_tests (TCase * tc, Suite * s, char ** tags, void (**fixtures)(), void * extra)
{
  tcase_add_test(tc, parse_unknown_key_nokeys_success);
  tcase_add_test(tc, parse_unknown_key_somekeys_success);
}

Suite * mk_suite()
{
  Suite * s = suite_create("easyjsonparser");

  char ** tags        = new_tags();
  void (**fixtures)() = new_fixtures();

  build_suite(add_tag(tags, "parse"),
              add_fixture(fixtures, setup_logger, teardown_logger),
              parse_tests,
              s, NULL);

  build_suite(add_tag(tags, "default_logger"),
              add_fixture(fixtures, setup_default_logger, teardown_default_logger),
              default_logger_tests,
              s, NULL);

  build_suite(add_tag(tags, "quash_errors"),
              add_fixture(fixtures, setup_quashing_errhandler, teardown_quashing_errhandler),
              errhandler_override_tests,
              s, NULL);

  return s;
}


int main ()
{
  Suite * s = mk_suite();

  SRunner * sr = srunner_create(s);
  srunner_run_all(sr, EASYJSONPARSER_CHECK_SRUNNER_FLAGS);
  int num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  free_suite_rec_allocs();

  return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
