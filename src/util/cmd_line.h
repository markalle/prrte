/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2012      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2015-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2016-2017 Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2017-2020 IBM Corporation. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/**
 * @file
 *
 * General command line parsing facility for use throughout PRRTE.
 *
 * This scheme is inspired by the GNU getopt package.  Command line
 * options are registered.  Each option can have up to three different
 * matching tokens: a "short" name, a "single dash" name, and a "long"
 * name.  Each option can also take 0 or more arguments.  Finally,
 * each option can be repeated on the command line an arbitrary number
 * of times.
 *
 * The "short" name can only be a single letter, and will be found
 * after a single dash (e.g., "-a").  Multiple "short" names can be
 * combined into a single command line argument (e.g., "-abc" can be
 * equivalent to "-a -b -c").
 *
 * The "single dash" name is a multi-character name that only
 * requires a single dash.  This only exists to provide backwards
 * compatibility for some well-known command line options in prior
 * MPI implementations (e.g., "mpirun -np 3").  It should be used
 * sparingly.
 *
 * The "long" name is a multi-character name that is found after a
 * pair of dashes.  For example, "--some-option-name".
 *
 * A command line option is a combination of 1 or more of a short
 * name, single dash name, and a long name.  Any of the names may be
 * used on the command line; they are treated as synonyms.  For
 * example, say the following was used in for an executable named
 * "foo":
 *
 * \code
 * prrte_cmd_line_make_opt3(cmd, 'a', NULL, 'add', 1, "Add a user");
 * \endcode
 *
 * In this case, the following command lines are exactly equivalent:
 *
 * \verbatim
 * shell$ foo -a jsmith
 * shell$ foo --add jsmith
 * \endverbatim
 *
 * Note that this interface can also track multiple invocations of the
 * same option.  For example, the following is both legal and able to
 * be retrieved through this interface:
 *
 * \verbatim
 * shell$ foo -a jsmith --add bjones
 * \endverbatim
 *
 * The caller to this interface creates a command line handle
 * (prrte_cmd_line_t) with PRRTE_NEW() and then uses it to register the
 * desired parameters via prrte_cmd_line_make_opt3(). Once all the
 * parameters have been registered, the user invokes
 * prrte_cmd_line_parse() with the command line handle and the argv/argc
 * pair to be parsed (typically the arguments from main()).  The parser
 * will examine the argv and find registered options and parameters.
 * It will stop parsing when it runs into an recognized string token or
 * the special "--" token.
 *
 * After the parse has occurred, various accessor functions can be
 * used to determine which options were selected, what parameters were
 * passed to them, etc.:
 *
 * - prrte_cmd_line_get_usage_msg() returns a string suitable for "help"
 *   kinds of messages.
 * - prrte_cmd_line_is_taken() returns a true or false indicating
 *   whether a given command line option was found on the command
 *   line.
 * - prrte_cmd_line_get_argc() returns the number of tokens parsed on
 *   the handle.
 * - prrte_cmd_line_get_argv() returns any particular string from the
 *   original argv.
 * - prrte_cmd_line_get_ninsts() returns the number of times a
 *   particular option was found on a command line.
 * - prrte_cmd_line_get_param() returns the Nth parameter in the Mth
 *   instance of a given parameter.
 * - prrte_cmd_line_get_tail() returns an array of tokens not parsed
 *   (i.e., if the parser ran into "--" or an unrecognized token).
 *
 * Note that a shortcut to creating a large number of options exists
 * -- one can make a table of prrte_cmd_line_init_t instances and the
 * table to prrte_cmd_line_create().  This creates an prrte_cmd_line_t
 * handle that is pre-seeded with all the options from the table
 * without the need to repeatedly invoke prrte_cmd_line_make_opt3() (or
 * equivalent).  This prrte_cmd_line_t instance is just like any other;
 * it is still possible to add more options via
 * prrte_cmd_line_make_opt3(), etc.
 */

#ifndef PRRTE_CMD_LINE_H
#define PRRTE_CMD_LINE_H

#include "prrte_config.h"

#include "src/class/prrte_object.h"
#include "src/class/prrte_list.h"
#include "src/dss/dss_types.h"
#include "src/threads/mutex.h"

#define PRRTE_CMD_OPTIONS_MAX  15

BEGIN_C_DECLS

/**
 * Data types supported by the parser
 */
typedef enum prrte_cmd_line_type_t {
    PRRTE_CMD_LINE_TYPE_NULL,
    PRRTE_CMD_LINE_TYPE_STRING,
    PRRTE_CMD_LINE_TYPE_INT,
    PRRTE_CMD_LINE_TYPE_SIZE_T,
    PRRTE_CMD_LINE_TYPE_BOOL,

    PRRTE_CMD_LINE_TYPE_MAX
} prrte_cmd_line_type_t;


