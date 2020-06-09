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
m4_define([MDCLOG_MICRO], 5)

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
m4_define([MDCLOG_REVISION], [5])
m4_define([MDCLOG_AGE], [0])
AC_CONFIG_MACRO_DIR([m4])
AC_INIT([mdclog], [MDCLOG_MAJOR.MDCLOG_MINOR.MDCLOG_MICRO], [], [], [])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile])
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
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([JSONCPP], [jsoncpp],
[
    have_jsoncpp=yes
    AC_DEFINE(HAVE_JSONCPP, 1, [Have jsoncpp])
], [AC_MSG_WARN([jsoncpp not found])])

#
# Configuration option --with-rpm-dir=DIR
#   If this option is given, rpm packages building is enabled and ready rpms are
#   copied to the directory.
#
AC_ARG_WITH([rpm-dir],
    AS_HELP_STRING([--with-rpm-dir=DIR],
        [Directory for build rpm packages]),
    [AC_CHECK_PROG(RPMBUILD, rpmbuild, "yes", "no")
     AC_CHECK_PROG(RPM, rpm, "yes", "no")
     AS_IF([test "x$RPMBUILD" != xyes],
         [AC_MSG_ERROR([rpmbuild needs to be installed])],
     [test "x$RPM" != xyes],
         [AC_MSG_ERROR([rpm is needs to be installed])])],
    [with_rpm_dir=no])

AC_MSG_CHECKING([rpm package])
if test "x$with_rpm_dir" = "xno"; then
    AC_MSG_RESULT([no])
    RPM_DIR="/tmp"
else
    AC_MSG_RESULT([yes])
    AC_MSG_NOTICE([rpm directory: $with_rpm_dir])
    RPM_DIR="$with_rpm_dir"
fi
AC_SUBST(RPM_DIR)
AM_CONDITIONAL([ENABLE_RPM_BUILD],[test "x$with_rpm_dir" != "xno"])


#
# Configuration option --with-deb-dir=DIR
#   If this option is given, Debian packages packages building is enabled and ready
#   packages are copied to the directory.
#
AC_ARG_WITH([deb-dir],
    AS_HELP_STRING([--with-deb-dir=DIR],
        [Directory for build Debian packages]),
    [AC_CHECK_PROG(DEBUILD, dpkg-buildpackage, "yes", "no")
     AS_IF([test "x$DEBUILD" != xyes],
         [AC_MSG_ERROR([dpkg-buildpackage needs to be installed])],
             [test "x$with_rpm_dir" != "xno"],
                             [AC_MSG_ERROR([--with-deb-dir and --with-rpm-dir are mutually exclusive])])],
    [with_deb_dir=no])

AC_MSG_CHECKING([deb package])
if test "x$with_deb_dir" = "xno"; then
    AC_MSG_RESULT([no])
    DEB_DIR="/tmp"
else
    AC_MSG_RESULT([yes])
    AC_MSG_NOTICE([deb directory: $with_deb_dir])
    DEB_DIR="$with_deb_dir"
fi
AC_SUBST(DEB_DIR)
AM_CONDITIONAL([ENABLE_DEB_BUILD],[test "x$with_deb_dir" != "xno"])


#
# Configuration option --with-gcov-report-dir=DIR
#   If this option is given, gcov unit test coverage analysis is enabled and
#   results are copied to the directory.
#
AC_ARG_WITH([gcov-report-dir],
    AS_HELP_STRING([--with-gcov-report-dir=DIR],
        [Directory for GCOV report files]),
    [],
    [with_gcov_report_dir=no])
AC_MSG_CHECKING([gcov])
if test "x$with_gcov_report_dir" = "xno"; then
    AC_MSG_RESULT([no])
    GCOV_REPORT_DIR="gcov_report"
else
    AC_MSG_RESULT([yes])
    AC_MSG_NOTICE([gcov report directory: $with_gcov_report_dir])
    GCOV_REPORT_DIR="$with_gcov_report_dir"
fi
AC_SUBST(GCOV_REPORT_DIR)
AM_CONDITIONAL([ENABLE_GCOV],[test "x$with_gcov_report_dir" != "xno"])

MDCLOG_LT_VERSION=m4_format("%d:%d:%d", MDCLOG_CURRENT, MDCLOG_REVISION, MDCLOG_AGE)
AC_SUBST(MDCLOG_LT_VERSION)
AC_OUTPUT
