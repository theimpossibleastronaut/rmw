/*
This file is part of rmw<https://theimpossibleastronaut.github.io/rmw-website/>

Copyright (C) 2012-2024  Andy Alt (arch_stanton5995@proton.me)
Other authors: https://github.com/theimpossibleastronaut/rmw/blob/master/AUTHORS.md

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _INC_BTRFS_H
#define _INC_BTRFS_H

#include <fcntl.h>
#include <linux/btrfs.h>
#include <linux/btrfs_tree.h>
#include <sys/ioctl.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>


#define BTRFS_SUPER_MAGIC 0x9123683E

bool is_btrfs(const char *path);

int do_btrfs_move(const char *source, const char *dest);

int is_btrfs_subvolume(const char *path);

#endif
