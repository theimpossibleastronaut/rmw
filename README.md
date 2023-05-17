# rmw-0.9.1
## Description

rmw (ReMove to Waste) is a trashcan/recycle bin utility for the command line.
It can move and restore files to and from directories specified in a
configuration file, and can also be integrated with your regular desktop trash
folder (if your desktop environment uses the FreeDesktop.org Trash
specification). One of the unique features of rmw is the ability to purge
items from your waste (or trash) directories after x number of days.

Web site: <https://theimpossibleastronaut.github.io/rmw-website/>

[![codeql-badge]][codeql-url]
[![c-cpp-badge]][c-cpp-url]

[c-cpp-badge]: https://github.com/theimpossibleastronaut/rmw/actions/workflows/c-cpp.yml/badge.svg
[c-cpp-url]: https://github.com/theimpossibleastronaut/rmw/actions/workflows/c-cpp.yml
[codeql-badge]: https://github.com/theimpossibleastronaut/rmw/workflows/CodeQL/badge.svg
[codeql-url]: https://github.com/theimpossibleastronaut/rmw/actions?query=workflow%3ACodeQL

rmw is for people who sometimes use rm or rmdir at the command line and
would occasionally like an alternative choice. It's not intended or
designed to act as a replacement for rm, as it's more closely related
to how the [FreeDesktop.org trash
system](https://specifications.freedesktop.org/trash-spec/trashspec-latest.html)
functions.

## Features and Usage

See the [manual on the
website](https://theimpossibleastronaut.github.io/rmw-website/rmw_man.html)

## Screenshots

See the [Screenshots](https://theimpossibleastronaut.github.io/rmw-website/screenshots.html)
page on the website.

## Contact / Support

* [Bug Reports and Feature Requests](https://github.com/theimpossibleastronaut/rmw/blob/master/CONTRIBUTING.md#bug-reports-and-feature-requests)
* [General Help, Support, Discussion](https://theimpossibleastronaut.github.io/rmw-website/#support)

## Installation

rmw is available in the [homebrew and
linuxbrew](https://github.com/Homebrew/) repositories; or there may may
be a binary package available for your OS. You can view a list at
[Repology](https://repology.org/project/rmw/versions) to see in which
repositories rmw is included. Since v0.7.09, x86_64 AppImages are
available.

AppImages and maintainer-created amd64 Debian packages are available in
the [releases section][releases-url].

[releases-url]: https://github.com/theimpossibleastronaut/rmw/releases

## Installing from source

### Required libraries

* libncursesw (ncurses-devel on some systems, such as CentOS)
* gettext (or use '-Dnls=false' when configuring with meson if you only need English language support)

If you're building from source, you will need the libncursesw(5 or
6)-dev package from your operating system distribution. On some systems
just the ncurses packages is needed, and it's often already installed.

### Compiling

#### As a normal user:

(This examples places the generated files to a separate folder, but you can
run 'configure' from any directory you like.)

```
    meson setup builddir
    cd builddir
    ninja
```

Use `meson configure` in the build dir to view or change available
options.

#### Installing without superuser privileges

If you would like to install rmw without superuser privileges, use a prefix
that you have write access to. Example:

    meson setup -Dprefix=$HOME/.local builddir

or while in the build dir

    meson configure -Dprefix=$HOME/.local

To install:

    meson install

In the example above, the rmw binary will be installed to
`$HOME/.local/bin` and documentation to `$HOME/.local/doc`.

### Uninstall

    ninja uninstall (uninstalls the program if installed with 'ninja install`)

### Docker

see the [Docker README](https://github.com/theimpossibleastronaut/rmw/tree/master/docker)
