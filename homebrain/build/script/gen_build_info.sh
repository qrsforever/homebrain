#! /bin/bash

GEN_BUILD_INFO_DIR=$1

BUILD_INFO_FILE="$GEN_BUILD_INFO_DIR/build_info.h"
BUILD_DATETIME=`date +"%m/%d/%y %H:%M:%S"`
BUILD_USER=`whoami`

__get_git_version()
{
    export LANG="en_US.utf8"
    VERSION_NUMBER=`git rev-list HEAD | wc -l | awk '{print $1}'`
    VERSION_STRING="1.0.$VERSION_NUMBER $(git rev-list HEAD -n 1 | cut -c 1-7)"
}

__get_svn_version()
{
    export LANG="en_US.utf8"
    VERSION_NUMBER=`svn info | grep Revision | cut -d\: -f2 | sed 's/[[:space:]]//g'`
    echo "$VERSION_NUMBER"
    VERSION_STRING="1.0.$VERSION_NUMBER"
}

__get_git_version

echo "#ifndef __PROJECT_VERSION_H" > $BUILD_INFO_FILE
echo "#define __PROJECT_VERSION_H" >> $BUILD_INFO_FILE
echo "" >> $BUILD_INFO_FILE
echo "#define HB_BUILD_DATETIME \"$BUILD_DATETIME\"" >> $BUILD_INFO_FILE
echo "#define HB_BUILD_USER \"$BUILD_USER\"" >> $BUILD_INFO_FILE
echo "#define HB_BUILD_VERSION \"$VERSION_STRING\"" >> $BUILD_INFO_FILE
echo "" >> $BUILD_INFO_FILE
echo "#endif" >> $BUILD_INFO_FILE
