MDC logging library
===================

A thread-safe logging C API library with Mapped Diagnostics Context (MDC) support.

Overview
--------

### Library initialization

Library initialization is an optional step, which can be done using mdclog_init() function.
By calling mdclog_init() the library user can define the logger identity tag, which is added to every log entry by
the library. If the mdclog_init() function is not called, the library uses the program name as the identity.

### Mapped Diagnostics Context

The MDCs are thread specific key-value pairs, which are included to all log entries by the library.
Thread specific here means that if a thread sets an MDC, then it is added only to the log entries
written by that thread. Same applies to all MDC functions, a thread can only remove or get MDCs
it has set.

### Log entry format

Each log entry written with mdclog_write() function contains

 * Timestamp
 * Logger identity
 * Log entry severity
 * All existing MDC pairs
 * Log message text

Currently the library only supports JSON formatted output written to standard out of the process

*Example log output*

`{"ts":1551183682974,"crit":"INFO","id":"myprog","mdc":{"second key":"other value","mykey":"keyval"},"msg":"hello world!"}`


License
-------
 Copyright (c) 2019 AT&T Intellectual Property.
 Copyright (c) 2018-2019 Nokia.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This source code is part of the near-RT RIC (RAN Intelligent Controller)
 platform project (RICP).


Requires
--------

### Build requires
* gcc > version 4
* autoconf
* gawk
* libtool
* automake
* make
* pkg-config
* autoconf-archive
* libjsoncpp-dev

### Unit testing requires
* gcc-c++

### Doxygen document generation requires
* doxygen

Compiling
---------

Compilation uses GNU autotools

```
./autogen.sh
./configure
make
```

Installing
----------

Installation is done using command `make install`.
Installation directory can be set with the `configure` options `--prefix` and `--libdir` before compilation.
Optionally the root can be set with `DESTDIR=<my root dir>` argument to the `make install` command.

Using the library
-----------------

### Example usage

```c
#include <mdclog/mdclog.h>

void init_log()
{
    mdclog_attr_t *attr;
    mdclog_attr_init(&attr);
    mdclog_attr_set_ident(attr, "myapp");
    mdclog_init(attr);
    mdclog_attr_destroy(attr);
}

int main()
{
    init_log();
    mdclog_mdc_add("mykey", "keyval");
    mdclog_mdc_add("second key", "other value");
    mdclog_write(MDCLOG_INFO, "hello world!");
}
```

### Compilation
The library should be taken to C code compilation with the pkg-config. The library installation installs also the corresponding
.pc file under the library directory. If pkg-config does not find the mdclog .pc file, you should set environment
variable `PKG_CONFIG_PATH` poiting to the installation directory. For example, if the installation `prefix` was */home/username/mdclog* then
set the environment variable like this `export PKG_CONFIG_PATH=/home/username/mdclog/lib/pkgconfig`.

```
CFLAGS=$(pkg-config mdclog -cflags)
LIBS=$(pkg-config mdclog -libs)

gcc myapp.c $CFLAGS $LIBS -o myapp
```

API documentation
-----------------

Library API documentation is made with *doxygen* and can be generated with `doxygen Doxyfile` command.


Unit testing
------------

Unit testing is executed using `make check` or `make test` commands.


Continuous Integration
----------------------

The supplied Dockerfile defines an image that builds and tests this library
for continuous integration (CI) purposes such as a Jenkins job.


Code coverage report
--------------------

Enable unit test gcov code coverage analysis by configuring gcov reporting
directory:
`
configure --with-gcov-report-dir=DIR
`

Directory can be an absolute path or a relative path to an log source root.
Unit test build creates directory if it does not exist.

Build and run unit tests with code coverage analysis:
`
make test_gcov
`

After successful unit test run code coverage (.gcov) result files are in
a directory, what was defined by `--with-gcov-report-dir` configure option.

In addition, graphical gcov front-ends such as lcov can be used for coverage
analysis:
`
lcov --directory tst/ --directory src --capture --output-file coverage.info
genhtml coverage.info --output-directory out
`

Open the out/index.html using any web browser.

Binary package creation
-----------------------

Debian and RPM packages can be generated with the `package.sh` script.
To make debian (.dep) packages you need to install
 * devscripts
 * debhelper

and run `./package.sh debian`

To make RPM packages you need to install
 * rpm-build

and run `./package.sh rpm`

Docker Tests
------------

It's also possible to test compilation, run unit tests and test building of
rpm and Debian packages in a Docker:
`
docker build  --no-cache -f docker_test/Dockerfile-Test -t logtest:latest .
`

If needed, ready rpm and Debian packages can be copied from Docker to host. In
below example packages are copied to host's /tmp/logtest-packages directory:
`
docker run -v /tmp/logtest-packages:/export logtest:latest /export
`
