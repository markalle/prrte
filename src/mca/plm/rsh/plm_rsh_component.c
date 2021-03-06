/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2008 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007-2015 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2008-2009 Sun Microsystems, Inc.  All rights reserved.
 * Copyright (c) 2010      Oracle and/or its affiliates.  All rights
 *                         reserved.
 * Copyright (c) 2009-2018 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2011-2020 IBM Corporation.  All rights reserved.
 * Copyright (c) 2015-2020 Intel, Inc.  All rights reserved.
 * Copyright (c) 2019      Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * These symbols are in a file by themselves to provide nice linker
 * semantics.  Since linkers generally pull in symbols by object
 * files, keeping these symbols as the only symbols in this file
 * prevents utility programs such as "ompi_info" from having to import
 * entire components just to query their version and parameters.
 */

#include "prrte_config.h"
#include "constants.h"

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>

#include "src/util/prrte_environ.h"
#include "src/util/output.h"
#include "src/util/argv.h"
#include "src/util/basename.h"
#include "src/util/path.h"
#include "src/util/string_copy.h"

#include "src/mca/state/state.h"
#include "src/util/name_fns.h"
#include "src/runtime/prrte_globals.h"
#include "src/util/show_help.h"

#include "src/mca/plm/plm.h"
#include "src/mca/plm/base/plm_private.h"
#include "src/mca/plm/rsh/plm_rsh.h"

/*
 * Public string showing the plm ompi_rsh component version number
 */
const char *prrte_plm_rsh_component_version_string =
  "PRRTE rsh plm MCA component version " PRRTE_VERSION;


static int rsh_component_register(void);
static int rsh_component_open(void);
static int rsh_component_query(prrte_mca_base_module_t **module, int *priority);
static int rsh_component_close(void);
static int rsh_launch_agent_lookup(const char *agent_list, char *path);

/* Local variables */
static char *prrte_plm_rsh_delay_string = NULL;
static int agent_var_id = -1;

/*
 * Instantiate the public struct with all of our public information
 * and pointers to our public functions in it
 */

prrte_plm_rsh_component_t prrte_plm_rsh_component = {
    {
    /* First, the mca_component_t struct containing meta information
       about the component itself */

        .base_version = {
            PRRTE_PLM_BASE_VERSION_2_0_0,

            /* Component name and version */
            .mca_component_name = "rsh",
            PRRTE_MCA_BASE_MAKE_VERSION(component, PRRTE_MAJOR_VERSION, PRRTE_MINOR_VERSION,
                                        PRRTE_RELEASE_VERSION),

            /* Component open and close functions */
            .mca_open_component = rsh_component_open,
            .mca_close_component = rsh_component_close,
            .mca_query_component = rsh_component_query,
            .mca_register_component_params = rsh_component_register,
        },
        .base_data = {
            /* The component is checkpoint ready */
            PRRTE_MCA_BASE_METADATA_PARAM_CHECKPOINT
        },
    }
};