/**
 * Command line option type, for use in
 * --help output.
 */
typedef enum prrte_cmd_line_otype_t {
    PRRTE_CMD_LINE_OTYPE_GENERAL = 0,
    PRRTE_CMD_LINE_OTYPE_DEBUG,
    PRRTE_CMD_LINE_OTYPE_OUTPUT,
    PRRTE_CMD_LINE_OTYPE_INPUT,
    PRRTE_CMD_LINE_OTYPE_MAPPING,
    PRRTE_CMD_LINE_OTYPE_RANKING,
    PRRTE_CMD_LINE_OTYPE_BINDING,
    PRRTE_CMD_LINE_OTYPE_DEVEL,
    PRRTE_CMD_LINE_OTYPE_LAUNCH,
    PRRTE_CMD_LINE_OTYPE_FT,
    PRRTE_CMD_LINE_OTYPE_DVM,
    PRRTE_CMD_LINE_OTYPE_UNSUPPORTED,
    PRRTE_CMD_LINE_OTYPE_NULL
} prrte_cmd_line_otype_t;


/**
 * Main top-level handle.  This interface should not be used by users!
 */
typedef struct prrte_cmd_line_t {
    /** Make this an OBJ handle */
    prrte_object_t super;

    /** Thread safety */
    prrte_recursive_mutex_t lcl_mutex;

    /** List of prrte_cmd_line_option_t's (defined internally) */
    prrte_list_t lcl_options[PRRTE_CMD_OPTIONS_MAX];

    /** Duplicate of argc from opal_cmd_line_parse() */
    int lcl_argc;
    /** Duplicate of argv from opal_cmd_line_parse() */
    char **lcl_argv;

    /** Parsed output; list of ompi_cmd_line_param_t's (defined internally) */
    prrte_list_t lcl_params;

    /** List of tail (unprocessed) arguments */
    int lcl_tail_argc;
    /** List of tail (unprocessed) arguments */
    char **lcl_tail_argv;
} prrte_cmd_line_t;
PRRTE_EXPORT PRRTE_CLASS_DECLARATION(prrte_cmd_line_t);

/*
 * Description of a command line option
 */
typedef struct prrte_cmd_line_option_t {
    prrte_list_item_t super;

    char clo_short_name;
    char *clo_long_name;

    int clo_num_params;
    char *clo_description;

    prrte_cmd_line_type_t clo_type;
    prrte_cmd_line_otype_t clo_otype;

} prrte_cmd_line_option_t;
PRRTE_EXPORT PRRTE_CLASS_DECLARATION(prrte_cmd_line_option_t);

/*
 * An option that was used in the argv that was parsed
 */
typedef struct prrte_cmd_line_param_t {
    prrte_list_item_t super;

    /* Note that clp_arg points to storage "owned" by someone else; it
       has the original option string by reference, not by value.
       Hence, it should not be free()'ed. */

    char *clp_arg;

    /* Pointer to the existing option.  This is also by reference; it
       should not be free()ed. */

    prrte_cmd_line_option_t *clp_option;

    /* This is a list of all the parameters of this option.
       It is owned by this parameter, and should be freed when this
       param_t is freed. */

    prrte_list_t clp_values;
} prrte_cmd_line_param_t;
PRRTE_EXPORT PRRTE_CLASS_DECLARATION(prrte_cmd_line_param_t);

/**
 * Datatype used to construct a command line handle; see
 * prrte_cmd_line_create().
 */
typedef struct prrte_cmd_line_init_t {

    /** "Short" name (i.e., "-X", where "X" is a single letter) */
    char ocl_cmd_short_name;

    /** Long name (i.e., "--foo"). */
    const char *ocl_cmd_long_name;

    /** Number of parameters that this option takes */
    int ocl_num_params;

    /** datatype of any provided parameter. */
    prrte_cmd_line_type_t ocl_variable_type;

    /** Description of the command line option, to be used with
        prrte_cmd_line_get_usage_msg(). */
    const char *ocl_description;

    /** Category for --help output */
    prrte_cmd_line_otype_t ocl_otype;
} prrte_cmd_line_init_t;

/*
 * Keep track of command line options imply what MCA settings.  This is
 * needed for options like --bind-to where hwloc initialization happens
 * before the cmdline is parsed, and mca_var_register() calls start saving
 * settings that aren't visible yet.  And more generally whenever settings
 * are queried with mca_var_register() the cmdline options don't
 * necessarily cause those MCA settings to be visible.
 */
