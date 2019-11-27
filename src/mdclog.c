/*
 * mdclog.c
 *
 *  Copyright (c) 2019 AT&T Intellectual Property.
 *  Copyright (c) 2018-2019 Nokia.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This source code is part of the near-RT RIC (RAN Intelligent Controller)
 *  platform project (RICP).
 *
 * API functions
 *
 */

#include "mdclog/mdclog.h"

#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <limits.h>

#include "private/mdc.h"
#include "private/system.h"
#include "private/json_format.h"

static mdclog_severity_t current_level = MDCLOG_DEBUG;
extern char *__progname;

/*
 * The configuration variable is library global
 * and is protected with read/write mutex
 */
static struct
{
    int   init_done;
    char *identity;
} mdclog_configuration;

static pthread_rwlock_t config_mutex = PTHREAD_RWLOCK_INITIALIZER;

typedef struct mdclog_attr
{
    char *identity;
} mdclog_attr_t;

static void init_library(mdclog_attr_t *attr)
{
    size_t      escaped_size;
    char       *identity;

    if (mdclog_configuration.init_done)
        return;
    mdclog_internal_init_mdc();
    pthread_rwlock_wrlock(&config_mutex);
    if (mdclog_configuration.identity)
    {
        free(mdclog_configuration.identity);
        mdclog_configuration.identity = NULL;
    }
    if (attr && attr->identity)
    {
        mdclog_configuration.identity = attr->identity;
        attr->identity = NULL;
    }
    else
    {
        /*
         * If strdup or malloc here fail then
         * process identity will be NULL
         */
        identity = strdup(__progname);
        if (identity)
        {
            if (mdclog_internal_contains_special_characters(identity))
            {
                escaped_size = mdclog_internal_escaped_size(identity);
                mdclog_configuration.identity = malloc(escaped_size);
                if (mdclog_configuration.identity)
                    mdclog_internal_escape(mdclog_configuration.identity, escaped_size, identity, NULL);
                free(identity);
            }
            else
                mdclog_configuration.identity = identity;
        }
    }
    mdclog_configuration.init_done = 1;
    pthread_rwlock_unlock(&config_mutex);
}

int mdclog_init(mdclog_attr_t *attr)
{
    // Set init_done to zero so that the init is done always
    // when this function is called.
    mdclog_configuration.init_done = 0;
    init_library(attr);
    return 0;
}

void mdclog_write(mdclog_severity_t severity, const char *format, ...)
{
    char           buffer[PIPE_BUF];
    struct timeval tv;
    int            len;
    va_list        va;

    if (severity > current_level)
        return;

    init_library(NULL);
    gettimeofday(&tv, NULL);

    va_start(va, format);
    pthread_rwlock_rdlock(&config_mutex);
    // Format function can use buffer size -1. The last byte is reserved for a newline char.
    len = mdclog_internal_format_to_json_str(buffer, sizeof(buffer) - 1, &tv, mdclog_configuration.identity,
            severity, mdclog_internal_get_first_mdc(), format, va);
    pthread_rwlock_unlock(&config_mutex);
    va_end(va);
    if (len > 0)
    {
        buffer[len] = '\n';
        TEMP_FAILURE_RETRY(SYSTEM(write(STDOUT_FILENO, buffer, len + 1)));
    }
}

void mdclog_level_set(mdclog_severity_t level)
{
    current_level = level;
}

mdclog_severity_t mdclog_level_get(void)
{
    return current_level;
}

int mdclog_attr_init(mdclog_attr_t **attr)
{
    if ((*attr = malloc(sizeof(mdclog_attr_t))) == NULL)
    {
        errno = ENOMEM;
        return -1;
    }
    memset(*attr, 0, sizeof(mdclog_attr_t));
    return 0;
}

void mdclog_attr_destroy(mdclog_attr_t *attr)
{
    if (attr)
    {
        if (attr->identity)
            free(attr->identity);
        free(attr);
    }
}

int mdclog_attr_set_ident(mdclog_attr_t *attr, const char *identity)
{
    if (!attr || !identity || mdclog_internal_contains_special_characters(identity) ||
        strlen(identity) > IDENTITY_MAX_LENGTH)
    {
        errno = EINVAL;
        return -1;
    }
    attr->identity = strdup(identity);
    return 0;
}

int mdclog_mdc_add(const char *key, const char *value)
{
    if (!key || !value || mdclog_internal_contains_special_characters(key))
    {
        errno = EINVAL;
        return -1;
    }
    init_library(NULL);
    return mdclog_internal_put_mdc(key, value);
}

char *mdclog_mdc_get(const char *key)
{
    mdc_t *mdc;

    if (!key)
        return NULL;
    init_library(NULL);

    mdc = mdclog_internal_search_mdc(key);
    if (mdc)
        return strdup(mdclog_internal_get_mdc_val(mdc));
    return NULL;
}

void mdclog_mdc_remove(const char *key)
{
    if (!key)
        return;
    init_library(NULL);
    mdclog_internal_rm_mdc(key);
}

void mdclog_mdc_clean(void)
{
    init_library(NULL);
    mdclog_internal_clean_mdclist();
}

void mdclog_lib_clean(void)
{
    if (mdclog_configuration.identity)
        free(mdclog_configuration.identity);
    mdclog_configuration.identity = NULL;
    mdclog_configuration.init_done = 0;
}
