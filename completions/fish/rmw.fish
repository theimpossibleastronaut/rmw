# This file contains the fish completion for rmw
#
# rmw is a trashcan/recycle bin utility for the command line
#
# More information about this tool on https://github.com/theimpossibleastronaut/rmw
#
# This file is intended to be released with rmw with various packaging managers

complete rmw -s h -l help -d 'show help for command line options' -f
complete rmw -s c -l config -d 'use an alternate configuration' -r
complete rmw -s l -l list -d 'list waste directories' -f
complete rmw -s g -l purge -d 'purge expired files'
complete rmw -s o -l orphaned -d 'check for orphaned files (maintenance)'  -f
complete rmw -s f -l force -d 'allow purging of expired files' -f
complete rmw -l empty -d 'completely empty (purge) all waste directories'  -f
complete rmw -l top-level-bypass -d 'bypass protection of top-level files'

complete rmw -s v -l verbose -d 'increase output messages' -f
complete rmw -l warranty -d 'display warranty' -f
complete rmw -l version -d 'display version and license information' -f

complete rmw -s z -l restore -d 'restore FILE(s) (see man page example)'
complete rmw -s s -l select -d 'select files from list to restore' -f
complete rmw -s u -l undo-last -d 'undo last move' -f
complete rmw -s m -l most-recent-list -d "list most recently rmw'ed files" -f

# Options kept for compatibility purpose that do not need to be completed
# they are listed here to avoid people wondering why they are missing
# complete rmw -s R -s r -l recursive -d 'kept for compatibility purpose with rm (recursive operation is enabled by default)'
# complete rmw -s i -l interactive -d 'Prompt for removal (not implemented)'