typedef struct {
  char *cmdline_arg;
  char *list_item;
  char *list_item_separators;
  char *mca_name;
  int is_required_early;
} prrte_cmdline_equivalencies_t;

extern prrte_cmdline_equivalencies_t prrte_cmd_line_equivalencies[];

/**
 * Top-level command line handle.
 *
 * This handle is used for accessing all command line functionality
 * (i.e., all prrte_cmd_line*() functions).  Multiple handles can be
 * created and simultaneously processed; each handle is independant
 * from others.
 *
 * The prrte_cmd_line_t handles are [simplistically] thread safe;
 * processing is guaranteed to be mutually exclusive if multiple
 * threads invoke functions on the same handle at the same time --
 * access will be serialized in an unspecified order.
 *
 * Once finished, handles should be released with PRRTE_RELEASE().  The
 * destructor for prrte_cmd_line_t handles will free all memory
 * associated with the handle.
 */

/**
 * Make a command line handle from a table of initializers.
 *
 * @param cmd PRRTE command line handle.
 * @param table Table of prrte_cmd_line_init_t instances for all
 * the options to be included in the resulting command line
 * handler.
 *
 * @retval PRRTE_SUCCESS Upon success.
 *
 * This function takes a table of prrte_cmd_line_init_t instances
 * to pre-seed an PRRTE command line handle.  The last instance in
 * the table must have '\0' for the short name and NULL for the
 * single-dash and long names.  The handle is expected to have
 * been PRRTE_NEW'ed or PRRTE_CONSTRUCT'ed already.
 *
 * Upon return, the command line handle is just like any other.  A
 * sample using this syntax:
 *
 * \code
 * prrte_cmd_line_init_t cmd_line_init[] = {
 *    { 'h', "help", 0,
 *      PRRTE_CMD_LINE_TYPE_BOOL,
 *      "This help message" },
 *
 *    { '\0', "wd", 1,
 *      PRRTE_CMD_LINE_TYPE_STRING,
 *      "Set the working directory of the started processes" },
 *
 *    { '\0', NULL, 0,
 *      PRRTE_CMD_LINE_TYPE_NULL, NULL }
 * };
 * \endcode
 */
PRRTE_EXPORT int prrte_cmd_line_create(prrte_cmd_line_t *cmd,
                                       prrte_cmd_line_init_t *table);

/* Add a table of prrte_cmd_line_init_t instances
 * to an existing PRRTE command line handle.
 *
 * Multiple calls to prrte_cmd_line_add are permitted - each
 * subsequent call will simply append new options to the existing
 * handle. Note that any duplicates will return an error.
 */
 PRRTE_EXPORT int prrte_cmd_line_add(prrte_cmd_line_t *cmd,
                                     prrte_cmd_line_init_t *table);

/**
 * Create a command line option.
 *
 * @param cmd PRRTE command line handle.
 * @param entry Command line entry to add to the command line.
 *
 * @retval PRRTE_SUCCESS Upon success.
 *
 */
PRRTE_EXPORT int prrte_cmd_line_make_opt_mca(prrte_cmd_line_t *cmd,
                                             prrte_cmd_line_init_t entry);

/**
 * Create a command line option.
 *
 * @param cmd PRRTE command line handle.
 * @param short_name "Short" name of the command line option.
 * @param long_name "Long" name of the command line option.
 * @param num_params How many parameters this option takes.
 * @param dest Short string description of this option.
 *
 * @retval PRRTE_ERR_OUT_OF_RESOURCE If out of memory.
 * @retval PRRTE_ERR_BAD_PARAM If bad parameters passed.
 * @retval PRRTE_SUCCESS Upon success.
 *
 * Adds a command line option to the list of options that an PRRTE
 * command line handle will accept.  The short_name may take the
 * special value '\0' to not have a short name.  Likewise, the
 * long_name may take the special value NULL to not have
 * long name.  However, one of the
 * two must have a name.
 *
 * num_params indicates how many parameters this option takes.  It
 * must be greater than or equal to 0.
 *
 * Finally, desc is a short string description of this option.  It is
 * used to generate the output from prrte_cmd_line_get_usage_msg().
 *
 */
PRRTE_EXPORT int prrte_cmd_line_make_opt3(prrte_cmd_line_t *cmd,
                                          char short_name,
                                          const char *long_name,
                                          int num_params,
                                          const char *desc,
                                          prrte_cmd_line_otype_t otype);

