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
#include <stdint.h>
#include <sys/inotify.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>

#include "private/mdc.h"
#include "private/system.h"
#include "private/json_format.h"

static mdclog_severity_t current_level = MDCLOG_ERR;
extern char *__progname;

#define STR_BUFF 128
#define LOG_FILE_CONFIG_MAP "CONFIG_MAP_NAME"


/*
 * The configuration variable is library global
 * and is protected with read/write mutex
 */
static struct
{
    uint8_t  init_done;
    uint8_t  log_format_init_done;
    char    *identity;
} mdclog_configuration;

static pthread_rwlock_t config_mutex = PTHREAD_RWLOCK_INITIALIZER;

typedef struct mdclog_attr
{
    char *identity;
} mdclog_attr_t;

typedef enum log_format_fields {
    SYSTEM=0,
    HOST_NAME=1,
    SERVICE_NAME=2,
    CONTAINER_NAME=3,
    POD_NAME=4
} log_field_t;

const char *LOGFIELDS[5] = {"SYSTEM_NAME","HOST_NAME","SERVICE_NAME","CONTAINER_NAME","POD_NAME"};


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
    mdclog_configuration.log_format_init_done=0;
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
    mdclog_configuration.log_format_init_done = 0;
}

char *read_env_param(const char*envkey)
{
    if(envkey)
    {
        char *value = getenv(envkey);
        if(value)
            return strdup(value);
    }
    return NULL;
}

void  update_mdc_log_level_severity(char* log_level)
{
    mdclog_severity_t level = MDCLOG_ERR;

    if(log_level == NULL)
    {
        printf("### ERR ### Invalid Log-Level Configuration in ConfigMap, Default Log-Level Applied:   %d\n",level);
    }
    else if(strcasecmp(log_level,"ERR")==0)
    {
        level = MDCLOG_ERR;
    }
    else if(strcasecmp(log_level,"WARN")==0)
    {
        level = MDCLOG_WARN;
    }
    else if(strcasecmp(log_level,"INFO")==0)
    {
        level = MDCLOG_INFO;
    }
    else if(strcasecmp(log_level,"DEBUG")==0)
    {
        level = MDCLOG_DEBUG;
    }

    mdclog_level_set(level);
}

#ifdef UNITTEST
char* parse_file(char* filename)
#else
static char* parse_file(char* filename)
#endif
{
    char *token=NULL;
    char *search = ": ";
    char *string_match = "log-level";
    bool found = false;
    FILE *file = fopen ( filename, "r" );
    if ( file != NULL )
    {
        char line [ 128 ];
        while ( fgets ( line, sizeof line, file ) != NULL )
        {
            token = strtok(line, search);
            if(strcmp(token,string_match)==0)
            {
                found = true;
                token = strtok(NULL, search);
                token = strtok(token, "\n");//removing newline if any
                break;
            }
        }
        fclose ( file );
     }
     if(found)
         return(strdup(token));
     else
         return(NULL);
}

