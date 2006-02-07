/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "orte_config.h"
#include "orte/include/orte_constants.h"
#include "orte/include/orte_types.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "opal/runtime/opal.h"
#include "orte/util/proc_info.h"
#include "orte/mca/errmgr/errmgr.h"

#include "orte/dss/dss.h"

#define NUM_ITERS 3
#define NUM_ELEMS 10

static bool test1(void);        /* verify different buffer inits */
static bool test2(void);        /* verify int16 */
static bool test3(void);      /* verify int */
static bool test4(void);        /* verify int32 */
static bool test5(void);      /* verify int64 */
static bool test6(void);        /* verify string */
static bool test7(void);        /* verify BOOL */
static bool test8(void);        /* verify OBJECT */
static bool test9(void);        /* verify composite (multiple types and element counts) */
static bool test10(void);        /* verify KEYVAL */
static bool test11(void);        /* verify size_t */
static bool test12(void);        /* verify pid_t */
static bool test13(void);        /* verify pid_t */

FILE *test_out;


int main (int argc, char* argv[])
{
    int ret;

    opal_init();

    /* register handler for errnum -> string converstion */
    opal_error_register("ORTE", ORTE_ERR_BASE, ORTE_ERR_MAX, orte_err2str);

    test_out = stderr;

    /* Ensure the process info structure is instantiated and initialized */
    if (ORTE_SUCCESS != (ret = orte_proc_info())) {
        return ret;
    }

    orte_process_info.seed = true;
    orte_process_info.my_name = (orte_process_name_t*)malloc(sizeof(orte_process_name_t));
    orte_process_info.my_name->cellid = 0;
    orte_process_info.my_name->jobid = 0;
    orte_process_info.my_name->vpid = 0;


    /* open the dss */
    if (ORTE_SUCCESS == orte_dss_open()) {
        fprintf(test_out, "DSS started\n");
    } else {
        fprintf(test_out, "DSS could not start\n");
        exit (1);
    }

    /* run the tests */

    fprintf(test_out, "executing test1\n");
    if (test1()) {
        fprintf(test_out, "Test1 succeeded\n");
    }
    else {
      fprintf(test_out, "Test1 failed\n");
    }

    fprintf(test_out, "executing test2\n");
    if (test2()) {
        fprintf(test_out, "Test2 succeeded\n");
    }
    else {
      fprintf(test_out, "Test2 failed\n");
    }

    fprintf(test_out, "executing test3\n");
    if (test3()) {
        fprintf(test_out, "Test3 succeeded\n");
    }
    else {
      fprintf(test_out, "Test3 failed\n");
    }

    fprintf(test_out, "executing test4\n");
    if (test4()) {
        fprintf(test_out, "Test4 succeeded\n");
    }
    else {
      fprintf(test_out, "Test4 failed\n");
    }

    fprintf(test_out, "executing test5\n");
    if (test5()) {
        fprintf(test_out, "Test5 succeeded\n");
    }
    else {
      fprintf(test_out, "Test5 failed\n");
    }

    fprintf(test_out, "executing test6\n");
    if (test6()) {
        fprintf(test_out, "Test6 succeeded\n");
    }
    else {
      fprintf(test_out, "Test6 failed\n");
    }

    fprintf(test_out, "executing test7\n");
    if (test7()) {
        fprintf(test_out, "Test7 succeeded\n");
    }
    else {
      fprintf(test_out, "Test7 failed\n");
    }

    fprintf(test_out, "executing test8\n");
    if (test8()) {
        fprintf(test_out, "Test8 succeeded\n");
    }
    else {
      fprintf(test_out, "Test8 failed\n");
    }

    fprintf(test_out, "executing test9\n");
    if (test9()) {
        fprintf(test_out, "Test9 succeeded\n");
    }
    else {
      fprintf(test_out, "orte_dss test9 failed\n");
    }

    fprintf(test_out, "executing test10\n");
    if (test10()) {
        fprintf(test_out, "Test10 succeeded\n");
    }
    else {
      fprintf(test_out, "orte_dss test10 failed\n");
    }

    fprintf(test_out, "executing test11\n");
    if (test11()) {
        fprintf(test_out, "Test11 succeeded\n");
    }
    else {
      fprintf(test_out, "orte_dss test11 failed\n");
    }

    fprintf(test_out, "executing test12\n");
    if (test12()) {
        fprintf(test_out, "Test12 succeeded\n");
    }
    else {
      fprintf(test_out, "orte_dss test12 failed\n");
    }

    fprintf(test_out, "executing test13\n");
    if (test13()) {
        fprintf(test_out, "Test13 succeeded\n");
    }
    else {
        fprintf(test_out, "orte_dss test13 failed\n");
    }

    fclose(test_out);

    orte_dss_close();

    opal_finalize();

    return(0);
}

