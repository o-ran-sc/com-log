Name:     mdclog
Version:  0.0.5
Release:  1%{?dist}
Summary:  A thread safe structured logging library

License:  Apache-2
URL:      https://gerrit.o-ran-sc.org/r/admin/repos/com/log

BuildRequires: pkgconfig
BuildRequires: gcc
BuildRequires: autoconf
BuildRequires: autoconf-archive
BuildRequires: automake
BuildRequires: make
BuildRequires: gawk
BuildRequires: libtool

%description
%{name} provices a thread safe logging library

%package devel
Summary:   Development files for %{name}
Requires:  %{name}%{?isa} = %{version}-%{release}

%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%build
./autogen.sh
%configure
%make_build

%install
%make_install
rm -f %{buildroot}%{_libdir}/lib*.*a

%post -p /sbin/ldconfig

%files
%{_libdir}/*.so.*

%files devel
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
%{_includedir}/mdclog

%changelog
* Tue Jun 09 2020 Timo Tietavainen <timo.tietavainen@nokia.com> - 0.0.5-1
- Bump version number to test new makefile targets with Jenkins.

* Tue Jun 18 2019 Roni Riska <roni.riska@nokia.com> - 0.0.4-1
- new version




