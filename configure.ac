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
# mdclog version
#
# 1. If no interface changes has been made, increment MDCLOG_MICRO.
# 2. Else, if new interfaces have been added but existing interfaces have not
#    been changed, increment MDCLOG_MINOR and zero MDCLOG_MICRO.
#    (Backward compatible interface change.)
# 3. Else, increment MDCLOG_MAJOR and zero both MDCLOG_MINOR and
#    MDCLOG_MICRO. (Backward incompatible interface change.)
#
# Change the numbers just before release.

m4_define([MDCLOG_MAJOR], 0)
m4_define([MDCLOG_MINOR], 0)
m4_define([MDCLOG_MICRO], 3)

# MDCLOG ABI version with libtool
#
# 1. Always increase the revision value.
# 2. Increase the current value whenever an interface has been added, removed
#    or changed.
# 3. Increase the age value only if the changes made to the ABI are backward
#    compatible.
#
# See https://autotools.io/libtool/version.html
#
# Change the numbers just before release.

m4_define([MDCLOG_CURRENT], [0])
m4_define([MDCLOG_REVISION], [3])
m4_define([MDCLOG_AGE], [0])
AC_CONFIG_MACRO_DIR([m4])
AC_INIT([mdclog], [MDCLOG_MAJOR.MDCLOG_MINOR.MDCLOG_MICRO], [], [], [])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([debian/changelog])
AC_CONFIG_FILES([rpm/mdclog.spec])
AC_CONFIG_FILES([run-tests.sh], [chmod +x run-tests.sh])
AM_INIT_AUTOMAKE([subdir-objects foreign 1.13 tar-pax -Wall -Werror])
AM_PROG_AR
AM_PROG_LIBTOOL
AC_PROG_CC_STDC
AC_LANG([C])
AC_PROG_CXX
AM_PROG_CC_C_O
AC_LANG([C++])
AX_GCC_FUNC_ATTRIBUTE([visibility])

AS_IF([test "xax_cv_have_func_attribute_visibility" != "x"],
[
    CFLAG_VISIBILITY="-fvisibility=hidden"
])
AC_SUBST([CFLAG_VISIBILITY])

LT_INIT

AX_PTHREAD
AX_CODE_COVERAGE
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([JSONCPP], [jsoncpp],
[
    have_jsoncpp=yes
    AC_DEFINE(HAVE_JSONCPP, 1, [Have jsoncpp])
], [AC_MSG_WARN([jsoncpp not found])])

MDCLOG_LT_VERSION=m4_format("%d:%d:%d", MDCLOG_CURRENT, MDCLOG_REVISION, MDCLOG_AGE)
AC_SUBST(MDCLOG_LT_VERSION)
AC_OUTPUT