static int rsh_component_register(void)
{
    prrte_mca_base_component_t *c = &prrte_plm_rsh_component.super.base_version;
    int var_id;

    prrte_plm_rsh_component.num_concurrent = 128;
    (void) prrte_mca_base_component_var_register (c, "num_concurrent",
                                            "How many plm_rsh_agent instances to invoke concurrently (must be > 0)",
                                            PRRTE_MCA_BASE_VAR_TYPE_INT, NULL, 0, 0,
                                            PRRTE_INFO_LVL_5,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.num_concurrent);

    prrte_plm_rsh_component.force_rsh = false;
    (void) prrte_mca_base_component_var_register (c, "force_rsh", "Force the launcher to always use rsh",
                                            PRRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                            PRRTE_INFO_LVL_2,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.force_rsh);
    prrte_plm_rsh_component.disable_qrsh = false;
    (void) prrte_mca_base_component_var_register (c, "disable_qrsh",
                                            "Disable the use of qrsh when under the Grid Engine parallel environment",
                                            PRRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                            PRRTE_INFO_LVL_2,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.disable_qrsh);

    prrte_plm_rsh_component.daemonize_qrsh = false;
    (void) prrte_mca_base_component_var_register (c, "daemonize_qrsh",
                                            "Daemonize the orted under the Grid Engine parallel environment",
                                            PRRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                            PRRTE_INFO_LVL_2,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.daemonize_qrsh);

    prrte_plm_rsh_component.disable_llspawn = false;
    (void) prrte_mca_base_component_var_register (c, "disable_llspawn",
                                            "Disable the use of llspawn when under the LoadLeveler environment",
                                            PRRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                            PRRTE_INFO_LVL_2,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.disable_llspawn);

    prrte_plm_rsh_component.daemonize_llspawn = false;
    (void) prrte_mca_base_component_var_register (c, "daemonize_llspawn",
                                            "Daemonize the orted when under the LoadLeveler environment",
                                            PRRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                            PRRTE_INFO_LVL_2,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.daemonize_llspawn);

    prrte_plm_rsh_component.priority = 10;
    (void) prrte_mca_base_component_var_register (c, "priority", "Priority of the rsh plm component",
                                            PRRTE_MCA_BASE_VAR_TYPE_INT, NULL, 0, 0,
                                            PRRTE_INFO_LVL_9,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.priority);

    prrte_plm_rsh_delay_string = NULL;
    (void) prrte_mca_base_component_var_register (c, "delay",
                                            "Delay between invocations of the remote agent (sec[:usec])",
                                            PRRTE_MCA_BASE_VAR_TYPE_STRING, NULL, 0, 0,
                                            PRRTE_INFO_LVL_4,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_delay_string);

    prrte_plm_rsh_component.no_tree_spawn = false;
    (void) prrte_mca_base_component_var_register (c, "no_tree_spawn",
                                            "If set to true, do not launch via a tree-based topology",
                                            PRRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                            PRRTE_INFO_LVL_5,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.no_tree_spawn);

    /* local rsh/ssh launch agent */
    prrte_plm_rsh_component.agent = "ssh : rsh";
    var_id = prrte_mca_base_component_var_register (c, "agent",
                                              "The command used to launch executables on remote nodes (typically either \"ssh\" or \"rsh\")",
                                              PRRTE_MCA_BASE_VAR_TYPE_STRING, NULL, 0, 0,
                                              PRRTE_INFO_LVL_2,
                                              PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                              &prrte_plm_rsh_component.agent);
    (void) prrte_mca_base_var_register_synonym (var_id, "prrte", "pls", NULL, "rsh_agent", PRRTE_MCA_BASE_VAR_SYN_FLAG_DEPRECATED);
    (void) prrte_mca_base_var_register_synonym (var_id, "prrte", "prrte", NULL, "rsh_agent", PRRTE_MCA_BASE_VAR_SYN_FLAG_DEPRECATED);
    agent_var_id = var_id;

    prrte_plm_rsh_component.assume_same_shell = true;
    var_id = prrte_mca_base_component_var_register (c, "assume_same_shell",
                                              "If set to true, assume that the shell on the remote node is the same as the shell on the local node.  Otherwise, probe for what the remote shell [default: 1]",
                                              PRRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                              PRRTE_INFO_LVL_2,
                                              PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                              &prrte_plm_rsh_component.assume_same_shell);
    /* XXX -- var_conversion -- Why does this component register prrte_assume_same_shell? Components should ONLY register THEIR OWN variables. */
    (void) prrte_mca_base_var_register_synonym (var_id, "prrte", "prrte", NULL, "assume_same_shell", 0);

    prrte_plm_rsh_component.pass_environ_mca_params = true;
    (void) prrte_mca_base_component_var_register (c, "pass_environ_mca_params",
                                            "If set to false, do not include mca params from the environment on the orted cmd line",
                                            PRRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                            PRRTE_INFO_LVL_2,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.pass_environ_mca_params);
    prrte_plm_rsh_component.ssh_args = NULL;
    (void) prrte_mca_base_component_var_register (c, "args",
                                            "Arguments to add to rsh/ssh",
                                            PRRTE_MCA_BASE_VAR_TYPE_STRING, NULL, 0, 0,
                                            PRRTE_INFO_LVL_2,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.ssh_args);

    prrte_plm_rsh_component.pass_libpath = NULL;
    (void) prrte_mca_base_component_var_register (c, "pass_libpath",
                                            "Prepend the specified library path to the remote shell's LD_LIBRARY_PATH",
                                            PRRTE_MCA_BASE_VAR_TYPE_STRING, NULL, 0, 0,
                                            PRRTE_INFO_LVL_2,
                                            PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                            &prrte_plm_rsh_component.pass_libpath);

    prrte_plm_rsh_component.chdir = NULL;
    (void) prrte_mca_base_component_var_register (c, "chdir",
                                                  "Change working directory after rsh/ssh, but before exec of prted",
                                                  PRRTE_MCA_BASE_VAR_TYPE_STRING, NULL, 0, 0,
                                                  PRRTE_INFO_LVL_2,
                                                  PRRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                                  &prrte_plm_rsh_component.chdir);
    return PRRTE_SUCCESS;
}