/*
 * INT8
 */
static bool test1(void)
{
    int8_t v1, v2=100;
    uint8_t u1, u2=150;
    orte_data_type_t type=ORTE_INT8, utype=ORTE_UINT8;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value2 greater\n");
        return(false);
    }

    u1 = u2;
    if (ORTE_EQUAL != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned equality\n");
        return(false);
    }

    u1 = u2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value1 greater\n");
        return(false);
    }

    u1 = u2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value2 greater\n");
        return(false);
    }

    return (true);
}

/*
 * INT16
 */
static bool test2(void)
{
    int16_t v1, v2=100;
    uint16_t u1, u2=150;
    orte_data_type_t type=ORTE_INT16, utype=ORTE_UINT16;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value2 greater\n");
        return(false);
    }

    u1 = u2;
    if (ORTE_EQUAL != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned equality\n");
        return(false);
    }

    u1 = u2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value1 greater\n");
        return(false);
    }

    u1 = u2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value2 greater\n");
        return(false);
    }

    return (true);
}

/*
 * INT32
 */
static bool test3(void)
{
    int32_t v1, v2=100;
    uint32_t u1, u2=150;
    orte_data_type_t type=ORTE_INT32, utype=ORTE_UINT32;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value2 greater\n");
        return(false);
    }

    u1 = u2;
    if (ORTE_EQUAL != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned equality\n");
        return(false);
    }

    u1 = u2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value1 greater\n");
        return(false);
    }

    u1 = u2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value2 greater\n");
        return(false);
    }

    return (true);
}

/*
 * INT64
 */
static bool test4(void)
{
    int64_t v1, v2=100;
    uint64_t u1, u2=150;
    orte_data_type_t type=ORTE_INT64, utype=ORTE_UINT64;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value2 greater\n");
        return(false);
    }

    u1 = u2;
    if (ORTE_EQUAL != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned equality\n");
        return(false);
    }

    u1 = u2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value1 greater\n");
        return(false);
    }

    u1 = u2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value2 greater\n");
        return(false);
    }

    return (true);
}

/*
 * INT
 */
static bool test5(void)
{
    int v1, v2=100;
    uint u1, u2=150;
    orte_data_type_t type=ORTE_INT, utype=ORTE_UINT;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for signed value2 greater\n");
        return(false);
    }

    u1 = u2;
    if (ORTE_EQUAL != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned equality\n");
        return(false);
    }

    u1 = u2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value1 greater\n");
        return(false);
    }

    u1 = u2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&u1, &u2, utype)) {
        fprintf(test_out, "orte_dss.compare failed for unsigned value2 greater\n");
        return(false);
    }

    return (true);
}

/*
 * STRING
 */
static bool test6(void)
{
    char *string1="This is a short string";
    char *string2="This is a longer string";

    if (ORTE_EQUAL != orte_dss.compare(string1, string1, ORTE_STRING)) {
        fprintf(test_out, "orte_dss.compare failed for equality\n");
        return(false);
    }

    if (ORTE_VALUE2_GREATER != orte_dss.compare(string1, string2, ORTE_STRING)) {
        fprintf(test_out, "orte_dss.compare failed for value2 greater\n");
        return(false);
    }

    if (ORTE_VALUE1_GREATER != orte_dss.compare(string2, string1, ORTE_STRING)) {
        fprintf(test_out, "orte_dss.compare failed for value1 greater\n");
        return(false);
    }

    return (true);
}