/**
 * Parse a command line according to a pre-built PRRTE command line
 * handle.
 *
 * @param cmd PRRTE command line handle.
 * @param ignore_unknown Whether to print an error message upon
 * finding an unknown token or not
 * @param ignore_unknown_option Whether to print an error message upon
 * finding an unknown option or not
 * @param argc Length of the argv array.
 * @param argv Array of strings from the command line.
 *
 * @retval PRRTE_SUCCESS Upon success.
 * @retval PRRTE_ERR_SILENT If an error message was printed.  This
 * value will only be returned if the command line was not
 * successfully parsed.
 *
 * Parse a series of command line tokens according to the option
 * descriptions from a PRRTE command line handle.  The PRRTE command line
 * handle can then be queried to see what options were used, what
 * their parameters were, etc.
 *
 * If an unknown token is found in the command line (i.e., a token
 * that is not a parameter or a registered option), the parsing will
 * stop (see below).  If ignore_unknown is false, an error message
 * is displayed.  If ignore_unknown is true, the error message is
 * not displayed.
 *
 * Error messages are always displayed regardless of the value
 * of ignore_unknown (to stderr, and PRRTE_ERR_SILENT is
 * returned) if:
 *
 * 1. A token was encountered that required N parameters, but <N
 * parameters were found (e.g., "cmd --param foo", but --param was
 * registered to require 2 option tokens).
 *
 * 2. An unknown token beginning with "-" is encountered.  For
 * example, if "--fo" is specified, and no "fo" option is
 * registered (e.g., perhaps the user meant to type "--foo"), an
 * error message is always printed, UNLESS this unknown token
 * happens after a "--" token (see below).
 *
 * The contents of argc and argv are not changed during parsing.
 * argv[0] is assumed to be the executable name, and is ignored during
 * parsing, except when printing error messages.
 *
 * Parsing will stop in the following conditions:
 *
 * - all argv tokens are processed
 * - the token "--" is found
 * - an unrecognized token is found
 * - a parameter registered with an integer type option finds a
 *   non-integer option token
 * - a parameted registered N option tokens, but finds less then
 *   <N tokens available
 *
 * Upon any of these conditions, any remaining tokens will be placed
 * in the "tail" (and therefore not examined by the parser),
 * regardless of the value of ignore_unknown.  The set of tail
 * tokens is available from the prrte_cmd_line_get_tail() function.
 *
 * Note that "--" is ignored if it is found in the middle an expected
 * number of arguments.  For example, if "--foo" is expected to have 3
 * arguments, and the command line is:
 *
 * executable --foo a b -- other arguments
 *
 * This will result in an error, because "--" will be parsed as the
 * third parameter to the first instance of "foo", and "other" will be
 * an unrecognized option.
 *
 * Note that -- can be used to allow unknown tokens that begin
 * with "-".  For example, if a user wants to mpirun an executable
 * named "-my-mpi-program", the "usual" way:
 *
 *   mpirun -my-mpi-program
 *
 * will cause an error, because mpirun won't find single-letter
 * options registered for some/all of those letters.  But two
 * workarounds are possible:
 *
 *   mpirun -- -my-mpi-program
 * or
 *   mpirun ./-my-mpi-program
 *
 * Finally, note that invoking this function multiple times on
 * different sets of argv tokens is safe, but will erase any
 * previous parsing results.
 */
PRRTE_EXPORT int prrte_cmd_line_parse(prrte_cmd_line_t *cmd,
                                      bool ignore_unknown,
                                      bool ignore_unknown_option,
                                      int argc, char **argv);

/**
 * Return a consolidated "usage" message for a PRRTE command line handle.
 *
 * @param cmd PRRTE command line handle.
 *
 * @retval str Usage message.
 *
 * Returns a formatted string suitable for printing that lists the
 * expected usage message and a short description of each option on
 * the PRRTE command line handle.  Options that passed a NULL
 * description to prrte_cmd_line_make_opt3() will not be included in the
 * display (to allow for undocumented options).
 *
 * This function is typically only invoked internally by the
 * prrte_show_help() function.
 *
 * This function should probably be fixed up to produce prettier
 * output.
 *
 * The returned string must be freed by the caller.
 */
PRRTE_EXPORT char *prrte_cmd_line_get_usage_msg(prrte_cmd_line_t *cmd, bool parseable) __prrte_attribute_malloc__ __prrte_attribute_warn_unused_result__;

/**
 * Test if a given option was taken on the parsed command line.
 *
 * @param cmd PRRTE command line handle.
 * @param opt Short or long name of the option to check for.
 *
 * @retval true If the command line option was found during
 * prrte_cmd_line_parse().
 *
 * @retval false If the command line option was not found during
 * prrte_cmd_line_parse(), or prrte_cmd_line_parse() was not invoked on
 * this handle.
 *
 * This function should only be called after prrte_cmd_line_parse().
 *
 * The function will return true if the option matching opt was found
 * (either by its short or long name) during token parsing.
 * Otherwise, it will return false.
 */
