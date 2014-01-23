# RPM specfile for ltuclient

Summary: ltusw - Alice TRIGGER software for LTU operation in test setup
Name: ltusw
Version: 5.0
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
#AutoReqProv: yes
AutoReq: no

Requires: dim python tkinter
          
# here is defined the installation root directory
%define _topdir /home/alice/trigger/rpmbuild
# nebavi tu, avsak bavi v ~/.rpmmacros!
#%define _localstatedir /tmp
%define pkgname %{name}-%{version}
%define destdir %{prefix}

%description
Alice Trigger sw for LTU operation
Installs in /opt/ltusw
More in: DOC/Installation 
1.
Modify a line in /opt/ltusw/vme/CFG/ctp/DB/ttcparts.cfg
setting the detector name, VME CPU name and base address of your LTU.
2.
Set environment:
. /opt/ltusw/scripts/setup.sh [DIM_DNS_NODE]
            -sets VMECFDIR,VMEBDIR,DIM_DNS_NODE,...
            DIM_DNS_NODE defaults to pcald30
3.
Start one of the following:
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
echo pwd: `pwd`
#printenv
#cd $RPM_BUILD_ROOT%{prefix}   -empty
. scripts/vmebse.bash `pwd`
scripts/distrib make

%install
#remove install dir if existing
[ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT
echo install pwd:`pwd`
echo install bld:$RPM_BUILD_ROOT%{prefix}
#make install in install root directory
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme
cp -a scripts $RPM_BUILD_ROOT%{prefix}
cd vme
cp -a CFG $RPM_BUILD_ROOT%{prefix}/vme
ls -R $RPM_BUILD_ROOT%{prefix}/vme
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/SSMANA
cp -a SSMANA/linux $RPM_BUILD_ROOT%{prefix}/vme/SSMANA
#make -C SSMANA
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/VME2FPGA
cd VME2FPGA
cp -a VME2FPGA.exe VME2FPGA_u.py VME2FPGA_cf.py mywrl.py $RPM_BUILD_ROOT%{prefix}/vme/VME2FPGA
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/ltu/ltulib/linux_c
cd ../ltu
cp -a ltu.exe ltu_u.py ltu_cf.py ltu6.tcl slmcmp.py slmcomp.py slmdefs.py cfgedit.py ssman.py $RPM_BUILD_ROOT%{prefix}/vme/ltu
cp -a ltulib/linux_c/libltu.a $RPM_BUILD_ROOT%{prefix}/vme/ltu/ltulib/linux_c
#cp -a vmeb $RPM_BUILD_ROOT%{prefix}
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/ltu_proxy/linux
cd ../ltu_proxy 
cp -a linux/ltu_proxy $RPM_BUILD_ROOT%{prefix}/vme/ltu_proxy/linux
cp -a noclasses $RPM_BUILD_ROOT%{prefix}/vme/ltu_proxy
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/ltudim
cd ..
cp -a ltudim/lin* $RPM_BUILD_ROOT%{prefix}/vme/ltudim
mkdir -p $RPM_BUILD_ROOT%{prefix}/vmeb/vmeblib
cd ../vmeb
cp -a myw.py crate.py cmdlin2.py counters.py $RPM_BUILD_ROOT%{prefix}/vmeb
mkdir -p $RPM_BUILD_ROOT%{prefix}/DOC
cd ../DOC
cp -a Installation history.txt $RPM_BUILD_ROOT%{prefix}/DOC
mkdir -p $RPM_BUILD_ROOT%{prefix}/vme/CFG/ctp/DB
cd ..
#cp -a vme/CFG $RPM_BUILD_ROOT%{prefix}/vme
cp vme/CFG/ltu/ttcparts_lab.cfg $RPM_BUILD_ROOT%{prefix}/vme/CFG/ctp/DB/ttcparts.cfg
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

