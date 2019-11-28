/*
 * json_format.h
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
 */


#ifndef INCLUDE_JSON_FORMAT_H_
#define INCLUDE_JSON_FORMAT_H_

#include <stddef.h>
#include <stdarg.h>
#include <sys/time.h>

#include "mdclog/mdclog.h"
#include "private/mdc.h"

#define MIN_BUFFER_LENGTH   1000

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Format a log entry into a json string
 *
 * @param   buffer     output: json string with the ending zero
 * @param   len        size of the buffer, including the ending zero
 * @param   timestamp  timestamp
 * @param   logger     name of the logger
 * @param   severity   severity of the log message
 * @param   mdc        MDC
 * @param   msg        log message
 * @param   va_list    variable length arguments list
 *
 * @return  in case of success: length of the output json string, excluding the ending zero
 *          in case of error: -1
 */

int mdclog_internal_format_to_json_str(char* buffer,
                       size_t len,
                       struct timeval* timestamp,
                       const char* logger,
                       mdclog_severity_t severity,
                       mdc_t* mdc,
                       const char* msg,
                       va_list arglist);

/**
 * Escape \ and " characters
 * If the escaped string does not fit to the buffer, it is cut.
 *
 * @param   buffer    output: escaped str
 * @param   len       size of the buffer, including the ending zero
 * @param   str       string to be escaped
 * @param   truncated output: was the escaped string truncated
 *
 * @return  length of the escaped string, excluding the ending zero
 */
size_t mdclog_internal_escape(char* buffer, size_t len, const char* str, int* truncated);

/**
 * De-escape the string and replace non-printable chars with a space.
 * If the de-escaped string does not fit to the buffer, it is cut.
 *
 * @param   buffer    output: de-escaped str
 * @param   len       size of the buffer, including the ending zero
 * @param   str       string to be de-escaped
 *
 * @return  in case of success: length of the output de-escaped string, excluding the ending zero
 *          in case of error: 0
 */
size_t mdclog_internal_deescape(char* buffer, size_t len, const char* str);

/**
 * Return the size of the buffer needed for the escaped string, including the ending zero
 *
 * @param   str       string to be escaped, zero ending
 *
 * @return  size of the buffer for escaping the given string
 */
size_t mdclog_internal_escaped_size(const char* str);

/**
 * Return 1, if the escape function would modify the given string, i.e.
 * if the given string contains non-printable characters or characters that would
 * be escaped.
 *
 * @param   str       string to be checked, zero ending
 *
 * @return  1 if the string contains such characters
 *          0 if the string does not contain such characters
 */
int mdclog_internal_contains_special_characters(const char* str);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_JSON_FORMAT_H_