/*
 * BOOL
 */
static bool test7(void)
{
    bool v1, v2=true;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, ORTE_BOOL)) {
        fprintf(test_out, "orte_dss.compare failed for equality\n");
        return(false);
    }

    v1 = true;
    v2 = false;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, ORTE_BOOL)) {
        fprintf(test_out, "orte_dss.compare failed for value1 greater\n");
        return(false);
    }

    v1 = false;
    v2 = true;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, ORTE_BOOL)) {
        fprintf(test_out, "orte_dss.compare failed for value2 greater\n");
        return(false);
    }

    return (true);
}


/*
 * SIZE
 */
static bool test8(void)
{
    size_t v1, v2=100;
    orte_data_type_t type=ORTE_SIZE;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value2 greater\n");
        return(false);
    }

    return (true);
}

/*
 * PID
 */
static bool test9(void)
{
    pid_t v1, v2=100;
    orte_data_type_t type=ORTE_PID;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value2 greater\n");
        return(false);
    }

    return (true);
}

/*
 * DAEMON CMD
 */
static bool test10(void)
{
    orte_daemon_cmd_flag_t v1, v2=100;
    orte_data_type_t type=ORTE_DAEMON_CMD;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value2 greater\n");
        return(false);
    }

    return (true);
}

/*
 * DATA TYPE
 */
static bool test11(void)
{
    orte_data_type_t v1, v2=100;
    orte_data_type_t type=ORTE_DATA_TYPE;

    v1 = v2;
    if (ORTE_EQUAL != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for equality\n");
        return(false);
    }

    v1 = v2 + 1;
    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value1 greater\n");
        return(false);
    }

    v1 = v2 - 1;
    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value2 greater\n");
        return(false);
    }

    return (true);
}

/**
 * ORTE_BYTE_OBJECT
 */

static bool test12(void)
{
    size_t i;
    orte_byte_object_t v1, v2;
    orte_data_type_t type=ORTE_BYTE_OBJECT;

    v1.size = 20;
    v1.bytes = (uint8_t*)malloc(v1.size);
    for (i=0; i<v1.size; i++) v1.bytes[i] = i;

    v2.size = v1.size;
    v2.bytes = (uint8_t*)malloc(v2.size);
    for (i=0; i<v2.size; i++) v2.bytes[i] = v1.bytes[i];
    v2.bytes[(v2.size)/2] += 1;


    if (ORTE_EQUAL != orte_dss.compare(&v1, &v1, type)) {
        fprintf(test_out, "orte_dss.compare failed for equality\n");
        return(false);
    }

    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v2, &v1, type)) {
        fprintf(test_out, "orte_dss.compare failed for value1 greater\n");
        return(false);
    }

    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value2 greater\n");
        return(false);
    }

    return (true);
}


/* ORTE_DATA_VALUE */
static bool test13(void)
{
    int dat1=100, dat2=200;
    orte_data_value_t v1, v2;
    orte_data_type_t type=ORTE_DATA_VALUE;

    v1.type = ORTE_INT;
    v1.data = &dat1;

    v2.type = ORTE_INT;
    v2.data = &dat2;

    if (ORTE_EQUAL != orte_dss.compare(&v1, &v1, type)) {
        fprintf(test_out, "orte_dss.compare failed for equality\n");
        return(false);
    }

    if (ORTE_VALUE1_GREATER != orte_dss.compare(&v2, &v1, type)) {
        fprintf(test_out, "orte_dss.compare failed for value1 greater\n");
        return(false);
    }

    if (ORTE_VALUE2_GREATER != orte_dss.compare(&v1, &v2, type)) {
        fprintf(test_out, "orte_dss.compare failed for value2 greater\n");
        return(false);
    }

    return (true);
}