static int rsh_component_open(void)
{
    char *ctmp;

    /* initialize globals */
    prrte_plm_rsh_component.using_qrsh = false;
    prrte_plm_rsh_component.using_llspawn = false;
    prrte_plm_rsh_component.agent_argv = NULL;

    /* lookup parameters */
    if (prrte_plm_rsh_component.num_concurrent <= 0) {
        prrte_show_help("help-plm-rsh.txt", "concurrency-less-than-zero",
                       true, prrte_plm_rsh_component.num_concurrent);
        prrte_plm_rsh_component.num_concurrent = 1;
    }

    if (NULL != prrte_plm_rsh_delay_string) {
        prrte_plm_rsh_component.delay.tv_sec = strtol(prrte_plm_rsh_delay_string, &ctmp, 10);
        if (ctmp == prrte_plm_rsh_delay_string) {
            prrte_plm_rsh_component.delay.tv_sec = 0;
        }
        if (':' == ctmp[0]) {
            prrte_plm_rsh_component.delay.tv_nsec = 1000 * strtol (ctmp + 1, NULL, 10);
        }
    }

    return PRRTE_SUCCESS;
}


static int rsh_component_query(prrte_mca_base_module_t **module, int *priority)
{
    char *tmp;

    /* Check if we are under Grid Engine parallel environment by looking at several
     * environment variables.  If so, setup the path and argv[0].
     * Note that we allow the user to specify the launch agent
     * even if they are in a Grid Engine environment */
    int ret;
    prrte_mca_base_var_source_t source;
    ret = prrte_mca_base_var_get_value(agent_var_id, NULL, &source, NULL);
    if (PRRTE_SUCCESS != ret) {
        return ret;
    }
    if (PRRTE_MCA_BASE_VAR_SOURCE_DEFAULT != source) {
        /* if the user specified a launch agent, then
         * respect that request */
        goto lookup;
    }

    /* check for SGE */
    if (!prrte_plm_rsh_component.disable_qrsh &&
        NULL != getenv("SGE_ROOT") && NULL != getenv("ARC") &&
        NULL != getenv("PE_HOSTFILE") && NULL != getenv("JOB_ID")) {
        /* setup the search path for qrsh */
        prrte_asprintf(&tmp, "%s/bin/%s", getenv("SGE_ROOT"), getenv("ARC"));
        /* see if the agent is available */
        if (PRRTE_SUCCESS != rsh_launch_agent_lookup("qrsh", tmp)) {
            /* can't be SGE */
             prrte_output_verbose(1, prrte_plm_base_framework.framework_output,
                                "%s plm:rsh: unable to be used: SGE indicated but cannot find path "
                                "or execution permissions not set for launching agent qrsh",
                                PRRTE_NAME_PRINT(PRRTE_PROC_MY_NAME));
             free(tmp);
             *module = NULL;
             return PRRTE_ERROR;
        }
        prrte_plm_rsh_component.agent = tmp;
        prrte_plm_rsh_component.using_qrsh = true;
        goto success;
    }

    /* otherwise, check for LoadLeveler */
    if (!prrte_plm_rsh_component.disable_llspawn &&
        NULL != getenv("LOADL_STEP_ID")) {
        /* Search for llspawn in the users PATH */
        if (PRRTE_SUCCESS != rsh_launch_agent_lookup("llspawn", NULL)) {
             prrte_output_verbose(1, prrte_plm_base_framework.framework_output,
                                "%s plm:rsh: unable to be used: LoadLeveler "
                                "indicated but cannot find path or execution "
                                "permissions not set for launching agent llspawn",
                                PRRTE_NAME_PRINT(PRRTE_PROC_MY_NAME));
            *module = NULL;
            return PRRTE_ERROR;
        }
        prrte_plm_rsh_component.agent = strdup("llspawn");
        prrte_plm_rsh_component.using_llspawn = true;
        goto success;
    }

    /* if this isn't an Grid Engine or LoadLeveler environment, or
     * if the user specified a launch agent, look for it */
  lookup:
    if (PRRTE_SUCCESS != rsh_launch_agent_lookup(NULL, NULL)) {
        /* if the user specified an agent and we couldn't find it,
         * then we want to error out and not continue */
        if (NULL != prrte_plm_rsh_component.agent) {
            prrte_show_help("help-plm-rsh.txt", "agent-not-found", true,
                           prrte_plm_rsh_component.agent);
            PRRTE_FORCED_TERMINATE(PRRTE_ERR_NOT_FOUND);
            return PRRTE_ERR_FATAL;
        }
        /* this isn't an error - we just cannot be selected */
        PRRTE_OUTPUT_VERBOSE((1, prrte_plm_base_framework.framework_output,
                             "%s plm:rsh: unable to be used: cannot find path "
                             "for launching agent \"%s\"\n",
                             PRRTE_NAME_PRINT(PRRTE_PROC_MY_NAME),
                             prrte_plm_rsh_component.agent));
        *module = NULL;
        return PRRTE_ERROR;
    }

  success:
    /* we are good - make ourselves available */
    *priority = prrte_plm_rsh_component.priority;
    *module = (prrte_mca_base_module_t *) &prrte_plm_rsh_module;
    return PRRTE_SUCCESS;
}


