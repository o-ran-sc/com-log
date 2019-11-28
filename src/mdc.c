/*
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
 * Internal MDC manipulation functions
 * The MDC values are stored to a thread specific list.
 * The list pointer is stored to a memory area got from pthread key functions.
 * The thread MDC list is destroyed when the thread exits.
 *
 */
#include "private/mdc.h"
#include "private/json_format.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct mdc
{
    struct mdc *prev;
    struct mdc *next;
    char       *key;
    char       *value;
};

struct mdclist
{
    struct mdc *head;
};

static pthread_key_t  mdcpthreadkey;
static pthread_once_t mdckey_once = PTHREAD_ONCE_INIT;

static struct mdc *alloc_entry(void)
{
    struct mdc *mdc = malloc(sizeof(*mdc));

    if (mdc)
        memset(mdc, 0, sizeof(*mdc));
    return mdc;
}

static void add_to_list(struct mdc *mdc, struct mdclist *list)
{
    if (list->head)
    {
        mdc->next = list->head;
        list->head->prev = mdc;
    }
    list->head = mdc;
}

static struct mdc *list_head(struct mdclist *list)
{
    return list->head;
}

static void mdc_destroy(struct mdc *mdc)
{
    if (mdc->key)
        free(mdc->key);
    if (mdc->value)
        free(mdc->value);
    free(mdc);
}

static void rm_from_list(struct mdc *mdc, struct mdclist *list)
{

    if (mdc == list->head)
        list->head = mdc->next;
    if (mdc->prev)
        mdc->prev->next = mdc->next;
    if (mdc->next)
        mdc->next->prev = mdc->prev;
    mdc_destroy(mdc);
}

static void empty_list(struct mdclist *list)
{
    struct mdc *mdc;

    if (!list)
        return;
    mdc = list_head(list);
    while (mdc)
    {
        struct mdc *next = mdc->next;
        rm_from_list(mdc, list);
        mdc = next;
    }
}

static void destroy_list(void *ptr)
{
    struct mdclist *list = (struct mdclist*)ptr;

    empty_list(list);
    free(list);
    pthread_setspecific(mdcpthreadkey, NULL);
}

static void create_pthread_key(void)
{
    int ec = pthread_key_create(&mdcpthreadkey, destroy_list);

    if (ec)
    {
        fprintf(stderr, "Cannot create pthread key: %s", strerror(errno));
        abort();
    }
}

int mdclog_internal_init_mdc(void)
{
    int ec = pthread_once(&mdckey_once, create_pthread_key);

    if (ec)
    {
        errno = ec;
        return -1;
    }
    return 0;
}

static struct mdclist *get_list(void)
{
    struct mdclist *list;
    int             ec;

    list = (struct mdclist*)pthread_getspecific(mdcpthreadkey);
    if (!list)
    {
        list = malloc(sizeof(*list));
        if (!list)
        {
            errno = ENOMEM;
            return NULL;
        }
        memset(list, 0, sizeof(*list));
        ec = pthread_setspecific(mdcpthreadkey, list);
        if (ec)
        {
            errno = ec;
            return NULL;
        }
    }
    return list;
}

static char* escape_and_copy(const char* str)
{
    size_t len = mdclog_internal_escaped_size(str);
    char  *buf = malloc(len);

    if (buf)
    {
        mdclog_internal_escape(buf, len, str, NULL);
        return buf;
    }
    else
        return NULL;
}

int mdclog_internal_put_mdc(const char *key, const char *value)
{
    struct mdc     *mdc;
    struct mdclist *list = get_list();

    if (!list)
        return -1;

    mdc = mdclog_internal_search_mdc(key);
    if (!mdc)
    {
        mdc = alloc_entry();
        if (!mdc)
        {
            errno = ENOMEM;
            return -1;
        }
        mdc->key = strdup(key);
        add_to_list(mdc, list);
    } else
        free(mdc->value);

    mdc->value = escape_and_copy(value);
    if (mdc->key && mdc->value)
        return 0;

    mdc_destroy(mdc);
    errno = ENOMEM;
    return -1;
}

mdc_t *mdclog_internal_search_mdc(const char *key)
{
    struct mdc     *mdc;
    struct mdclist *list = get_list();

    if (!list)
        return NULL;

    mdc = list_head(list);
    while (mdc)
    {
        if (!strcmp(key, mdc->key))
            return mdc;
        mdc = mdc->next;
    }
    return NULL;
}

void mdclog_internal_rm_mdc(const char *key)
{
    struct mdc *mdc = mdclog_internal_search_mdc(key);

    if (mdc)
        rm_from_list(mdc, get_list());
}

mdc_t *mdclog_internal_get_first_mdc()
{
    struct mdclist *list = get_list();

    return list ? list_head(list) : NULL;
}

mdc_t *mdclog_internal_get_next_mdc(mdc_t *mdc)
{
    return mdc ? mdc->next : NULL;
}

const char *mdclog_internal_get_mdc_val(mdc_t *mdc)
{
    return mdc ? mdc->value : NULL;
}

const char *mdclog_internal_get_mdc_key(mdc_t *mdc)
{
    return mdc ? mdc->key : NULL;
}

void mdclog_internal_clean_mdclist(void)
{
    empty_list(get_list());
}

void mdclog_internal_destroy_mdclist(void)
{
    destroy_list(get_list());
}
