/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2013      Los Alamos National Security, LLC.  All rights reserved.
 * Copyright (c) 2017      Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2017-2019 Intel, Inc.  All rights reserved.
 * Copyright (c) 2019      Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/**
 * @file
 *
 * Resource Mapping
 */
#ifndef PRRTE_RMAPS_MINDIST_H
#define PRRTE_RMAPS_MINDIST_H

#include "prrte_config.h"

#include "src/hwloc/hwloc-internal.h"
#include "src/class/prrte_list.h"

#include "src/mca/rmaps/rmaps.h"

BEGIN_C_DECLS

PRRTE_MODULE_EXPORT extern prrte_rmaps_base_component_t prrte_rmaps_mindist_component;
extern prrte_rmaps_base_module_t prrte_rmaps_mindist_module;

END_C_DECLS

#endif
