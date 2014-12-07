#
# spec file for package amarok-nightly-kdebase
#
# Copyright (C) 2008 Harald Sitter <harald@getamarok.com>
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#

# norootforbuild


Name:           amarok-nightly-kdebase
BuildRequires:  amarok-nightly-qt amarok-nightly-kdesupport amarok-nightly-kdelibs amarok-nightly-deps
License:        GPL v2 or later
Group:          Productivity/Multimedia/Sound/Players
Summary:        KDEBase for Project Neon (Amarok Nightly)
Version:        0x1344224
Release:        1
Source0:        %name.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
%define prefix /opt/amarok-nightly

%description
KDEBase

%debug_package

%prep
%setup -q -n %name

%build
  mkdir build
  cd build
  export PATH=/opt/amarok-nightly/bin:$PATH
  export LD_LIBRARY_PATH=/opt/amarok-nightly/lib:$LD_LIBRARY_PATH
  cmake -DCMAKE_INSTALL_PREFIX=%prefix -DCMAKE_BUILD_TYPE=debugfull ..
  make %{?jobs:-j %jobs}

%install
  cd build
  make install DESTDIR=$RPM_BUILD_ROOT

%clean
  rm -rf $RPM_BUILD_ROOT

%files  
%defattr(-,root,root)
/opt/amarok-nightly/*

%changelog
* Wed Apr 30 2008 nightly@getamarok.com
- initial build
