#!/bin/bash 
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

for i in "$@"
do
    case "$i" in
    --target-dir)
        shift
        TARGET_DIR=$i
        ;;
    --skip-config)
        SKIP_CONF=1
        ;;
    --skip-test)
        SKIP_TEST=1
        ;;
    rpm)
        BUILD_RPM=1
        shift
        ;; 
    debian)
        BUILD_DEB=1
        ;;
    *)
        echo Unknown argument $1
        exit 1
        ;;
    esac
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
    cp ../x86_64/*.rpm $TARGET_DIR
fi

if [ $BUILD_DEB -ne 0 ]
then
    if [ $SKIP_TEST -eq 1 ]
    then
        export DEB_BUILD_OPTIONS=nocheck
    fi
    debuild -b -us -uc 
    cp ../x86_64/*.rpm $TARGET_DIR
fi


