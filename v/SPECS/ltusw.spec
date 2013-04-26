# RPM specfile for ltuclient

Summary: ltusw - Alice TRIGGER software for LTU operation in test setup
Name: ltusw
Version: 4.2
Release: 1
#Copyright: Alice_TRG
License: Alice_TRG
Distribution: Alice_TRG
URL: http://alicetrigger.web.cern.ch/alicetrigger/
Source: %{name}-%{version}.src.tar.gz
Group: Applications/Alice
Prefix: /opt/ltusw
BuildRoot: %{_tmppath}/%{name}-root

#automatic dependencies
AutoReqProv: yes

Requires: dim python tkinter
          
# here is defined the installation root directory
%define _topdir /home/alice/trigger/rpms
%define pkgname %{name}-%{version}
%define destdir %{prefix}

%description
Alice Trigger sw for LTU operation
Installs in /opt/ltusw
More in: DOC/Installation 
Start:
Modify: /opt/ltusw/vme/CFG/ctp/DB/ttcparts.cfg
set environment:
. /opt/ltusw/scripts/setup.sh [DIMDNSNODE]
            -sets VMECFDIR,VMEBDIR,DIM_DNS_NODE,...)
            DIMDNSNODE defauls to pcald30
vmedirs                -check environment variables
ltuproxy               -status/start/stop ltuproxy
vmecrate [nbi] ltu     -start LTU sw (direct VME, ltuproxy not involved)
vmecrate tof           -start LTU sw client (TOF ltuproxy has to be active)

# no debug package
%define debug_package %{nil}
# store directly files under RPM directory
%define _build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm

%prep
# extract archive
%setup -c -q
#%setup -c -n %{pkgname} 

%build
#export OS=`uname`
#export DIMDIR=/opt/dim
#. /opt/dim/setup.sh
echo `pwd`
#printenv
#cd $RPM_BUILD_ROOT%{prefix}   -empty
. scripts/vmebse.bash `pwd`
scripts/distrib make

%install
#remove install dir if existing
[ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT
echo install pwd:`pwd`
echo install bld:$RPM_BUILD_ROOT/%{prefix}
#make install in install root directory
mkdir -p $RPM_BUILD_ROOT/%{prefix}
cp -a scripts $RPM_BUILD_ROOT/%{prefix}
cd vme
cp -a CFG $RPM_BUILD_ROOT/%{prefix}/vme
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/SSMANA
cp -a SSMANA/ssmanv.exe $RPM_BUILD_ROOT/%{prefix}/vme/SSMANA
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/VME2FPGA
cd VME2FPGA
cp -a VME2FPGA.exe VME2FPGA_u.py VME2FPGA_cf.py mywrl.py $RPM_BUILD_ROOT/%{prefix}/vme/VME2FPGA
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/ltu/ltulib/linux_c
cd ../ltu
cp -a ltu.exe ltu_u.py ltu_cf.py ltu6.tcl slmcmp.py slmcomp.py ssman.py $RPM_BUILD_ROOT/%{prefix}/vme/ltu
cp -a ltulib/linux_c/libltu.a $RPM_BUILD_ROOT/%{prefix}/vme/ltu/ltulib/linux_c
#cp -a vmeb $RPM_BUILD_ROOT/%{prefix}
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/ltu_proxy/linux
cd ../ltu_proxy/linux 
cp -a ltu_proxy $RPM_BUILD_ROOT/%{prefix}/vme/ltu_proxy/linux
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/ltudim/linux
cd ../..
cp -a ltudim/linux/ltuclient $RPM_BUILD_ROOT%{prefix}/vme/ltudim/linux
mkdir -p $RPM_BUILD_ROOT%{prefix}/vmeb/vmeblib
cd ../vmeb
cp -a myw.py crate.py cmdlin2.py counters.py $RPM_BUILD_ROOT/%{prefix}/vmeb
mkdir -p $RPM_BUILD_ROOT%{prefix}/DOC
cd ../DOC
cp -a Installation history.txt $RPM_BUILD_ROOT/%{prefix}/DOC
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/CFG/ctp/DB
cd ..
cp -a vme/CFG $RPM_BUILD_ROOT%{prefix}/vme
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

