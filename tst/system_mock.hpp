/*
 * system_mock.hpp
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

#ifndef TST_SYSTEM_MOCK_HPP_
#define TST_SYSTEM_MOCK_HPP_

#include <gmock/gmock.h>
#include <sys/uio.h>

namespace mdclogtest
{
   class SystemMock
   {
   public:
       MOCK_METHOD3(write, ssize_t(int fd, const void* buf, size_t count));
   };

   void setSystemMock(SystemMock *);
}
#endif /* TST_SYSTEM_MOCK_HPP_ */
