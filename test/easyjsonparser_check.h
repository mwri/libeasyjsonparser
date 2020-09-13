/// \file
/// \brief easyjsonparser check common utils.


#ifndef EASYJSONPARSER_CHECK_INCLUDED
#define EASYJSONPARSER_CHECK_INCLUDED


#include <check.h>
#include <stddef.h>


#define EASYJSONPARSER_CHECK_SRUNNER_FLAGS CK_VERBOSE
#define EASYJSONPARSER_MAX_REC_LEVELS      64
#define EASYJSONPARSER_MAX_ALLOC_TRACK     8192


extern char ** new_tags ();
extern char ** add_tag (char ** tags, char * tag);
extern void *  new_fixtures ();
extern void *  add_fixture (void (**fixtures)(), void (*setup)(), void (*teardown)());
extern void    build_suite (char ** tags, void (**fixtures)(), void (*add_tests_fn)(), Suite * s, void * extra);
extern void    free_suite_rec_allocs ();


#endif // EASYJSONPARSER_CHECK_INCLUDED
