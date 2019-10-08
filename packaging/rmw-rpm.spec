Name:           @PACKAGE@
Version:        @VERSION@
Release:        0
Summary:        A safe-remove utility for the command line
License:        GPLv3
URL:            https://remove-to-waste.info/
Source0:        https://github.com/theimpossibleastronaut/%{name}/archive/v%{version}/%{name}-v%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-root
Prefix:		%{_prefix}

%description
rmw (ReMove to Waste) is a safe-remove utility for the command line.

%prep
%setup

%build
%configure
make

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

%doc COPYING

%changelog
