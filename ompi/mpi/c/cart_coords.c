/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2008 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007      Cisco Systems, Inc.  All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */
#include "ompi_config.h"
#include <stdio.h>

#include "ompi/mpi/c/bindings.h"
#include "ompi/mca/topo/topo.h"
#include "ompi/group/group.h"
#include "ompi/memchecker.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILING_DEFINES
#pragma weak MPI_Cart_coords = PMPI_Cart_coords
#endif

#if OMPI_PROFILING_DEFINES
#include "ompi/mpi/c/profile/defines.h"
#endif

static const char FUNC_NAME[] = "MPI_Cart_coords";


int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int *coords) 
{
    int err;
    mca_topo_base_module_cart_coords_fn_t func;
    MEMCHECKER(
        memchecker_comm(comm);
    );
    OPAL_CR_TEST_CHECKPOINT_READY();

    OPAL_CR_TEST_CHECKPOINT_READY();

    /* check the arguments */
    if (MPI_PARAM_CHECK) {
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if (ompi_comm_invalid(comm)) {
            return OMPI_ERRHANDLER_INVOKE (MPI_COMM_WORLD, MPI_ERR_COMM,
                                          FUNC_NAME);
        }
        if (OMPI_COMM_IS_INTER(comm)) { 
            return OMPI_ERRHANDLER_INVOKE (comm, MPI_ERR_COMM,
                                          FUNC_NAME);
        }
        if (!OMPI_COMM_IS_CART(comm)) {
            return OMPI_ERRHANDLER_INVOKE (comm, MPI_ERR_TOPOLOGY,
                                          FUNC_NAME);
        }
        if ( (0 > maxdims) || ((0 < maxdims) && (NULL == coords))) {
            return OMPI_ERRHANDLER_INVOKE (comm, MPI_ERR_ARG,
                                          FUNC_NAME);
        }
        if ((0 > rank) || (rank > ompi_group_size(comm->c_local_group))) {
            return OMPI_ERRHANDLER_INVOKE (comm, MPI_ERR_RANK,
                                          FUNC_NAME);
        }
    }

    /* get the function pointer on this communicator */
    func = comm->c_topo->topo_cart_coords;

    /* call the function */
    if ( MPI_SUCCESS != 
            (err = func(comm, rank, maxdims, coords))) {
        return OMPI_ERRHANDLER_INVOKE(comm, err, FUNC_NAME);
    }

    /* all done */
    return MPI_SUCCESS;
}
