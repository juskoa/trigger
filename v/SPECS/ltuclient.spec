# RPM specfile for ltuclient

Summary: ltuclient - Alice TRIGGER software
Name: ltuclient
Version: 2.11
Release: 1
#Copyright: Alice_TRG
License: Alice_TRG
Distribution: Alice_TRG
URL: http://epweb2.ph.bham.ac.uk/user/pedja/alice/
Source: %{name}-%{version}.src.tar.gz
Group: Applications/Alice
Prefix: /opt/ltuclient
BuildRoot: %{_tmppath}/%{name}-root

#automatic dependencies
AutoReqProv: yes

Requires: dim python tkinter
          
# here is defined the installation root directory
#%define _topdir /home/alice/trigger/rpmbuild
%define pkgname %{name}-%{version}
%define destdir %{prefix}

%description
Alice Trigger ltuclient software.
Installs in /opt/ltuclient
Start:
set DIM_DNS_NODE                      
. /opt/ltuclient/scripts/setup.sh     (sets VMECFDIR,VMEBDIR,...)
ltuclient DETNAME                     (DETNAME: daq, spd, muon_trk,...)
rc: 0:ok   4:parameter error    8:corresponding LTUDIM server not available

# no debug package
%define debug_package %{nil}
# store directly files under RPM directory
%define _build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm

%prep
# extract archive
#%setup -n %{pkgname}
%setup -q

%build
#rm -rf %{buildroot}
#make install DESTDIR=%{buildroot}
export OS=`uname`
export DIMDIR=/opt/dim
. /opt/dim/setup.sh
echo `pwd`
printenv
cd vme/ltudim
make

%install
#remove install dir if existing
[ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT
#make install in install root directory
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/ltudim/linux
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/ltu
mkdir -p $RPM_BUILD_ROOT%{prefix}/scripts
mkdir -p $RPM_BUILD_ROOT%{prefix}/vmeb
#cp -a v/vme/ltudim/linux/ltuclient $RPM_BUILD_ROOT%{prefix}/vme/ltudim/linux
cp -a vmeb $RPM_BUILD_ROOT%{prefix}
cp -a vme $RPM_BUILD_ROOT%{prefix}
cp -a scripts $RPM_BUILD_ROOT%{prefix}

%pre

%clean
# remove source files
rm -rf $RPM_BUILD_DIR/%{pkgname}
# remove installed files
rm -rf $RPM_BUILD_ROOT

%files
%defattr (-,root,root)
%{destdir}

%post

%postun

