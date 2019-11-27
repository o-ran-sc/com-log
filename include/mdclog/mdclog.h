/** @file include/mdclog/mdclog.h*/

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
 */

/**
 * @mainpage
 *
 * @section intro_sec Introduction
 *
 * Structured logging library with Mapped Diagnostic Context
 *
 * - Outputs the log entries to standard out in structured format, json currently
 * - Severity based filtering
 * - Supports Mapped Diagnostic Context (MDC)
 * - Thread safe
 *
 * Set MDC pairs are automatically added to log entries by the library.
 * MDC pairs are thread specific. A thread can see only MDCs it has set.
 *
 * Note!
 * The library API functions are thread safe but not async-signal safe (see signal-safety(7)).
 *
 * @section use_sec Taking into use
 *
 * From C/C++ code the library is taken in use by including the header file and linking to the library shared object.
 * C compilation flags and linker options are provided with a pkg-config file.
 *
 * @code
 * #include <mdclog/mdclog.h>
 * @endcode
 */

#ifndef INCLUDE_MDCLOG_H_
#define INCLUDE_MDCLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#if BUILDING_MDCLOG && HAVE_FUNC_ATTRIBUTE_VISIBILITY
#       define MDCLOG_EXPORT __attribute__ ((visibility("default")))
#   else
#       define MDCLOG_EXPORT
#endif

#include <stdarg.h>

/**
 * Severity level enumerations
 */
typedef enum {
    MDCLOG_ERR     = 1, //! Error level log entry
    MDCLOG_WARN    = 2, //! Warning level log entry
    MDCLOG_INFO    = 3, //! Info level log entry
    MDCLOG_DEBUG   = 4  //! Debug level log entry
} mdclog_severity_t;

/**
 * Maximum length of the identity attribute
 */
#define IDENTITY_MAX_LENGTH 100

/**
 * Log a message
 *
 * Logs the message with the given severity if it is equal or higher than the current
 * logging level.
 * If the length of the log entry after formatting exceeds PIPE_BUF bytes, the log
 * entry is truncated. All non-printable characters in the log message, as well as in
 * MDC values, are replaced with a space. In addition, backslash (\) and double
 * quotation marks (") are escaped for JSON formatting.
 *
 *
 * @param   severity   severity of the log message
 * @param   format     log message
 */
MDCLOG_EXPORT void mdclog_write(mdclog_severity_t severity, const char *format, ...)
    __attribute__ ((format (printf, 2, 3)));

/**
 * Set current logging level. Log messages with lower severity
 * will be filtered.
 *
 * @param  level   new logging level
 */
MDCLOG_EXPORT void mdclog_level_set(mdclog_severity_t level);

/**
 * Get current logging level.
 *
 * @return    current logging level
 */
MDCLOG_EXPORT mdclog_severity_t mdclog_level_get(void);

typedef struct mdclog_attr mdclog_attr_t;

/**
 * Initialize the log attributes.
 * The function creates and initializes the log attributes.
 * The attributes are used by mdclog_init() function.
 * User must call mdclog_attr_destroy() to destroy the attributes
 * after the mdclog_init() function is called.
 *
 * @param   attr    pointer to the function allocated attributes.
 *
 * @return   0 in case of success
 *           -1 in case of error. Errno ENOMEM is set if memory cannot be allocated
 *
 */
MDCLOG_EXPORT int mdclog_attr_init(mdclog_attr_t **attr);

/**
 * Destroy log attrubutes
 *
 * @param   attr     pointer to attributes, previously allocated with mdclog_attr_init()
 */
MDCLOG_EXPORT void mdclog_attr_destroy(mdclog_attr_t *attr);

/**
 * Set log attribute
 *
 * @param   attr        pointer to attributes, previously allocated with mdclog_attr_init()
 * @param   identity    program identity which is added to every log. Defaults to program name.
 *                      The identity must not contain any non-printable characters or
 *                      backslash (\) or double quotation mark (").
 *                      Maximum length IDENTITY_MAX_LENGTH.
 *
 * @return   0 in case of success,
 *          -1 in case of error. Errno EINVAL is set if attr or identity parameters are NULL,
 *             or if the identity value is too long or contains illegal characters.
 */
MDCLOG_EXPORT int mdclog_attr_set_ident(mdclog_attr_t *attr, const char *identity);

/**
 * Initialize mdclog library. Calling is optional.
 * If the mdclog_init() is not called or is called
 * with NULL parameter, the library uses default values.
 *
 * - The default identity is the program name.
 *
 * If called more than once the latest call takes effect.
 *
 * @param   attr     pointer to attributes. Can be NULL
 *
 * @return   0 in case of success, -1 in case of error. Errno is set in case of error
 */
MDCLOG_EXPORT int mdclog_init(mdclog_attr_t *attr);

/**
 * Add a thread specific MDC. If an MDC with the given key exists, it is replaced with the new one.
 * The function makes a copy of the given key and value.
 * An MDC can be removed with mdclog_mdc_remove() or mdclog_mdc_clean().
 *
 * @param    key      MDC key
 * @param    value    MDC value
 *                    The value must not contain any non-printable characters or backslash (\)
 *                    or double quotation mark (").
 *
 * @return   0 in case of success,
 *          -1 in case of error.
 *             Errno EINVAL is set if key or value are null or the value contains illegal characters.
 *             Errno ENOMEM is set if memory cannot be allocated.
 */
MDCLOG_EXPORT int mdclog_mdc_add(const char *key, const char *value);

/**
 * Get the thread's MDC value with the given key
 *
 * @param   key    MDC key
 *
 * @return  MDC value or null if MDC with the key is not set
 *          User must free the returned value with free(3)
 */
MDCLOG_EXPORT char *mdclog_mdc_get(const char *key);

/**
 * Remove thread's MDC with the given key.
 *
 * @param  key    MDC key
 */
MDCLOG_EXPORT void mdclog_mdc_remove(const char *key);

/**
 * Remove all MDCs of the thread.
 */
MDCLOG_EXPORT void mdclog_mdc_clean(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_MDCLOG_H_ */
