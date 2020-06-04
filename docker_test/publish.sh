#!/bin/bash
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

# This script is used as a docker ENTRYPOINT in 'Dockerfile-Test'. Script
# purpose is to help rpm and Debian packages copying from docker to host.
# For further information please read the chapter 'Docker Tests' of the
# README file.

echo "$0: start copying packages"

TARGET=/export
if [ $# -eq 1 ]
then
    TARGET=$1
fi

if [ ! -d "$TARGET" ]
then
    echo "$0: Error: target dir $TARGET does not exist"
    exit 1
fi

cp -v /tmp/pkgs/*.rpm /tmp/pkgs/*.deb "$TARGET"
