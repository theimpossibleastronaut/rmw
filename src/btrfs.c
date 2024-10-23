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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
#include "globals.h"
#endif

#include "btrfs.h"
#include "messages_rmw.h"

bool
is_btrfs(const char *path)
{
  struct statfs buf;

  if (statfs(path, &buf) == -1)
  {
    print_msg_error();
    perror("statfs");
    exit(EXIT_FAILURE);
  }

  return buf.f_type == BTRFS_SUPER_MAGIC;
}


int
do_btrfs_move(const char *source, const char *dest)
{
  int src_fd, dest_fd;

  src_fd = open(source, O_RDONLY);
  if (src_fd == -1)
  {
    perror("open source");
    return src_fd;
  }

  // Open or create the destination file
  dest_fd = open(dest, O_WRONLY | O_CREAT, 0666);
  if (dest_fd == -1)
  {
    perror("open destination");
    close(src_fd);
    return src_fd;
  }

  int res = ioctl(dest_fd, BTRFS_IOC_CLONE, src_fd);
  if (res == -1)
  {
    perror("BTRFS_IOC_CLONE");
    close(src_fd);
    close(dest_fd);
    return res;
  }

  // Close file descriptors
  close(src_fd);
  close(dest_fd);

  res = unlink(source);
  if (res == -1)
  {
    perror("unlink source");
    return res;
  }
  return 0;
}
