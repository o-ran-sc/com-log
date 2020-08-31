..
.. Copyright (c) 2019 AT&T Intellectual Property.
..
.. Copyright (c) 2019 Nokia.
..
..
.. Licensed under the Creative Commons Attribution 4.0 International
..
.. Public License (the "License"); you may not use this file except
..
.. in compliance with the License. You may obtain a copy of the License at
..
..
..     https://creativecommons.org/licenses/by/4.0/
..
..
.. Unless required by applicable law or agreed to in writing, documentation
..
.. distributed under the License is distributed on an "AS IS" BASIS,
..
.. WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
..
.. See the License for the specific language governing permissions and
..
.. limitations under the License.
..
.. This source code is part of the near-RT RIC (RAN Intelligent Controller)
.. platform project (RICP).
..

Developer Guide
===============

Log repo location
-----------------

.. code:: bash

 git clone "https://gerrit.o-ran-sc.org/r/com/log"

Pre-Requisites
--------------

* gcc > version 4
* autoconf
* gawk
* libtool
* automake
* make
* pkg-config
* autoconf-archive
* libjsoncpp-dev
* gcc-c++
* doxygen

Compilation uses GNU autotools

.. code:: bash

 ./autogen.sh
 ./configure
 make

Installing
----------

Installation is done using command `make install`.
Installation directory can be set with the `configure` options `--prefix` and `--libdir` before compilation.
Optionally the root can be set with `DESTDIR=<my root dir>` argument to the `make install` command.

Example Log
-----------

.. code:: bash

 {"timestamp":1550045469,"severity":"INFO","logger":"applicationABC", "mdc":{"key1":"value1","key2":"value2"}, "message": "This is an example log"}


Using the library
-----------------

Example usage

.. code:: bash

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

Compilation
-----------

The library should be taken to C code compilation with the pkg-config. The library installation installs also the corresponding
.pc file under the library directory. If pkg-config does not find the mdclog .pc file, you should set environment
variable `PKG_CONFIG_PATH` poiting to the installation directory. For example, if the installation `prefix` was */home/username/mdclog* then
set the environment variable like this `export PKG_CONFIG_PATH=/home/username/mdclog/lib/pkgconfig`.

.. code:: bash

 CFLAGS=$(pkg-config mdclog -cflags)
 LIBS=$(pkg-config mdclog -libs)

 gcc myapp.c $CFLAGS $LIBS -o myapp

Log API's
---------
1. Init MDC Configuration

.. code:: bash

 int mdclog_init(mdclog_attr_t *attr)

2. MDC Log Write 

.. code:: bash

 void mdclog_write(mdclog_severity_t severity, const char *format, ...)  

3. Set MDC Log level

.. code:: bash

 void mdclog_level_set(mdclog_severity_t level)

4. Get MDC Log level

.. code:: bash

 mdclog_severity_t mdclog_level_get(void)

5. Initialise the MDC log attributes

.. code:: bash

 int mdclog_attr_init(mdclog_attr_t **attr)

6. Clean-up the MDC Log attributes

.. code:: bash

 void mdclog_attr_destroy(mdclog_attr_t *attr)  

7. Set MDC Log identity

.. code:: bash

 int mdclog_attr_set_ident(mdclog_attr_t *attr, const char *identity)  

8. Add Key-Value in MDC Log

.. code:: bash

 int mdclog_mdc_add(const char *key, const char *value)  

9. Remove Key-Value from MDC Log

.. code:: bash

 void mdclog_mdc_remove(const char *key)

10. Get Value of the Key from MDC Log 

.. code:: bash

 char *mdclog_mdc_get(const char *key)

11. Cleanup the MDC Log instance

.. code:: bash

 void mdclog_mdc_clean(void)   

Unit testing
------------

Unit testing is executed using `make check` or `make test` commands.


Continuous Integration
----------------------

The supplied Dockerfile defines an image that builds and tests this library
for continuous integration (CI) purposes such as a Jenkins job.


Code coverage report
--------------------

Enable unit test gcov code coverage analysis by configuring gcov reporting directory:

.. code:: bash

 configure --with-gcov-report-dir=DIR


Directory can be an absolute path or a relative path to an log source root.
Unit test build creates directory if it does not exist.

Build and run unit tests with code coverage analysis:

.. code:: bash

 make test_gcov

After successful unit test run code coverage (.gcov) result files are in
a directory, what was defined by `--with-gcov-report-dir` configure option.

In addition, graphical gcov front-ends such as lcov can be used for coverage
analysis:

.. code:: bash

 lcov --directory tst/ --directory src --capture --output-file coverage.info
 genhtml coverage.info --output-directory out

Open the out/index.html using any web browser.

Binary package creation
-----------------------

Debian and RPM packages can be generated with the `package.sh` script.
To make debian (.dep) packages you need to install

 * devscripts
 * debhelper

and run 

.. code:: bash

 ./package.sh debian

To make RPM packages you need to install
 * rpm-build

and run

.. code:: bash

 ./package.sh rpm

Docker Tests
------------

It's also possible to test compilation, run unit tests and test building of rpm and Debian packages in a Docker:

.. code:: bash

 docker build  --no-cache -f docker_test/Dockerfile-Test -t logtest:latest 

If needed, ready rpm and Debian packages can be copied from Docker to host.In below example packages are copied to host's /tmp/logtest-packages directory:

.. code:: bash

 docker run -v /tmp/logtest-packages:/export logtest:latest /export