#ifdef UNITTEST
void * monitor_loglevel_change_handler(void* arg)
#else
static void * monitor_loglevel_change_handler(void* arg)
#endif
{
    char *fileName = (char*) arg;
    int ifd;                   // the inotify file des
    int wfd;                   // the watched file des
    ssize_t n = 0;
    char rbuf[4096];           // large read buffer as the event is var len
    fd_set fds;
    int res = 0;
    struct timeval timeout;
    char* dname=NULL;          // directory name
    char* bname = NULL;        // basename
    char* tok=NULL;
    char* log_level=NULL;

    dname = strdup( fileName); // defrock the file name into dir and basename
    if( (tok = strrchr( dname, '/' )) != NULL ) {
        *tok = '\0';
        bname = strdup( tok+1 );
    }


    ifd = inotify_init1( 0 ); // initialise watcher setting blocking read (no option)
    if( ifd < 0 ) {
        fprintf( stderr, "### ERR ### unable to initialise file watch %s\n", strerror( errno ) );
    } else {
        wfd = inotify_add_watch( ifd, dname, IN_MOVED_TO | IN_CLOSE_WRITE ); // we only care about close write changes

        if( wfd < 0 ) {
            fprintf( stderr, "### ERR ### unable to add watch on config file %s: %s\n", fileName, strerror( errno ) );
        } else {
            mdclog_configuration.log_format_init_done = 1;
#ifdef UNITTEST
            mdclog_configuration.log_format_init_done = 0;
#endif
            memset( &timeout, 0, sizeof(timeout) );
            while( mdclog_configuration.log_format_init_done ) {
                FD_ZERO (&fds);
                FD_SET (ifd, &fds);
                timeout.tv_sec=1;
                res = select (ifd + 1, &fds, NULL, NULL, &timeout);
                if(res)
                {
                    n = read( ifd, rbuf, sizeof( rbuf ) ); // read the event
                    if( n < 0  ) {
                        if( errno == EAGAIN ) {
                        } else {
                            fprintf( stderr, "### CRIT ### config listener read err: %s\n", strerror( errno ) );
                        }
                        continue;
                    }

                    //Retrieving Log Level from configmap by parsing configmap file
                    log_level = parse_file(fileName);
                    update_mdc_log_level_severity(log_level); //setting log level
                    free(log_level);
                }
            }
            inotify_rm_watch(ifd,wfd);
        }
        close(ifd);
    }
    free(bname);
    free(dname);
#ifdef UNITTEST
	return NULL;
#endif
    pthread_exit(NULL);
}

static int register_log_change_notify(const char *fileName)
{
    pthread_attr_t cb_attr;
    pthread_t tid;
    pthread_attr_init(&cb_attr);
    pthread_attr_setdetachstate(&cb_attr,PTHREAD_CREATE_DETACHED);
    return pthread_create(&tid, &cb_attr,&monitor_loglevel_change_handler,(void *)strdup(fileName));
}

#ifdef UNITTEST
int enable_log_change_notify(const char* fileName)
#else
static int enable_log_change_notify(const char* fileName)
#endif
{
    int ret = -1;
    struct stat fileInfo;
    if ( lstat(fileName,&fileInfo) == 0 )
    {
        ret = register_log_change_notify(fileName);
    }
    return ret;
}

static int update_mdclog_array(const char* key, const char* value)
{
    char *old_value = mdclog_mdc_get(key);
    if(old_value)
    {
        mdclog_mdc_remove(key);
        free(old_value);
    }
    return  mdclog_mdc_add(key,value);

}

int mdclog_format_initialize(const int logfile_monitor)
{
    int ret = 0;
    char* log_level_init=NULL;
    if( 0 == mdclog_configuration.log_format_init_done)
    {
        char buff_string[STR_BUFF]={'\0'};
        for(uint8_t i=0; ((0 == ret) && (i < sizeof(LOGFIELDS)/sizeof(char*))); i++)
        {
            char *field_value = read_env_param(LOGFIELDS[i]);
            if (field_value) {
                ret = update_mdclog_array(LOGFIELDS[i],field_value);
                free(field_value);
            } else {
                mdclog_mdc_add(LOGFIELDS[i],"");
            }
        }
        if(0 == ret)
        {
            memset(buff_string,'\0',STR_BUFF);
            snprintf(buff_string,STR_BUFF,"%d",getpid());
            char *logFile_Name = read_env_param(LOG_FILE_CONFIG_MAP);
            ret = update_mdclog_array("PID",buff_string);
            if(logFile_Name)
            {
                log_level_init = parse_file(logFile_Name);
                update_mdc_log_level_severity(log_level_init); //setting log level
                free(log_level_init);

            }
            if( (logfile_monitor != 0)  && (0 == ret) && logFile_Name)
            {
                ret = enable_log_change_notify(logFile_Name);
            }
            if(logFile_Name) {
                free(logFile_Name);
                logFile_Name = NULL;
            }
        }
    }
    return ret;
}
