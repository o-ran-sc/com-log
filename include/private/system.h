/*
 * Internal defines
 * - unit test support macros
 *
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
 */

#ifndef INCLUDE_PRIVATE_SYSTEM_H_
#define INCLUDE_PRIVATE_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <sys/uio.h>

#ifndef UNITTEST
#define SYSTEM(x) x
#define STATIC static
#else
// SYSTEM macro to map ut mocked system fuctions with system_ prefix.
// Mock function implementation is on system_mock.cpp in tst dir
#define SYSTEM(x) system_ ## x
// In case of unit testing we can use STATIC define which is empty in UT
#define STATIC

ssize_t system_write(int, const void*,size_t);
#endif
  
#if !HAVE_TEMP_FAILURE_RETRY
// copied from glibc's unistd.h
#define TEMP_FAILURE_RETRY(expression)          \
  (__extension__                                \
   ({ long int __result;                        \
     do __result = (long int) (expression);     \
     while (__result == -1L && errno == EINTR); \
     __result; }))
#endif
  
#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_PRIVATE_SYSTEM_H_ */