PRRTE_EXPORT bool prrte_cmd_line_is_taken(prrte_cmd_line_t *cmd,
                                          const char *opt) __prrte_attribute_nonnull__(1) __prrte_attribute_nonnull__(2);

/**
 * Return the number of instances of an option found during parsing.
 *
 * @param cmd PRRTE command line handle.
 * @param opt Short or long name of the option to check for.
 *
 * @retval num Number of instances (to include 0) of a given option
 * found during prrte_cmd_line_parse().
 *
 * @retval PRRTE_ERR If the command line option was not found during
 * prrte_cmd_line_parse(), or prrte_cmd_line_parse() was not invoked on
 * this handle.
 *
 * This function should only be called after prrte_cmd_line_parse().
 *
 * The function will return the number of instances of a given option
 * (either by its short or long name) -- to include 0 -- or PRRTE_ERR if
 * either the option was not specified as part of the PRRTE command line
 * handle, or prrte_cmd_line_parse() was not invoked on this handle.
 */
PRRTE_EXPORT int prrte_cmd_line_get_ninsts(prrte_cmd_line_t *cmd,
                                           const char *opt) __prrte_attribute_nonnull__(1) __prrte_attribute_nonnull__(2);

/**
 * Return a specific parameter for a specific instance of a option
 * from the parsed command line.
 *
 * @param cmd PRRTE command line handle.
 * @param opt Long name of the option to check for.
 * @param instance_num Instance number of the option to query.
 * @param param_num Which parameter to return.
 *
 * @retval param String of the parameter.
 * @retval NULL If any of the input values are invalid.
 *
 * This function should only be called after prrte_cmd_line_parse().
 *
 * This function returns the Nth parameter for the Ith instance of a
 * given option on the parsed command line (both N and I are
 * zero-indexed).  For example, on the command line:
 *
 * executable --foo bar1 bar2 --foo bar3 bar4
 *
 * The call to prrte_cmd_line_get_param(cmd, "foo", 1, 1) would return
 * "bar4".  prrte_cmd_line_get_param(cmd, "bar", 0, 0) would return
 * NULL, as would prrte_cmd_line_get_param(cmd, "foo", 2, 2);
 *
 * The returned string should \em not be modified or freed by the
 * caller.
 */
PRRTE_EXPORT prrte_value_t *prrte_cmd_line_get_param(prrte_cmd_line_t *cmd,
                                                     const char *opt,
                                                     int instance_num,
                                                     int param_num);

/**
 * The next function is a wrapper of the above that additionally checks
 * if the setting is available at the specified MCA setting env var
 */
PRRTE_EXPORT prrte_value_t *prrte_cmd_line_get_param_or_env(prrte_cmd_line_t *cmd,
                                                     const char *opt,
                                                     const char *env,
                                                     int instance_num,
                                                     int param_num);

/**
 * Return the entire "tail" of unprocessed argv from a PRRTE
 * command line handle.
 *
 * @param cmd A pointer to the PRRTE command line handle.
 * @param tailc Pointer to the output length of the null-terminated
 * tail argv array.
 * @param tailv Pointer to the output null-terminated argv of all
 * unprocessed arguments from the command line.
 *
 * @retval PRRTE_ERROR If cmd is NULL or otherwise invalid.
 * @retval PRRTE_SUCCESS Upon success.
 *
 * The "tail" is all the arguments on the command line that were
 * not processed for some reason.  Reasons for not processing
 * arguments include:
 *
 * \sa The argument was not recognized
 * \sa The argument "--" was seen, and therefore all arguments
 * following it were not processed
 *
 * The output tailc parameter will be filled in with the integer
 * length of the null-terminated tailv array (length including the
 * final NULL entry).  The output tailv parameter will be a copy
 * of the tail parameters, and must be freed (likely with a call
 * to prrte_argv_free()) by the caller.
 */
PRRTE_EXPORT int prrte_cmd_line_get_tail(prrte_cmd_line_t *cmd, int *tailc,
                                         char ***tailv);

PRRTE_EXPORT prrte_cmd_line_option_t *prrte_cmd_line_find_option(prrte_cmd_line_t *cmd,
                                                                 prrte_cmd_line_init_t *e) __prrte_attribute_nonnull__(1) __prrte_attribute_nonnull__(2);

END_C_DECLS


#endif /* PRRTE_CMD_LINE_H */
