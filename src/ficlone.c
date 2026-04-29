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

#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_FICLONE
#include <dirent.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/magic.h>
#include <sys/ioctl.h>
#include <sys/statfs.h>

#ifndef BCACHEFS_SUPER_MAGIC
#define BCACHEFS_SUPER_MAGIC 0xca451a4e
#endif
#endif

#include "ficlone.h"
#include "messages.h"

bool
is_ficlone_fs(const char *path)
{
#ifdef HAVE_FICLONE
  struct statfs buf;

  if (statfs(path, &buf) == -1)
  {
    print_msg_error();
    perror("statfs");
    exit(EXIT_FAILURE);
  }

  return buf.f_type == BTRFS_SUPER_MAGIC ||
    buf.f_type == BCACHEFS_SUPER_MAGIC;
#else
  (void) path;
  return false;
#endif
}


int
do_ficlone(const char *source, const char *dest, int *save_errno)
{
#ifdef HAVE_FICLONE
  int src_fd, dest_fd;
  struct stat src_stat;

  src_fd = open(source, O_RDONLY);
  if (src_fd == -1)
  {
    *save_errno = errno;
    perror("open source");
    return -1;
  }

  if (fstat(src_fd, &src_stat) == -1)
  {
    *save_errno = errno;
    perror("fstat source");
    close(src_fd);
    return -1;
  }

  mode_t old_umask = umask(0);
  dest_fd = open(dest, O_WRONLY | O_CREAT, src_stat.st_mode & 0777);
  umask(old_umask);
  if (dest_fd == -1)
  {
    *save_errno = errno;
    perror("open destination");
    close(src_fd);
    return -1;
  }

  int res = ioctl(dest_fd, FICLONE, src_fd);
  *save_errno = errno;

  if (res != -1)
  {
    struct timespec times[2] = { src_stat.st_atim, src_stat.st_mtim };
    if (futimens(dest_fd, times) == -1)
      perror("futimens");
    if (fchown(dest_fd, src_stat.st_uid, src_stat.st_gid) == -1)
      perror("fchown");
  }

  close(src_fd);
  close(dest_fd);

  if (res == -1)
  {
    if (*save_errno != EXDEV)
      fprintf(stderr, "ioctl: %s in %s\n", strerror(*save_errno), __func__);
    if (unlink(dest) != 0)
      fprintf(stderr, "unlink: %s in %s\n", strerror(errno), __func__);
    return -1;
  }

  if (unlink(source) == -1)
  {
    *save_errno = errno;
    perror("unlink source");
    /* dest is a valid clone but source couldn't be removed; clean up dest
       so the caller can retry rather than leaving an orphan in the waste folder */
    if (unlink(dest) != 0)
      fprintf(stderr, "unlink: %s in %s\n", strerror(errno), __func__);
    return -1;
  }

  return 0;
#else
  (void) source;
  (void) dest;
  *save_errno = EXDEV;
  return -1;
#endif
}


/* Recursively move a directory using FICLONE per file.
   Returns 0 on success. On failure sets *save_errno and returns -1.
   If FICLONE is not available, sets *save_errno = EXDEV (skip signal). */
int
do_ficlone_dir(const char *src, const char *dst, int *save_errno)
{
#ifdef HAVE_FICLONE
  DIR *dir = opendir(src);
  if (!dir)
  {
    *save_errno = errno;
    return -1;
  }

  if (mkdir(dst, 0777) != 0)
  {
    *save_errno = errno;
    closedir(dir);
    return -1;
  }

  int result = 0;
  int files_moved = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL)
  {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    char src_child[PATH_MAX];
    snprintf(src_child, sizeof src_child, "%s/%s", src, entry->d_name);

    struct stat st;
    if (lstat(src_child, &st) != 0)
    {
      *save_errno = errno;
      fprintf(stderr, "lstat '%s': %s\n", src_child, strerror(*save_errno));
      result = -1;
    }
    else
    {
      char dst_child[PATH_MAX];
      snprintf(dst_child, sizeof dst_child, "%s/%s", dst, entry->d_name);

      if (S_ISDIR(st.st_mode))
      {
        result = do_ficlone_dir(src_child, dst_child, save_errno);
      }
      else if (S_ISLNK(st.st_mode))
      {
        char link_target[PATH_MAX];
        ssize_t len = readlink(src_child, link_target, sizeof(link_target) - 1);
        if (len == -1)
        {
          *save_errno = errno;
          fprintf(stderr, "readlink '%s': %s\n", src_child, strerror(*save_errno));
          result = -1;
        }
        else
        {
          link_target[len] = '\0';
          if (symlink(link_target, dst_child) != 0)
          {
            *save_errno = errno;
            fprintf(stderr, "symlink '%s': %s\n", dst_child, strerror(*save_errno));
            result = -1;
          }
          else if (unlink(src_child) != 0)
          {
            *save_errno = errno;
            fprintf(stderr, "unlink '%s': %s\n", src_child, strerror(*save_errno));
            /* src_child is still intact; remove dst_child to avoid duplicate */
            unlink(dst_child);
            result = -1;
          }
        }
      }
      else if (S_ISREG(st.st_mode))
      {
        result = do_ficlone(src_child, dst_child, save_errno);
      }
      else
      {
        /* special files (FIFOs, sockets, devices) can't be cloned */
        *save_errno = ENOTSUP;
        result = -1;
      }
    }

    if (result != 0)
      break;
    files_moved++;
  }

  closedir(dir);

  if (result != 0)
  {
    if (files_moved > 0)
      fprintf(stderr,
              "partial move: check both '%s' and '%s' -- some files may have already been moved\n",
              src, dst);
    return -1;
  }

  if (rmdir(src) != 0)
  {
    *save_errno = errno;
    return -1;
  }
  return 0;
#else
  (void) src;
  (void) dst;
  *save_errno = EXDEV;
  return -1;
#endif
}


/* Move src to dst on a FICLONE-capable filesystem.
   Directories use do_ficlone_dir (returns EXDEV if clone not supported).
   Symlinks are recreated at dst and unlinked from src.
   Returns 0 on success, -1 on failure with errno set. */
int
ficlone_move(const char *src, const char *dst)
{
  struct stat st;
  if (lstat(src, &st) == -1)
    return -1;

  if (S_ISDIR(st.st_mode))
  {
    int clone_errno = 0;
    int r = do_ficlone_dir(src, dst, &clone_errno);
    if (r != 0)
      errno = clone_errno;
    return r;
  }

  if (S_ISLNK(st.st_mode))
  {
    char target[PATH_MAX];
    ssize_t len = readlink(src, target, sizeof(target) - 1);
    if (len == -1)
      return -1;
    target[len] = '\0';
    if (symlink(target, dst) != 0)
      return -1;
    if (unlink(src) != 0)
      return -1;
    return 0;
  }

  errno = ENOTSUP;
  return -1;
}
