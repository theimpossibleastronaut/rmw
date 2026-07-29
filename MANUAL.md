RMW 1 "July 2026" "rmw 0.10.0" "User Commands"
==============================================

<!--
This file is the single source for BOTH the rmw(1) man page and the manual
page on the rmw website. HTML comments like this one are stripped from both
outputs, so they are safe to use as maintainer notes.

  * Man page: generated into rmw.1 with go-md2man. After editing this
    file, run `ninja -C <builddir> regen-man` and commit MANUAL.md and the
    regenerated rmw.1 together.
  * Website: copied into the rmw-website repo (Jekyll front matter added).

go-md2man formatting rules to keep in mind:
  * '# X' becomes a .SH section; '## X' becomes a .SS subsection.
  * An option/term line followed by ': description' becomes a .TP entry;
    indent continuation paragraphs by 4 spaces to keep them under the term.
  * Line breaks come from a BLANK LINE between lines (a new paragraph). The
    blank lines between the "rmw ..." SYNOPSIS forms and the two AUTHORS
    lines are intentional breaks -- do NOT collapse them onto one line, and
    do not rely on trailing-space breaks (they get stripped by editors).
-->

# NAME

rmw - safe-remove utility for the command line

# SYNOPSIS

**rmw** [*OPTION*]... *FILE*...

Move FILE(s) to a WASTE directory listed in configuration file

<!-- The blank lines between these three forms are intentional line breaks. -->
**rmw** `-s`

**rmw** `-u`

**rmw** `-z` *FILE*...

Restore FILE(s) from a WASTE directory

# DESCRIPTION

rmw (ReMove to Waste) is a trashcan/recycle bin utility for the command line.
It can move and restore files to and from directories specified in a
configuration file, and can also be integrated with your regular desktop trash
folder (if your desktop environment uses the FreeDesktop.org Trash
specification). One of the unique features of rmw is the ability to purge
items from your waste (or trash) directories after x number of days.

# OPTIONS

`-h`, `--help`
: show help for command line options

`-c`, `--config` *FILE*
: use an alternate configuration

`-l`, `--list`
: list waste directories, one path per line

    Add `-v` to also show each folder's attributes (such as "no-add")
    and a list of candidate trash locations that rmw would create on other
    file systems when needed.

`-g[N_DAYS]`, `--purge`[=*N_DAYS*]
: purge expired files; optional argument 'N_DAYS' overrides 'expire_age'
    value from the configuration file (Examples: `-g90`, `--purge`=*90*)

    By default, purging is disabled ('expire_age' is set to '0' in the
    configuration file). To enable, set the 'expire_age' value in your
    config file to a value greater than '0'

    You can use '-vvg' to see when the remaining files in the waste
    directories will expire.

    The argument must be attached to the option, as in the examples
    above. If a space separates them, rmw reads 'N_DAYS' as the name of
    a file to remove.

    To purge from a scheduled job, keep 'expire_age' at '0' so that
    normal runs never purge, and let a cron job give the number of days:

        30 4 * * 0 rmw -g45 >> "$HOME/.local/state/rmw-purge.log" 2>&1

    See the FAQ for the full example, including log rotation:
    <https://theimpossibleastronaut.github.io/rmw-website/faq.html#can-rmw-be-run-as-a-scheduled-job-to-purge-expired-files>

