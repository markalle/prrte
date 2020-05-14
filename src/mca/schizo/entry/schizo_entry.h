/*
 * Copyright (c) 2016      Intel, Inc. All rights reserved.
 * Copyright (c) 2020 IBM Corporation. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef _MCA_SCHIZO_ENTRY_H_
#define _MCA_SCHIZO_ENTRY_H_

#include "prrte_config.h"

#include "src/include/types.h"

#include "src/mca/base/base.h"
#include "src/mca/schizo/schizo.h"


BEGIN_C_DECLS

PRRTE_MODULE_EXPORT extern prrte_schizo_base_component_t prrte_schizo_entry_component;
extern prrte_schizo_base_module_t prrte_schizo_entry_module;

END_C_DECLS

#endif /* MCA_SCHIZO_ENTRY_H_ */

