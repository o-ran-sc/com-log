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

ACLOCAL_AMFLAGS = -I m4

BASE_CFLAGS = -Wall -Werror -Wextra $(PTHREAD_CFLAGS) -I$(top_srcdir)/include -D_GNU_SOURCE
BASE_LDFLAGS = $(PTHREAD_CFLAGS)
BASE_LIBS = $(PTHREAD_LIBS)

lib_LTLIBRARIES = libmdclog.la
libmdclog_la_SOURCES = \
   src/mdclog.c \
   src/json_format.c \
   include/mdclog/mdclog.h \
   include/private/json_format.h \
   src/mdc.c \
   include/private/system.h \
   include/private/mdc.h

libmdclog_la_CFLAGS = $(BASE_CFLAGS) @CFLAG_VISIBILITY@ -DBUILDING_MDCLOG
libmdclog_la_LDFLAGS = $(BASE_LDFLAGS) -version-info @MDCLOG_LT_VERSION@
libmdclog_la_LIBADD = $(BASE_LIBS)

pkgincludedir = $(includedir)/mdclog
pkginclude_HEADERS = include/mdclog/mdclog.h

pkgconfigdir = $(libdir)/pkgconfig
nodist_pkgconfig_DATA = mdclog.pc

mdclog.pc: $(top_srcdir)/mdclog.pc.in
	sed -e 's![@]prefix[@]!$(prefix)!g' \
        -e 's![@]includedir[@]!$(includedir)!g' \
        -e 's![@]libdir[@]!$(libdir)!g' \
        -e 's![@]PACKAGE_VERSION[@]!$(PACKAGE_VERSION)!g' \
        -e 's![@]PACKAGE_URL[@]!$(PACKAGE_URL)!g' \
        $< > $@

check_LTLIBRARIES = libgmock.la
libgmock_la_SOURCES = \
    3rdparty/googlemock/include/gmock/gmock-actions.h \
    3rdparty/googlemock/include/gmock/gmock-cardinalities.h \
    3rdparty/googlemock/include/gmock/gmock-function-mocker.h \
    3rdparty/googlemock/include/gmock/gmock-generated-actions.h \
    3rdparty/googlemock/include/gmock/gmock-generated-function-mockers.h \
    3rdparty/googlemock/include/gmock/gmock-generated-matchers.h \
    3rdparty/googlemock/include/gmock/gmock-matchers.h \
    3rdparty/googlemock/include/gmock/gmock-more-actions.h \
    3rdparty/googlemock/include/gmock/gmock-more-matchers.h \
    3rdparty/googlemock/include/gmock/gmock-nice-strict.h \
    3rdparty/googlemock/include/gmock/gmock-spec-builders.h \
    3rdparty/googlemock/include/gmock/gmock.h \
    3rdparty/googlemock/include/gmock/internal/gmock-internal-utils.h \
    3rdparty/googlemock/include/gmock/internal/gmock-port.h \
    3rdparty/googlemock/include/gmock/internal/gmock-pp.h \
    3rdparty/googlemock/include/gmock/internal/custom/gmock-generated-actions.h \
    3rdparty/googlemock/include/gmock/internal/custom/gmock-matchers.h \
    3rdparty/googlemock/include/gmock/internal/custom/gmock-port.h \
    3rdparty/googlemock/src/gmock-cardinalities.cc \
    3rdparty/googlemock/src/gmock-internal-utils.cc \
    3rdparty/googlemock/src/gmock-matchers.cc \
    3rdparty/googlemock/src/gmock-spec-builders.cc \
    3rdparty/googlemock/src/gmock.cc \
    3rdparty/googlemock/src/gmock_main.cc \
    3rdparty/googletest/include/gtest/gtest-death-test.h \
    3rdparty/googletest/include/gtest/gtest-matchers.h \
    3rdparty/googletest/include/gtest/gtest-message.h \
    3rdparty/googletest/include/gtest/gtest-param-test.h \
    3rdparty/googletest/include/gtest/gtest-printers.h \
    3rdparty/googletest/include/gtest/gtest-spi.h \
    3rdparty/googletest/include/gtest/gtest-test-part.h \
    3rdparty/googletest/include/gtest/gtest-typed-test.h \
    3rdparty/googletest/include/gtest/gtest.h \
    3rdparty/googletest/include/gtest/gtest_pred_impl.h \
    3rdparty/googletest/include/gtest/gtest_prod.h \
    3rdparty/googletest/include/gtest/internal/gtest-death-test-internal.h \
    3rdparty/googletest/include/gtest/internal/gtest-filepath.h \
    3rdparty/googletest/include/gtest/internal/gtest-internal.h \
    3rdparty/googletest/include/gtest/internal/gtest-param-util-generated.h \
    3rdparty/googletest/include/gtest/internal/gtest-param-util.h \
    3rdparty/googletest/include/gtest/internal/gtest-port-arch.h \
    3rdparty/googletest/include/gtest/internal/gtest-port.h \
    3rdparty/googletest/include/gtest/internal/gtest-string.h \
    3rdparty/googletest/include/gtest/internal/gtest-type-util.h \
    3rdparty/googletest/include/gtest/internal/custom/gtest.h \
    3rdparty/googletest/include/gtest/internal/custom/gtest-port.h \
    3rdparty/googletest/include/gtest/internal/custom/gtest-printers.h \
    3rdparty/googletest/src/gtest-death-test.cc \
    3rdparty/googletest/src/gtest-filepath.cc \
    3rdparty/googletest/src/gtest-internal-inl.h \
    3rdparty/googletest/src/gtest-matchers.cc \
    3rdparty/googletest/src/gtest-port.cc \
    3rdparty/googletest/src/gtest-printers.cc \
    3rdparty/googletest/src/gtest-test-part.cc \
    3rdparty/googletest/src/gtest-typed-test.cc \
    3rdparty/googletest/src/gtest.cc \
    3rdparty/googletest/src/gtest_main.cc

