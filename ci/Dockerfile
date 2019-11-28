# O-RAN-SC
#
# Copyright (C) 2019 AT&T Intellectual Property and Nokia
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#   This source code is part of the near-RT RIC (RAN Intelligent Controller)
#   platform project (RICP).
#


# CI to verify the MDCLOG library
# Inherits C toolchain from buildpack-deps:stretch
# Adds required build tools

FROM buildpack-deps:stretch

RUN apt-get update && apt-get -q -y install \
  autoconf \
  autoconf-archive \
  automake \
  libjsoncpp-dev \
  libtool \
  pkg-config \
  rpm \
  devscripts \
  debhelper \
  gawk \
  build-essential


RUN mkdir -p /tmp/build
COPY . /tmp/build
RUN cd /tmp/build && ci/ci_build.sh
RUN cp /tmp/build/ci/publish.sh /bin

ENTRYPOINT ["/bin/publish.sh"]