static int rsh_component_close(void)
{
    return PRRTE_SUCCESS;
}

/*
 * Take a colon-delimited list of agents and locate the first one that
 * we are able to find in the PATH.  Split that one into argv and
 * return it.  If nothing found, then return NULL.
 */
char **prrte_plm_rsh_search(const char* agent_list, const char *path)
{
    int i, j;
    char *line, **lines;
    char **tokens, *tmp;
    char cwd[PRRTE_PATH_MAX];

    if (NULL == agent_list && NULL == prrte_plm_rsh_component.agent) {
        return NULL;
    }

    if (NULL == path) {
        getcwd(cwd, PRRTE_PATH_MAX);
    } else {
        prrte_string_copy(cwd, path, PRRTE_PATH_MAX);
    }
    if (NULL == agent_list) {
        lines = prrte_argv_split(prrte_plm_rsh_component.agent, ':');
    } else {
        lines = prrte_argv_split(agent_list, ':');
    }
    for (i = 0; NULL != lines[i]; ++i) {
        line = lines[i];

        /* Trim whitespace at the beginning and end of the line */
        for (j = 0; '\0' != line[j] && isspace(line[j]); ++line) {
            continue;
        }
        for (j = strlen(line) - 2; j > 0 && isspace(line[j]); ++j) {
            line[j] = '\0';
        }
        if (strlen(line) <= 0) {
            continue;
        }

        /* Split it */
        tokens = prrte_argv_split(line, ' ');

        /* Look for the first token in the PATH */
        tmp = prrte_path_findv(tokens[0], X_OK, environ, cwd);
        if (NULL != tmp) {
            free(tokens[0]);
            tokens[0] = tmp;
            prrte_argv_free(lines);
            return tokens;
        }

        /* Didn't find it */
        prrte_argv_free(tokens);
    }

    /* Doh -- didn't find anything */
    prrte_argv_free(lines);
    return NULL;
}

static int rsh_launch_agent_lookup(const char *agent_list, char *path)
{
    char *bname;
    int i;

    if (NULL == agent_list && NULL == prrte_plm_rsh_component.agent) {
        PRRTE_OUTPUT_VERBOSE((5, prrte_plm_base_framework.framework_output,
                             "%s plm:rsh_lookup on agent (null) path %s - No agent specified.",
                              PRRTE_NAME_PRINT(PRRTE_PROC_MY_NAME),
                             (NULL == path) ? "NULL" : path));
        return PRRTE_ERR_NOT_FOUND;
    }

    PRRTE_OUTPUT_VERBOSE((5, prrte_plm_base_framework.framework_output,
                         "%s plm:rsh_lookup on agent %s path %s",
                         PRRTE_NAME_PRINT(PRRTE_PROC_MY_NAME),
                         (NULL == agent_list) ? prrte_plm_rsh_component.agent : agent_list,
                         (NULL == path) ? "NULL" : path));
    if (NULL == (prrte_plm_rsh_component.agent_argv = prrte_plm_rsh_search(agent_list, path))) {
        return PRRTE_ERR_NOT_FOUND;
    }

    /* if we got here, then one of the given agents could be found - the
     * complete path is in the argv[0] position */
    prrte_plm_rsh_component.agent_path = strdup(prrte_plm_rsh_component.agent_argv[0]);
    bname = prrte_basename(prrte_plm_rsh_component.agent_argv[0]);
    if (NULL == bname) {
        return PRRTE_SUCCESS;
    }
    /* replace the initial position with the basename */
    free(prrte_plm_rsh_component.agent_argv[0]);
    prrte_plm_rsh_component.agent_argv[0] = bname;
    /* see if we need to add an xterm argument */
    if (0 == strcmp(bname, "ssh")) {
        /* if xterm option was given, add '-X', ensuring we don't do it twice */
        if (NULL != prrte_xterm) {
            prrte_argv_append_unique_nosize(&prrte_plm_rsh_component.agent_argv, "-X", false);
        } else if (0 >= prrte_output_get_verbosity(prrte_plm_base_framework.framework_output)) {
            /* if debug was not specified, and the user didn't explicitly
             * specify X11 forwarding/non-forwarding, add "-x" if it
             * isn't already there (check either case)
             */
            for (i = 1; NULL != prrte_plm_rsh_component.agent_argv[i]; ++i) {
                if (0 == strcasecmp("-x", prrte_plm_rsh_component.agent_argv[i])) {
                    break;
                }
            }
            if (NULL == prrte_plm_rsh_component.agent_argv[i]) {
                prrte_argv_append_nosize(&prrte_plm_rsh_component.agent_argv, "-x");
            }
        }
    }
    return PRRTE_SUCCESS;
}
