# rmw-0.8.0-dev
## Description

rmw (ReMove to Waste) is a safe-remove utility for the command line. It
can move and restore files to and from directories specified in a
configuration file, and can also be integrated with your regular
desktop trash folder (if your desktop environment uses the
FreeDesktop.org Trash specification). One of the unique features of rmw
is the ability to purge items from your waste (or trash) directories
after x number of days.

Web site: <https://remove-to-waste.info/>

[![travis-badge]][travis-url]
[![codeql-badge]][codeql-url]
[![semaphore-badge]][semaphore-url]

[travis-badge]: https://api.travis-ci.com/theimpossibleastronaut/rmw.svg?branch=master
[travis-url]: https://travis-ci.com/github/theimpossibleastronaut/rmw
[codeql-badge]: https://github.com/theimpossibleastronaut/rmw/workflows/CodeQL/badge.svg
[codeql-url]: https://github.com/theimpossibleastronaut/rmw/actions?query=workflow%3ACodeQL
[semaphore-badge]: https://semaphoreci.com/api/v1/andy5995/rmw-3/branches/master/badge.svg
[semaphore-url]: https://semaphoreci.com/andy5995/rmw-3

rmw is for people who sometimes use rm or rmdir at the command line and
would occasionally like an alternative choice. It's not intended or
designed to act as a replacement for rm, as it's more closely related
to how the [FreeDesktop.org trash
system](https://specifications.freedesktop.org/trash-spec/trashspec-latest.html)
functions.

## Features and Usage
<!-- Don't make changes below this line, but to src/man/rmw.1 instead. -->
<!-- This is generated with 'man ./rmw.1 | col -b > plaintext' -->
```
OPTIONS
       -h, --help
	      show help for command line options

       -c, --config FILE
	      use an alternate configuration

       -l, --list
	      list waste directories

       -g[N_DAYS], --purge[=N_DAYS]
	      purge expired files; optional argument  'N_DAYS'	overrides  'expire_age'
	      value from the configuration file (Examples: -g90, --purge=90)

	      By  default,  purging is disabled ('expire_age' is set to '0' in the con‐
	      figuration file). To enable, set the 'expire_age' value  in  your  config
	      file to a value greater than '0'

	      You  can use '-vvg' to see when the remaining files in the waste directo‐
	      ries will expire.

       -o, --orphaned
	      check for orphaned files (maintenance)

	      An orphan is an item in a  waste	directory  that  has  no  corresponding
	      .trashinfo file, or vice versa. This option is intended primarily for de‐
	      velopers. Orphans may happen while testing code changes or if rmw is  un‐
	      intentionally released with a bug.
	      (see also: <https://remove-to-waste.info/faq.html#dot_trashinfo>)

       -f, --force
	      allow purging of expired files

	      rmw  will  refuse to purge directories if they contain non-writable files
	      or subdirectories. rmw will show a message that tells you "permission de‐
	      nied;  directory	still  contains files". To override, you can re-run rmw
	      using '-ffg'.

	      By default, force is not required to enable the  purge  feature.	If  you
	      would like to require it, add 'force_required' to your config file.

       -e, --empty
	      completely empty (purge) all waste directories

       -r, -R, --recursive
	      option  used for compatibility with rm (recursive operation is enabled by
	      default)

       -v, --verbose
	      increase output messages

       -w, --warranty
	      display warranty

       -V, --version
	      display version and license information

   RESTORING
       -z, --restore FILE(s)

	      To restore items, specify the path to them in the <WASTE>/files directory
	      (wildcards ok).

	      When restoring an item, if a file or directory with the same name already
	      exists at the destination, the item being restored will have a  time/date
	      string   (formatted   as	 "_%H%M%S-%y%m%d")   appended	to   it   (e.g.
	      'foo_164353-210508').

       -s, --select
	      select files from list to restore

	      Displays a list of items in your	waste  directories.  You  can  use  the
	      left/right cursor keys to switch between waste directories. Use the space
	      bar to select the items you wish to restore, then press enter to	restore
	      all selected items.

       -u, --undo-last
	      undo last move

	      Restores files that were last rmw'ed

       -m, --most-recent-list
	      list most recently rmw'ed files

ENVIRONMENT
       These  variables  are intended only to be used for testing. See the code-testing
       page on the rmw website for more details.

       RMW_FAKE_HOME

       RMW_FAKE_YEAR

       RMW_FAKE_MEDIA_ROOT

FILES
       On some systems, $HOME/.config  and  $HOME/.local/share	may  be  replaced  with
       $XDG_CONFIG_HOME and $XDG_DATA_HOME

       $HOME/.config/rmwrc
	      configuration file

       $HOME/.local/share/rmw/purge-time
	      text file that stores the time of the last purge

       $HOME/.local/share/rmw/mrl
	      text file containing a list of items that were last rmw'ed

NOTES
       rmw  will  not  move  items from one file system to another. If you try to rmw a
       file but don't have a waste directory configured that matches the file system on
       which it resides, rmw will refuse to do anything with it.

   DESKTOP INTEGRATION
       Items will be moved to a waste basket in the same manner as when using the "move
       to trash" option from your desktop GUI. They will be separated from your desktop
       trash  by  default; or if you wish for them to share the same "trash" directory,
       uncomment the line (in your config file):

       (Note that this does not apply to MacOS; while rmw is yet  unable  to  integrate
       with  the desktop trash directory, you'll still be able to use the default Waste
       directory.)

	      WASTE = $HOME/.local/share/Trash

       then comment out the line

	      WASTE = $HOME/.local/share/Waste

       You can reverse which directories are enabled at any time  if  you  ever  change
       your  mind. If both directories are on the same filesystem, rmw will use the di‐
       rectory listed first in your config file.

       It can be beneficial to have them both uncommented. If your desktop trash direc‐
       tory   (~/.local/share/Trash)   is   listed   after   the  rmw  default	(~/.lo‐
       cal/share/Waste) and uncommented, rmw will place newly rmw'ed items into the de‐
       fault, and it will purge expired files from both.

       When  rmw'ing  an item, if a file or directory with the same name already exists
       in the waste (or trash) directory, it will not be overwritten; instead, the cur‐
       rent   file   being   rmw'ed   will   have  a  time/date  string  (formatted  as
       "_%H%M%S-%y%m%d") appended to it (e.g. 'foo_164353-210508').

   REMOVABLE MEDIA
       The first time rmw is run, it will create a configuration file.	Waste  directo‐
       ries  will  be created automatically (Except for when the ',removable' option is
       used; see below) e.g., if '$HOME/.local/share/Waste' is uncommented in the  con‐
       fig file, these two directories will be created:

	      $HOME/.local/share/Waste/files
	      $HOME/.local/share/Waste/info

       If  a  WASTE  directory	is on removable media, you may append ',removable'.  In
       that case, rmw will not try to create it; it must be initially created manually.
       When rmw runs, it will check to see if the directory exists (which means the re‐
       movable media containing the directory is currently mounted). If rmw can't  find
       the  directory,	it  is assumed the media containing the directory isn't mounted
       and that directory will not be used for the current run of rmw.

       With the media mounted, once you manually create the waste  directory  for  that
       device (e.g. "/mnt/flash/.Trash-$UID") and run rmw, it will automatically create
       the two required child directories "files" and "info".

EXAMPLES
   RESTORING
       rmw -z ~/.local/share/Waste/files/foo
       rmw -z ~/.local/share/Waste/files/bars*

   CONFIGURATION
       WASTE=/mnt/flash/.Trash-$UID, removable
	      When using the removable attribute, you must also manually create the di‐
	      rectory

       expire_age = 45
	      rmw  will permanently delete files that have been in the waste (or trash)
	      for more than 45 days.

AUTHORS
       Project Manager: Andy Alt
       The RMW team: see AUTHORS.md

REPORTING BUGS
       Report bugs to <https://github.com/theimpossibleastronaut/rmw/issues>.

COPYRIGHT
       Copyright © 2012-2021 Andy Alt

       License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.
       This is free software: you are free to change and redistribute it.  There is  NO
       WARRANTY, to the extent permitted by law.
```

## Screenshots

See the [Screenshots](https://remove-to-waste.info/screenshots.html)
page on the website.

## Contact / Support

* [Bug Reports and Feature Requests](https://github.com/theimpossibleastronaut/rmw/blob/master/CONTRIBUTING.md#bug-reports-and-feature-requests)
* [General Help, Support, Discussion](https://remove-to-waste.info/#support)

## Installation

[![Packaging status](https://repology.org/badge/vertical-allrepos/rmw.svg)](https://repology.org/project/rmw/versions)

rmw is available in the [homebrew and
linuxbrew](https://github.com/Homebrew/) repositories; or there may may
be a binary package available for your OS. You can view a list at
[Repology](https://repology.org/project/rmw/versions) to see in which
repositories rmw is included.

Maintainer-created amd64 Debian packages are available in the [releases
section](https://github.com/theimpossibleastronaut/rmw/releases).

## Installing from source

### Required libraries

* libncursesw (ncurses-devel on some systems, such as CentOS)
* gettext (or use --disable-nls if you only need English language support)

If you're building from source, you will need the libncursesw(5 or
6)-dev package from your operating system distribution. On some systems
just the ncurses packages is needed, and it's often already installed.

### Compiling

#### As a normal user:

(This examples places the generated files to a separate folder, but you can
run 'configure' from any directory you like.)

Use `../configure --help` to view available compile-time options.

```
    mkdir _build
    cd _build
    ../configure
    make
```

#### Installing without superuser privileges

If you would like to install rmw without superuser privileges, use a prefix
that you have write access to. Example:

    ../configure --prefix=$HOME/.local
    make
    make install

The rmw binary will be installed to `$HOME/.local/bin` and documentation to
`$HOME/.local/doc`.

### If configure fails

On **macOS**, you may get a message during 'configure' that the menu
library can't be found. The ncurses menu library isn't provided by
default but can be installed using `brew install ncurses`. Then run
'configure' like this:

    PKG_CONFIG_PATH="/usr/local/opt/ncurses/lib/pkgconfig" ../configure

Or you can install [rmw using
brew](https://formulae.brew.sh/formula/rmw).

### Uninstall / Cleaning up

* make uninstall (uninstalls the program if installed with 'make install`)
* make distclean (removes files in the build directory created by `configure` and 'make')
