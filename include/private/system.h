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
 *
 *  This source code is part of the near-RT RIC (RAN Intelligent Controller)
 *  platform project (RICP).
 *
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

#ifndef TEMP_FAILURE_RETRY
// this has slightly different semantics
// than glibc's unistd.h: we continue
// while the return value is <0 (not just
// == -1) and explicitly handle more
// retryable failures (which are typically
// all the same, but this is safer.)
#define TEMP_FAILURE_RETRY(expr)       \
  ({ long int __TFR_R=0;               \
    do __TFR_R=(long int)(expr);       \
    while(__TFR_R < (long int)0 &&     \
          (errno == EINTR  ||          \
           errno == EAGAIN ||          \
           errno == EWOULDBLOCK));     \
    __TFR_R; })
#endif

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_PRIVATE_SYSTEM_H_ */
