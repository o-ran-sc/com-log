/*
 * json_format.c
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
 * This module implements formatting of the log message into a json string.
 *
 * The following is an example of a json-formatted log. The formatted log
 * does not contain any line feeds. The following example log has been
 * divided into several lines for readability.
 *
 * {"timestamp":1550045469,"severity":"INFO","logger":"applicationABC",
 *  "mdc":{"key1":"value1","key2":"value2"},
 *  "message": "This is an example log"}
 *
 */

#include "private/json_format.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "private/system.h"

#define TIMESTAMP_KEY "ts"
#define SEVERITY_KEY  "crit"
#define LOGGER_KEY    "id"
#define MESSAGE_KEY   "msg"
#define MDC_KEY       "mdc"

#define SEVERITY_ERR_VAL    "ERROR"
#define SEVERITY_WARN_VAL   "WARNING"
#define SEVERITY_INFO_VAL   "INFO"
#define SEVERITY_DEBUG_VAL  "DEBUG"

#define TRUNCATED           "[truncated]"
#define MINIMUM_MESSAGE     "\"" MESSAGE_KEY "\":\"" TRUNCATED "\""
#define REPLACEMENT_CHAR    ' '

size_t mdclog_internal_escape(char* buffer, size_t len, const char* str, int* truncated)
{
    size_t s, d;
    size_t ret;
    int    tmp_truncated = 0;

    // escape \ and " characters
    for (s = 0, d = 0; d < len - 1 && str[s] != '\0'; s++, d++)
    {
        if (str[s] == '\\' || str[s] == '"' )
            buffer[d++] = '\\';
        if (isprint(str[s]))
            buffer[d] = str[s];
        else
            buffer[d] = REPLACEMENT_CHAR;
    }
    if (d == len)
    {
        // last char was escaped, but it must be truncated
        buffer[len - 2] = '\0';
        tmp_truncated = 1;
        ret = len - 2;
    }
    else
    {
        buffer[d] = '\0';
        if (str[s] != '\0')
            tmp_truncated = 1;
        ret = d;
    }
    if (truncated)
        *truncated = tmp_truncated;
    return ret;
}

size_t mdclog_internal_deescape(char* buffer, size_t len, const char* str)
{
    size_t s, d;

    for (s = 0, d = 0; d < len - 1 && str[s] != '\0'; s++, d++)
    {
        if (str[s] == '\\')
        {
            s++;
            if (str[s] != '\\' && str[s] != '"')
            {
                buffer[0] = '\0';
                return 0;
            }
        }
        buffer[d] = str[s];
    }
    buffer[d] = '\0';
    return d;
}

size_t mdclog_internal_escaped_size(const char* str)
{
    size_t i, len;

    for (i = 0, len = 0; str[i] != '\0'; i++, len++)
    {
        if (str[i] == '\\' || str[i] == '"')
            len++;
    }
    return len + 1;
}

int mdclog_internal_contains_special_characters(const char* str)
{
    int i;

    for (i = 0; str[i] != '\0'; i++)
        if (!isprint(str[i]) || str[i] == '\\' || str[i] == '"')
            return 1;
    return 0;
}

STATIC size_t format_timestamp(char* buffer, size_t len, struct timeval* tv)
{
    int                ret;
    unsigned long long timestamp =
        (unsigned long long)(tv->tv_sec) * 1000 +
        (unsigned long long)(tv->tv_usec) / 1000;

    ret = snprintf(buffer, len, "\"%s\":%llu", TIMESTAMP_KEY, timestamp);
    if (ret < 0 || (size_t)ret >= len)
    {
        buffer[0] = '\0';
        return 0U;
    }
    else
        return (size_t)ret;
}

STATIC size_t format_severity(char* buffer, size_t len, mdclog_severity_t severity)
{
    char* severity_val;
    int   ret;

    switch(severity)
    {
        case MDCLOG_ERR:    severity_val = SEVERITY_ERR_VAL; break;
        case MDCLOG_WARN:   severity_val = SEVERITY_WARN_VAL; break;
        case MDCLOG_INFO:   severity_val = SEVERITY_INFO_VAL; break;
        case MDCLOG_DEBUG:  severity_val = SEVERITY_DEBUG_VAL; break;
        default:
            buffer[0] = '\0';
            return 0;
    }

    ret = snprintf(buffer, len, "\"%s\":\"%s\"", SEVERITY_KEY, severity_val);
    if (ret < 0 || (size_t)ret >= len)
    {
        buffer[0] = '\0';
        return 0U;
    }
    return (size_t)ret;
}

STATIC size_t format_identity(char* buffer, size_t len, const char* identity)
{
    int ret;

    ret = snprintf(buffer, len, "\"%s\":\"%.*s\"", LOGGER_KEY, IDENTITY_MAX_LENGTH, identity ? identity : "(null)");
    if (ret < 0 || (size_t)ret >= len)
    {
        buffer[0] = '\0';
        return 0U;
    }
    return (size_t)ret;
}