libgmock_la_CXXFLAGS = \
    $(BASE_CFLAGS) \
    -I$(top_srcdir)/3rdparty/googletest/include \
    -I$(top_srcdir)/3rdparty/googlemock/include \
    -I$(top_srcdir)/3rdparty/googletest \
    -std=gnu++11

libgmock_la_LDFLAGS = \
    $(BASE_LDFLAGS)

libgmock_la_LIBADD = \
    $(BASE_LIBS)

check_PROGRAMS = testrunner

testrunner_SOURCES = \
   src/mdclog.c \
   tst/system_mock.cpp \
   tst/system_mock.hpp \
   tst/test_json_format.cpp \
   src/json_format.c \
   src/mdc.c \
   tst/test_mdc.cpp \
   tst/test_api.cpp

testrunner_CFLAGS = \
   $(BASE_CFLAGS) \
   -DUNITTEST

testrunner_CXXFLAGS = \
    $(BASE_CFLAGS) \
    -I$(top_srcdir)/3rdparty/googletest/include \
    -I$(top_srcdir)/3rdparty/googlemock/include \
    -std=gnu++11 \
    -DUNITTEST \
    $(JSONCPP_CFLAGS)

testrunner_LDFLAGS = $(BASE_LDFLAGS)
testrunner_LDADD = -lgmock $(BASE_LIBS) $(JSONCPP_LIBS)

EXTRA_testrunner_DEPENDENCIES = \
   libgmock.la

test: testrunner
	./run-tests.sh

TESTS = run-tests.sh

if ENABLE_GCOV
testrunner_CFLAGS += -O0 --coverage
testrunner_LDADD += -lgcov

test_gcov: test
	@for p in src/*.o; do \
		gcov -abcfru $$p 1>/dev/null; \
	done
	mkdir -p @GCOV_REPORT_DIR@
	mv *.gcov @GCOV_REPORT_DIR@
	ls -la @GCOV_REPORT_DIR@
endif

if ENABLE_RPM_BUILD
rpm-pkg: package.sh
	./package.sh rpm --target-dir @RPM_DIR@
endif


if ENABLE_DEB_BUILD
deb-pkg: package.sh
	./package.sh debian --target-dir @DEB_DIR@ --skip-test
endif

EXTRA_DIST = \
    $(top_srcdir)/mdclog.pc.in

clean-local:
	rm -f mdclog.pc

if ENABLE_GCOV
	rm -rf @GCOV_REPORT_DIR@
	@find . -name '*.gcno' -o -name '*.gcda'| xargs -r rm
endif
