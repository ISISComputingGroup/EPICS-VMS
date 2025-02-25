#!/bin/sh
set -o errexit
export EPICS_HOST_ARCH=linux-x86_64
MY_EPICS_BASE=`pwd`/base
find . -name RELEASE -exec sed -i -e "s%^EPICS_BASE=.*%EPICS_BASE=${MY_EPICS_BASE}%" {} \;
make