`-o`, `--orphaned`
: check for orphaned files (maintenance)

    An orphan is an item in a waste directory that has no corresponding
    **.trashinfo** file, or vice versa. This option is intended primarily
    for developers. Orphans may happen while testing code changes or if rmw
    is unintentionally released with a bug.

    A file that has no **.trashinfo** is repaired by `-o` alone (a new
    **.trashinfo** is created for it). Deleting the opposite kind of orphan, a
    **.trashinfo** with no matching file, is only done during a purge and
    requires force to be given twice, as in **rmw -offg** (`-o -f -f -g`).
    (see also:
    <https://theimpossibleastronaut.github.io/rmw-website/faq.html#dot_trashinfo>)

`-f`, `--force`
: allow purging of expired files

    By default, force is not required to enable the purge feature. If you
    would like to require it, add 'force_required' to your config file.

`--empty`
: completely empty (purge) all waste directories

`-r`, `-R`, `--recursive`
: option used for compatibility with rm (recursive operation is enabled by
    default)

`--top-level-bypass`
: bypass protection of top-level files (added in v0.9.0)

`-v`, `--verbose`
: increase output messages

`-w`, `--warranty`
: display warranty

`-V`, `--version`
: display version and license information

## RESTORING

`-z`, `--restore` *FILE(s)*
: To restore items, specify the path to them in the *files* subdirectory of
    a waste folder (wildcards ok).

    When restoring an item, if a file or directory with the same name
    already exists at the destination, the item being restored will have a
    time/date string (formatted as "_%H%M%S-%y%m%d") appended to it
    (e.g. 'foo_164353-210508').

`-s`, `--select`
: select files from list to restore

    Displays a list of items in your waste directories. You can use the
    left/right cursor keys to switch between waste directories. Use the
    space bar to select the items you wish to restore, then press enter to
    restore all selected items.

    The list opens on the waste directory of your current filesystem. If
    no waste directory is on that filesystem, it opens on the first one in
    your list instead.

    When the output is not a terminal (for example, piped to another
    program), rmw does not open the menu. Instead it prints the path of
    that waste directory, followed by the names of the items in it.

`-u`, `--undo-last`
: undo last move

    Restores files that were last rmw'ed

`-m`, `--most-recent-list`
: list most recently rmw'ed files

# CONFIGURATION

Configuration is optional. With no configured WASTE folder, rmw moves each
file to the trash on the file's own filesystem: files on the home filesystem
go to the FreeDesktop home trash (~/.local/share/Trash, the same trash your
desktop uses), and files on other filesystems go to a trash at the top of
that filesystem. These are created when first needed.

Add WASTE lines only for special cases, such as keeping rmw's files separate
from the desktop trash (uncomment the **WASTE = $HOME/.local/share/Waste**
line in the generated config), a folder on removable media, or a **no-add**
folder. Each WASTE folder must be on its own line; two WASTE folders cannot
be specified on the same line.

**WASTE=/mnt/flash/.Trash-$UID, removable**
: When using the removable attribute, you must also manually create the
    directory

**WASTE = /path, no-add**
: The "no-add" attribute tells rmw to never move files into this folder.
    rmw still lists it, restores files from it, and purges old files in it.
    This is useful for a folder you want to empty over time but no longer add
    to. You can combine attributes, for example "removable, no-add".

**expire_age = 45**
: rmw will permanently delete files that have been in the waste (or
    trash) for more than 45 days.

# ENVIRONMENT

These variables are set by rmw's meson test suite to run the code in a
controlled environment. They are not meant for normal use, and setting them
yourself is strongly discouraged. See the code-testing page on the rmw
website for more details.

**RMW_DISCOVERY**
: Set to "off" to stop rmw from scanning real mount points for $topdir
    trash folders. Any other value (or leaving it unset) keeps discovery on.

**RMW_FAKE_YEAR**
: Set to "true" to write the year 1999 as the deletion date, so the test
    suite can exercise the purge feature.

**RMW_FAKE_HOME**
: Deprecated; it will be removed in a future release. To choose a test home,
    set **HOME** (with **XDG_DATA_HOME** and **XDG_CONFIG_HOME**) yourself; to
    control mount discovery, use **RMW_DISCOVERY** instead.

# FILES

On some systems, $HOME/.config and $HOME/.local/share may be replaced
with $XDG_CONFIG_HOME and $XDG_DATA_HOME

*$HOME/.config/rmwrc*
: configuration file

*$HOME/.local/share/rmw/purge-time*
: text file that stores the time of the last purge

*$HOME/.local/share/rmw/mrl*
: text file containing a list of items that were last rmw'ed

# NOTES

rmw does not move items from one file system to another. When you rmw a
file, it looks for a waste directory on the same file system. It checks
the waste directories in your configuration file first. If none match, it
uses the trash for that file system, as described by the FreeDesktop.org
Trash specification: the home trash (~/.local/share/Trash) for files on the
home file system, or a trash at the top of the file system (for example,
"/mnt/disk/.Trash-1000") for other file systems. If one already exists, rmw
uses it; if not, rmw creates it. (Moves between btrfs or bcachefs subvolumes
are the exception; see BTRFS AND BCACHEFS below.)

rmw does not create a trash directory on file systems that are not meant
to hold one, such as temporary (tmpfs) or network file systems. On those,
if no configured waste directory matches, rmw refuses to remove the file,
the same way desktop file managers do. A trash directory that already
exists on such a file system is still used.

Because the default home trash is the same one your desktop uses, enabling
purging (an **expire_age** greater than 0) lets rmw permanently delete old
items that your file manager placed in the trash too, not only items removed
with rmw. Purging is off by default.

## DESKTOP INTEGRATION

Items are moved to the trash in the same manner as the "move to trash"
option in your desktop file manager. By default rmw uses the same trash
your desktop uses (the FreeDesktop home trash, ~/.local/share/Trash), so an
item removed with rmw appears in your desktop trash and can be restored from
either one.

To keep rmw's files separate from the desktop trash instead, uncomment this
line in your configuration file:

    WASTE = $HOME/.local/share/Waste

(On macOS rmw does not integrate with the system trash; the default
~/.local/share/Trash is used.)

When rmw'ing an item, if a file or directory with the same name already
exists in the waste (or trash) directory, it will not be overwritten;
instead, the current file being rmw'ed will have a time/date string
(formatted as "_%H%M%S-%y%m%d") appended to it (e.g. 'foo_164353-210508').

## REMOVABLE MEDIA

The first time rmw is run, it will create a configuration file.
Waste directories will be created automatically (Except for when the
',removable' option is used; see below) e.g., if '$HOME/.local/share/Waste'
is uncommented in the config file, these two directories will be created:

    $HOME/.local/share/Waste/files
    $HOME/.local/share/Waste/info

If a WASTE directory is on removable media, you may append ',removable'.
In that case, rmw will not try to create it; it must be
initially created manually. When rmw runs, it will check to see if the
directory exists (which means the removable media containing the
directory is currently mounted). If rmw can't find the directory, it is
assumed the media containing the directory isn't mounted and that
directory will not be used for the current run of rmw.

With the media mounted, once you manually create the waste directory
for that device (e.g. "/mnt/flash/.Trash-$UID") and run rmw, it will
automatically create the two required child directories "files" and "info".

## BTRFS AND BCACHEFS

rmw supports moving files across subvolumes on btrfs and bcachefs
filesystems. Because the kernel reports subvolumes with different
device numbers, a cross-subvolume move would normally fail with a
cross-device error; rmw detects this case and performs a reflink
clone-then-delete instead, preserving the expected trash semantics.

To use this feature, define a WASTE directory on the destination
subvolume in your configuration file. If your waste directory is on
a different subvolume than the files you want to remove, rmw will
handle the move transparently.

# EXAMPLES

## RESTORING

    rmw -z ~/.local/share/Waste/files/foo
    rmw -z ~/.local/share/Waste/files/bars*

# AUTHORS

Project Manager: Andy Alt

The RMW team: see AUTHORS.md

# REPORTING BUGS

Report bugs to <https://github.com/theimpossibleastronaut/rmw/issues>.

# COPYRIGHT

Copyright © 2012-2026 Andy Alt

License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

# SEE ALSO

mv(1), rm(1), rmdir(1)

Full documentation at: <https://theimpossibleastronaut.github.io/rmw-website/>
