# rmw-0.7.10-dev

## Description

rmw (ReMove to Waste) is a safe-remove utility for the command line.
Its goal is to conform to [the FreeDesktop.org Trash
specification](https://specifications.freedesktop.org/trash-spec/trashspec-latest.html)
and therefore be compatible with KDE, GNOME, Xfce, and others. Desktop
integration is optional however, and by default, rmw will only use a
waste directory separated from your desktop trash. One of its unique
features is the ability to purge items from your Waste/Trash
directories after x number of days.

Web site: <https://remove-to-waste.info/>

## Build Status

* [![Build Status](https://api.travis-ci.com/theimpossibleastronaut/rmw.svg?branch=master)](https://travis-ci.com/github/theimpossibleastronaut/rmw)
* [![CodeQL](https://github.com/theimpossibleastronaut/rmw/workflows/CodeQL/badge.svg)](https://github.com/theimpossibleastronaut/rmw/actions?query=workflow%3ACodeQL)
* [![Build Status](https://semaphoreci.com/api/v1/andy5995/rmw-3/branches/master/badge.svg)](https://semaphoreci.com/andy5995/rmw-3)

## Available packages

[![Packaging status](https://repology.org/badge/vertical-allrepos/rmw.svg)](https://repology.org/project/rmw/versions)

Maintainer-created Debian packages (amd64) are available in the
[releases
section](https://github.com/theimpossibleastronaut/rmw/releases).

## Screenshots

![rmw usage output](https://remove-to-waste.info/images/Screenshot_2020-04-11.png)

[More Screenshots](https://remove-to-waste.info/screenshots.html)

## Contact / Support

* [Bug Reports and Feature Requests](https://github.com/theimpossibleastronaut/rmw/blob/master/CONTRIBUTING.md#bug-reports-and-feature-requests)
* [General Help, Support, Discussion](https://remove-to-waste.info/#support)

## Required libraries

* libncursesw (ncurses-devel on some systems, such as CentOS)
* gettext (or use --disable-nls if you only need English language support)

If you are building from source, you will need the libncursesw(5 or
6)-dev package from your operating system distribution. On some systems
just the ncurses packages is needed, and it's often already installed.

## Compiling

### As a normal user:

Use `../configure --help` to view available compile-time options.

    mkdir build
    cd build
    ../configure
    make

### Installing without superuser privileges

If you would like to install rmw without superuser privileges, use a prefix
that you have write access to. Example:

    ../configure --prefix=$HOME/.local
    make
    make install

The rmw binary will be installed to `$HOME/.local/bin` and documentation to
`$HOME/.local/doc`.

### If configure fails

On **OSX**, ncursesw isn't provided by default but can be installed
using `brew install ncurses`. Then precede `./configure` with
`PKG_CONFIG_PATH="/usr/local/opt/ncurses/lib/pkgconfig"` Example:

    PKG_CONFIG_PATH="/usr/local/opt/ncurses/lib/pkgconfig" ../configure

If you can't use [brew](https://brew.sh/), or install libncursesw or
libmenuw some other way, rmw will use `ncurses` but you may experience
[this minor
bug](https://github.com/theimpossibleastronaut/rmw/issues/205).

Note: rmw was built on **Windows** 2 years ago using Cygwin but it
didn't use the proper directories. We have no Windows developers
working on this project and are hoping that some will join soon!. As
stated in the description, the goal of this project is a
"cross-platform" utility; so getting rmw to work on Windows is
still on the TODO list.

## Uninstall / Cleaning up

* make uninstall (uninstalls the program if installed with 'make install`)
* make distclean (removes files in the build directory created by
`configure` and 'make')

## Usage
```
== First-time use ==

After rmw is installed, running `rmw` will create a configuration file
(rmwrc) in $HOME/.config (or $XDG_CONFIG_HOME). Edit the file as
desired.

== Features and Options ==

=== Basic usage ===

'rmw <file(s)>'

Items (file or directories) will be moved to a wastebasket in the same
manner as when using the "move to trash" option from your desktop GUI.
They will be separated from your desktop trash by default; or if you
wish for them to share the same "trash" directory, uncomment the line (in
your config file):

(Note that this does not apply to MacOS; while rmw is yet unable to
integrate with the desktop trash directory, you'll still be able to use
the default Waste directory.)

    WASTE = $HOME/.local/share/Trash

then comment out the line

    WASTE = $HOME/.local/share/Waste

You can reverse which directories are enabled at any time if you ever
change your mind. If both directories are on the same filesystem, rmw will
place files in the first one listed.

It can be beneficial to have them both uncommented. If your desktop
trash directory is uncommented, rmw won't place newly rmw'ed files there,
but it will purge files that were trashed (or wasted) after the amount
of days specified by the 'purge_after' value in your config file.

When rmw'ing a file or directory, if it already exists in the waste (or
trash) directory, it will not be overwritten; instead, the current file
being rmw'ed will have a time/date string (formatted as
"_%H%M%S-%y%m%d") appended to it (e.g. 'foo_164353-210508').


-h, --help
-c, --config filename     use an alternate configuration
-l, --list                list waste directories

If -v is added, this option will also show whether or not the waste directory
is marked as removable and if its current "attached" or "detached".

-g[N_DAYS], --purge[=N_DAYS]
                          purge expired files;
                          optional argument 'N_DAYS' overrides 'purge_after'
                          value from the configuration file
                          (Examples: -g90, --purge=90)

If purging is enabled, rmw will permanently delete files from the
directories specified in the configuration file after 'x' number of days.
By default, purging is disabled ('purge_after' is set to '0' in the
configuration file). To enable, use a value greater than '0' (Example:
If '45' is specified, rmw will permanently delete files that have been
in the waste (or trash) for more than 45 days.

The value of 'purge_after' can be temporarily overridden by using -g
[N_DAYS] or --purge[=NDAYS].

The time of the last automatic purge check is stored in `purge-time`,
located in $HOME/.local/share/rmw (or $XDG_DATA_HOME/rmw).

You can use '-vvg' to see when the remaining files will expire.


-o, --orphaned            check for orphaned files (maintenance)

This option is intended primarily for devlopers. Orphans should only
happen while testing code changes, or if there's a bug released with
rmw or another program that interfaces with your waste directories.


-f, --force               allow purging of expired files

rmw will refuse to purge files or directories if they contain
non-writable subdirectories. You can use -f 2 times if you ever see a
message that tells you "permission denied; directory still contains
files" (e.g. rwm -ffg).


-e, --empty               completely empty (purge) all waste directories
-r, -R, --recursive       option used for compatibility with rm
                          (recursive operation is enabled by default)
-v, --verbose             increase output messages
-w, --warranty            display warranty
-V, --version             display version and license information


=== Restoring ===

-z, --restore <wildcard filename(s) pattern> (e.g. ~/.local/share/Waste/files/foo*)

To restore a file, or multiple files, specify the path to them in the
<WASTE>/files directory (wildcards ok).

    rmw -z ~/.local/share/Waste/files/foo*

If a file or directory already exist at the same location and with the
same name, the item being restored will have a time/date string
(formatted as "_%H%M%S-%y%m%d") appended to it (e.g.
'foo_164353-210508').

-s, --select              select files from list to restore

This will bring up an interactive list of files in your waste directories.
You can use the left/right cursor keys to switch between one waste
directory and another. You can select multiple files to restore at once,
then press enter to restore them.

-u, --undo-last           undo last ReMove

Restores files that were last rmw'ed. No arguments for `-u` are
necessary. The list of files that were last rmw'ed is stored in `mrl`,
located in $HOME/.local/share/rmw (or $XDG_DATA_HOME/rmw).

-m, --most-recent-list    list most recently rmw'ed files

== Configuration File ==

Waste directories will be created automatically (Except for when the ',removable' option is used; see below) e.g., if
'$HOME/.local/share/Waste' is uncommented in the config file, these 3
directories will be created:

    $HOME/.local/share/Waste
    $HOME/.local/share/Waste/files
    $HOME/.local/share/Waste/info

If one of the WASTE directories is on removable media, then the user has the
option of appending ',removable'.

If a directory has ',removable' appended to it, rmw will not try to create
it; it must be initially created manually. When rmw runs, it will check
to see if the directory exists (which means the removable media containing
the directory is currently mounted). If rmw can't find the directory, it is
assumed the media containing the directory isn't mounted and that directory
will not be used for the current run of rmw.

With the media mounted, once you manually create the waster directory for
that device (e.g. ".Trash-$UID") and run rmw, it will automatically
create the two required sub-directories "Trash-$UID/info" and
".Trash-$UID/files". The directory you create must match what's
included in your config file:

    e.g.: WASTE=/mnt/flash/.Trash-$UID, removable

```
