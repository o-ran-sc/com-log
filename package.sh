#!/bin/bash
#
# Copyright (c) 2019 AT&T Intellectual Property.
# Copyright (c) 2018-2019 Nokia.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#  This source code is part of the near-RT RIC (RAN Intelligent Controller)
#  platform project (RICP).
#
set -e

if [ $# -eq 0 ]
then
   echo "Generate a binary package"
   echo "Usage: $0 [--target-dir <dir>] [--skip-config] target..."
   echo "Where possible targets are debian and rpm"
   exit 1
fi

SKIP_CONF=0
BUILD_RPM=0
BUILD_DEB=0
SKIP_TEST=0
TARGET_DIR=/tmp

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --target-dir)
            TARGET_DIR="$2"
            shift
            ;;
        --skip-config)
            SKIP_CONF=1
            ;;
        --skip-test)
            SKIP_TEST=1
            ;;
        rpm)
            BUILD_RPM=1
            ;;
        debian)
            BUILD_DEB=1
            ;;
        *)
            echo "Unknown argument $1";
            exit 1
            ;;
    esac
    shift
done

if [ $SKIP_CONF -eq 0 ]
then
    ./autogen.sh && ./configure
fi

if [ $BUILD_RPM -ne 0 ]
then
    if [ $SKIP_TEST ]
    then
        TESTOPT=--nocheck
    fi
    rpmbuild --nodeps $TESTOPT -bb rpm/mdclog.spec --define="_sourcedir $PWD" --define="_builddir $PWD" --define="_rpmdir .."
    mkdir -p "$TARGET_DIR"
    if ! [[ ../x86_64 -ef "$TARGET_DIR" ]]; then
        cp ../x86_64/*.rpm "$TARGET_DIR"
    fi
fi

if [ $BUILD_DEB -ne 0 ]
then
    if [ $SKIP_TEST -eq 1 ]
    then
        export DEB_BUILD_OPTIONS="nocheck noddebs"
    fi
    debuild -b -us -uc
    mkdir -p "$TARGET_DIR"
    if ! [[ ../ -ef "$TARGET_DIR" ]]; then
        cp ../*.deb "$TARGET_DIR"
    fi
fi
