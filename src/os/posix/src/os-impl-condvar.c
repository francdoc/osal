/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 * \ingroup  posix
 * \author   joseph.p.hickey@nasa.gov
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-posix.h"
#include "os-shared-condvar.h"
#include "os-shared-idmap.h"
#include "os-impl-condvar.h"
#include "os-impl-mutex.h"

/* Tables where the OS object information is stored */
OS_impl_condvar_internal_record_t OS_impl_condvar_table[OS_MAX_CONDVARS];

/****************************************************************************************
                                  CONDVAR API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_Posix_CondVarAPI_Impl_Init(void)
{
    memset(OS_impl_condvar_table, 0, sizeof(OS_impl_condvar_table));
    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarCreate_Impl(const OS_object_token_t *token, osal_id_t mutex_id, uint32 options)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    (void)mutex_id;
    (void)options;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_cond_init(&impl->cv, NULL);
    if (status != 0)
    {
        OS_DEBUG("Error: CondVar could not be created. ID = %lu: %s\n",
                 OS_ObjectIdToInteger(OS_ObjectIdFromToken(token)), strerror(status));
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarDelete_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_cond_destroy(&impl->cv);
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarUnlock_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_condvar_internal_record_t *     condvar;
    OS_impl_mutex_internal_record_t *  mutex_impl;
    OS_object_token_t                  mutex_token;

    condvar = OS_OBJECT_TABLE_GET(OS_condvar_table, *token);
    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_MUTEX, condvar->bound_mutex, &mutex_token) !=
        OS_SUCCESS)
    {
        return OS_ERR_INVALID_ID;
    }

    mutex_impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, mutex_token);

    status = pthread_mutex_unlock(&mutex_impl->id);
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarLock_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_condvar_internal_record_t *     condvar;
    OS_impl_mutex_internal_record_t *  mutex_impl;
    OS_object_token_t                  mutex_token;

    condvar = OS_OBJECT_TABLE_GET(OS_condvar_table, *token);
    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_MUTEX, condvar->bound_mutex, &mutex_token) !=
        OS_SUCCESS)
    {
        return OS_ERR_INVALID_ID;
    }

    mutex_impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, mutex_token);

    status = pthread_mutex_lock(&mutex_impl->id);
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarSignal_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_cond_signal(&impl->cv);
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarBroadcast_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_cond_broadcast(&impl->cv);
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarWait_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_condvar_internal_record_t *     condvar;
    OS_impl_condvar_internal_record_t *impl;
    OS_impl_mutex_internal_record_t *  mutex_impl;
    OS_object_token_t                  mutex_token;

    condvar = OS_OBJECT_TABLE_GET(OS_condvar_table, *token);
    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_MUTEX, condvar->bound_mutex, &mutex_token) !=
        OS_SUCCESS)
    {
        return OS_ERR_INVALID_ID;
    }

    impl       = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);
    mutex_impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, mutex_token);

    status = pthread_cond_wait(&impl->cv, &mutex_impl->id);
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarTimedWait_Impl(const OS_object_token_t *token, const OS_time_t *abs_wakeup_time)
{
    struct timespec                    limit;
    int                                status;
    OS_condvar_internal_record_t *     condvar;
    OS_impl_condvar_internal_record_t *impl;
    OS_impl_mutex_internal_record_t *  mutex_impl;
    OS_object_token_t                  mutex_token;

    condvar = OS_OBJECT_TABLE_GET(OS_condvar_table, *token);
    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_MUTEX, condvar->bound_mutex, &mutex_token) !=
        OS_SUCCESS)
    {
        return OS_ERR_INVALID_ID;
    }

    impl       = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);
    mutex_impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, mutex_token);

    limit.tv_sec  = OS_TimeGetTotalSeconds(*abs_wakeup_time);
    limit.tv_nsec = OS_TimeGetNanosecondsPart(*abs_wakeup_time);

    status = pthread_cond_timedwait(&impl->cv, &mutex_impl->id, &limit);

    if (status == ETIMEDOUT)
    {
        return OS_ERROR_TIMEOUT;
    }
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarGetInfo_Impl(const OS_object_token_t *token, OS_condvar_prop_t *condvar_prop)
{
    return OS_SUCCESS;
}
