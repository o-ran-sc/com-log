#!/bin/bash

TARGET=/export
if [ $# -eq 1 ]
then
    TARGET=$1
fi

cp /tmp/*.rpm /tmp/*.deb "$TARGET"