STATIC size_t format_message(char* buffer, size_t len, const char* msg, va_list arglist)
{
    size_t msg_start;
    size_t msg_len;
    size_t escaped_msg_len;
    size_t total_len;
    int    truncated = 0;
    int    escape_truncated;
    size_t i;
    char*  tmp_buf;

    if ( len <= strlen(MINIMUM_MESSAGE))
    {
        buffer[0] = '\0';
        return 0;
    }

    msg_start = (size_t)sprintf(buffer, "\"%s\":\"", MESSAGE_KEY);

    tmp_buf = (char*)malloc(len - msg_start);
    if (!tmp_buf)
    {
        buffer[0] = '\0';
        return 0;
    }

    msg_len = vsnprintf(tmp_buf, len - msg_start, msg, arglist);
    if (msg_len + 1 >= len - msg_start)                            // +1 for the " character
        truncated = 1;

    escaped_msg_len = mdclog_internal_escape(&buffer[msg_start], len - msg_start - 1,
            tmp_buf, &escape_truncated);  // -1 for the " character
    if (escape_truncated)
        truncated = 1;
    free(tmp_buf);

    total_len = msg_start + escaped_msg_len;

    if (truncated)
    {
        // find the correct place to add the TRUNCATED text
        // do not cut between the escape char and the escaped char
        for (i = total_len; i + strlen(TRUNCATED) >= len - 1; i--)
        {
            if (buffer[i - 1] == '\\' || buffer[i - 1] == '"')
                i--;
        }
        total_len = i + sprintf(&buffer[i], "%s\"", TRUNCATED);
    }
    else
        total_len += sprintf(&buffer[total_len], "\"");

    return total_len;
}

STATIC size_t format_mdc(char* buffer, size_t len, mdc_t* mdc)
{
    int ret;
    int offset = 0;
    int mdc_count;

    ret = snprintf(buffer, len, "\"%s\":{", MDC_KEY);
    if (ret < 0 || (size_t)ret + 1 >= len)  // +1 for the } character
    {
        buffer[0] = '\0';
        return 0U;
    }

    offset += ret;
    for (mdc_count=0 ; mdc; (mdc = mdclog_internal_get_next_mdc(mdc)), mdc_count++)
    {
        ret = snprintf(&buffer[offset], len-offset, "\"%s\":\"%s\",", mdclog_internal_get_mdc_key(mdc),
                mdclog_internal_get_mdc_val(mdc));
        if (ret < 0 || (size_t)ret >= len-offset)
            break;

        offset += ret;
    }
    // remove the last comma
    if (mdc_count > 0)
        offset--;
    buffer[offset++] = '}';
    buffer[offset] = '\0';

    return (size_t)offset;
}


STATIC int format_log_entry(char* buffer,
                       size_t len,
                       struct timeval* timestamp,
                       const char* identity,
                       mdclog_severity_t severity,
                       mdc_t* mdc,
                       const char* msg,
                       va_list arglist)
{
    int    offset = 0;
    size_t ret;

    buffer[offset++] = '{';
    ret = format_timestamp(&buffer[offset], len - offset - strlen(MINIMUM_MESSAGE) - 1, timestamp);
    if (ret > 0)
    {
        offset += ret;
        buffer[offset++] = ',';
    }
    ret = format_severity(&buffer[offset], len - offset - strlen(MINIMUM_MESSAGE) - 1, severity);
    if (ret > 0)
    {
        offset += ret;
        buffer[offset++] = ',';
    }
    ret = format_identity(&buffer[offset], len - offset - strlen(MINIMUM_MESSAGE) - 1, identity);
    if (ret > 0)
    {
        offset += ret;
        buffer[offset++] = ',';
    }
    ret = format_mdc(&buffer[offset], len - offset - strlen(MINIMUM_MESSAGE) - 1, mdc);
    if (ret > 0)
    {
        offset += ret;
        buffer[offset++] = ',';
    }
    ret = format_message(&buffer[offset], len - offset - 1, msg, arglist);
    if (ret > 0)
        offset += ret;
    else
    {
        buffer[0] = '\0';
        return -1;
    }

    buffer[offset++] = '}';
    buffer[offset] = '\0';
    return offset;
}

int mdclog_internal_format_to_json_str(char* buffer,
                       size_t len,
                       struct timeval* timestamp,
                       const char* identity,
                       mdclog_severity_t severity,
                       mdc_t* mdc,
                       const char* msg,
                       va_list arglist)
{
    if (len < MIN_BUFFER_LENGTH)
        return -1;
    return format_log_entry(buffer, len, timestamp, identity, severity, mdc, msg, arglist);
}

