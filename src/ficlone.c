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

#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_FICLONE
#include <dirent.h>
#include <linux/fs.h>
#include <linux/magic.h>
#include <sys/ioctl.h>
#include <sys/statfs.h>
#include <sys/xattr.h>

#ifndef BCACHEFS_SUPER_MAGIC
#define BCACHEFS_SUPER_MAGIC 0xca451a4e
#endif

#ifndef BTRFS_SUPER_MAGIC
#define BTRFS_SUPER_MAGIC 0x9123683E
#endif
#endif

#include "ficlone.h"
#include "messages.h"
#include "utils.h"

bool
is_ficlone_fs(const char *path)
{
#ifdef HAVE_FICLONE
  struct statfs buf;

  /* statfs() follows symlinks and resolves a mount point to the mounted
     filesystem. For a symlink we want the filesystem the link itself lives
     on, not its target's, so probe the parent directory. For everything else
     probe the path directly, so a mount point resolves to the mounted
     filesystem rather than its parent. Fall back to the parent directory only
     when a direct probe fails (e.g. an unreachable path). */
  struct stat lst;
  bool is_link = lstat(path, &lst) == 0 && S_ISLNK(lst.st_mode);

  if (is_link || statfs(path, &buf) == -1)
  {
    gchar *dir = g_path_get_dirname(path);
    int r = statfs(dir, &buf);
    if (r == -1)
    {
      diag(DIAG_WARN, "statfs '%s': %s\n", dir, strerror(errno));
      g_free(dir);
      return false;
    }
    g_free(dir);
  }

  return buf.f_type == BTRFS_SUPER_MAGIC ||
    buf.f_type == BCACHEFS_SUPER_MAGIC;
#else
  (void) path;
  return false;
#endif
}


#ifdef HAVE_FICLONE
/* Copy ownership, permissions (including the setuid/setgid/sticky bits),
   timestamps, and xattrs from an open source fd to an open dest fd. Failures
   are non-fatal (warn only) -- the data is already in place. fchown runs before
   fchmod so the high mode bits aren't cleared by the ownership change. For a
   directory, call this only after its children are in place, since adding
   entries updates the directory's mtime. */
static void
clone_metadata(int src_fd, int dest_fd, const struct stat *src_stat,
               const char *dest, bool is_dir)
{
  if (fchown(dest_fd, src_stat->st_uid, src_stat->st_gid) == -1)
    diag(DIAG_ERR, "fchown: %s\n", strerror(errno));
  /* A directory copy in the waste must stay owner-traversable/writable, or
     rmw could neither purge nor restore it (removing a directory's entries
     needs write permission on that directory). Files are preserved exactly --
     a read-only file is still unlinkable via its writable parent. */
  mode_t mode = src_stat->st_mode & 07777;
  if (is_dir)
    mode |= S_IRWXU;
  if (fchmod(dest_fd, mode) == -1)
    diag(DIAG_ERR, "fchmod: %s\n", strerror(errno));
  struct timespec times[2] = { src_stat->st_atim, src_stat->st_mtim };
  if (futimens(dest_fd, times) == -1)
    diag(DIAG_ERR, "futimens: %s\n", strerror(errno));

  ssize_t names_len = flistxattr(src_fd, NULL, 0);
  if (names_len > 0)
  {
    char *names = malloc(names_len);
    if (names == NULL)
      fatal_malloc();
    if (flistxattr(src_fd, names, names_len) == names_len)
    {
      for (char *name = names; name < names + names_len; name += strlen(name) + 1)
      {
        ssize_t val_len = fgetxattr(src_fd, name, NULL, 0);
        if (val_len < 0)
          continue;
        if (val_len == 0)
        {
          if (fsetxattr(dest_fd, name, "", 0, 0) == -1)
            diag(DIAG_WARN, "fsetxattr '%s' on '%s': %s\n",
                 name, dest, strerror(errno));
          continue;
        }
        char *val = malloc(val_len);
        if (val == NULL)
          fatal_malloc();
        if (fgetxattr(src_fd, name, val, val_len) == val_len)
        {
          if (fsetxattr(dest_fd, name, val, val_len, 0) == -1)
            diag(DIAG_WARN, "fsetxattr '%s' on '%s': %s\n",
                 name, dest, strerror(errno));
        }
        free(val);
      }
    }
    free(names);
  }
}


/* Clone a single regular file from source to dest, preserving timestamps,
   ownership, and xattrs. Does NOT remove the source. On a clone failure the
   partial dest is removed. Returns 0 on success, -1 on failure (errno set). */
static int
clone_file(const char *source, const char *dest)
{
  int err;
  int src_fd = open(source, O_RDONLY);
  if (src_fd == -1)
  {
    err = errno;
    diag(DIAG_ERR, "open source: %s\n", strerror(err));
    errno = err;
    return -1;
  }

  struct stat src_stat;
  if (fstat(src_fd, &src_stat) == -1)
  {
    err = errno;
    diag(DIAG_ERR, "fstat source: %s\n", strerror(err));
    close(src_fd);
    errno = err;
    return -1;
  }

  mode_t old_umask = umask(0);
  int dest_fd = open(dest, O_WRONLY | O_CREAT, src_stat.st_mode & 0777);
  umask(old_umask);
  if (dest_fd == -1)
  {
    err = errno;
    diag(DIAG_ERR, "open destination: %s\n", strerror(err));
    close(src_fd);
    errno = err;
    return -1;
  }

  int res = ioctl(dest_fd, FICLONE, src_fd);
  err = errno;

  if (res != -1)
    clone_metadata(src_fd, dest_fd, &src_stat, dest, false);

  close(src_fd);
  close(dest_fd);

  if (res == -1)
  {
    if (err != EXDEV)
      diag(DIAG_ERR, "ioctl: %s in %s\n", strerror(err), __func__);
    if (unlink(dest) != 0)
      diag(DIAG_ERR, "unlink: %s in %s\n", strerror(errno), __func__);
    errno = err;
    return -1;
  }

  return 0;
}
#endif


static int
do_ficlone(const char *source, const char *dest)
{
#ifdef HAVE_FICLONE
  if (clone_file(source, dest) != 0)
    return -1;

  if (unlink(source) == -1)
  {
    int err = errno;
    diag(DIAG_ERR, "unlink source: %s\n", strerror(err));
    /* dest is a valid clone but source couldn't be removed; clean up dest
       so a file is never left only in the waste when rmw couldn't remove
       the original */
    if (unlink(dest) != 0)
      diag(DIAG_ERR, "unlink: %s in %s\n", strerror(errno), __func__);
    errno = err;
    return -1;
  }

  return 0;
#else
  (void) source;
  (void) dest;
  errno = EXDEV;
  return -1;
#endif
}


#ifdef HAVE_FICLONE
/* Recursively remove a directory tree. Returns 0 on success, -1 on the first
   failure (errno set). */
static int
remove_tree(const char *path)
{
  DIR *dir = opendir(path);
  if (!dir)
    return -1;

  int result = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL)
  {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    struct stat st;
    if (fstatat(dirfd(dir), entry->d_name, &st, AT_SYMLINK_NOFOLLOW) != 0)
    {
      result = -1;
      break;
    }

    if (S_ISDIR(st.st_mode))
    {
      char child[PATH_MAX];
      sn_check(snprintf(child, sizeof child, "%s/%s", path, entry->d_name),
               sizeof child);
      result = remove_tree(child);
    }
    else if (unlinkat(dirfd(dir), entry->d_name, 0) != 0)
      result = -1;

    if (result != 0)
      break;
  }

  int saved_err = errno;
  closedir(dir);
  if (result != 0)
  {
    errno = saved_err;
    return -1;
  }

  if (rmdir(path) != 0)
    return -1;

  return 0;
}


/* Recursively clone a directory tree from src to dst WITHOUT removing any
   source. Returns 0 on success, -1 on the first failure (errno set); the
   caller removes the partial dst to roll back. */
static int
clone_tree(const char *src, const char *dst)
{
  DIR *dir = opendir(src);
  if (!dir)
    return -1;

  if (mkdir(dst, 0777) != 0)
  {
    int err = errno;
    closedir(dir);
    errno = err;
    return -1;
  }

  struct stat src_st;
  bool have_src_st = fstat(dirfd(dir), &src_st) == 0;

  int result = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL)
  {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    char src_child[PATH_MAX];
    sn_check(snprintf(src_child, sizeof src_child, "%s/%s", src, entry->d_name),
             sizeof src_child);

    struct stat st;
    if (fstatat(dirfd(dir), entry->d_name, &st, AT_SYMLINK_NOFOLLOW) != 0)
    {
      diag(DIAG_ERR, "fstatat '%s': %s\n", src_child, strerror(errno));
      result = -1;
    }
    else
    {
      char dst_child[PATH_MAX];
      sn_check(snprintf(dst_child, sizeof dst_child, "%s/%s", dst, entry->d_name),
               sizeof dst_child);

      if (S_ISDIR(st.st_mode))
        result = clone_tree(src_child, dst_child);
      else if (S_ISLNK(st.st_mode))
      {
        char link_target[PATH_MAX + 1];
        ssize_t len =
          readlinkat(dirfd(dir), entry->d_name, link_target, PATH_MAX);
        if (len == -1)
        {
          diag(DIAG_ERR, "readlinkat '%s': %s\n", src_child, strerror(errno));
          result = -1;
        }
        else if (len == PATH_MAX)
        {
          diag(DIAG_ERR, "symlink target too long: '%s'\n", src_child);
          errno = ENAMETOOLONG;
          result = -1;
        }
        else
        {
          link_target[len] = '\0';
          if (symlink(link_target, dst_child) != 0)
          {
            diag(DIAG_ERR, "symlink '%s': %s\n", dst_child, strerror(errno));
            result = -1;
          }
        }
      }
      else if (S_ISREG(st.st_mode))
        result = clone_file(src_child, dst_child);
      else
      {
        /* special files (FIFOs, sockets, devices) can't be cloned */
        errno = ENOTSUP;
        result = -1;
      }
    }

    if (result != 0)
      break;
  }

  int saved_err = errno;

  if (result == 0 && have_src_st)
  {
    /* Apply the source directory's own metadata now that its contents are in
       place; populating it above reset the mode (umask) and bumped its mtime. */
    int dst_fd = open(dst, O_RDONLY | O_DIRECTORY);
    if (dst_fd != -1)
    {
      clone_metadata(dirfd(dir), dst_fd, &src_st, dst, true);
      close(dst_fd);
    }
  }

  closedir(dir);
  if (result != 0)
  {
    errno = saved_err;
    return -1;
  }

  return 0;
}
#endif


static int
do_ficlone_dir(const char *src, const char *dst)
{
#ifdef HAVE_FICLONE
  /* Copy the whole tree first without touching the source, so a clone
     failure rolls back cleanly by removing the partial copy. The source is
     removed only once the copy is complete (coreutils mv across file systems
     works the same way). */
  if (clone_tree(src, dst) != 0)
  {
    int err = errno;
    if (remove_tree(dst) != 0)
      diag(DIAG_WARN, "could not remove the partial copy '%s': %s\n",
           dst, strerror(errno));
    errno = err;
    return -1;
  }

  if (remove_tree(src) != 0)
  {
    int err = errno;
    diag(DIAG_WARN,
         _("'%s' was copied to the waste but the original could not be fully \
removed; the copy is safe, remove the leftover source manually\n"), src);
    errno = err;
    return -1;
  }

  return 0;
#else
  (void) src;
  (void) dst;
  errno = EXDEV;
  return -1;
#endif
}


/* Move src to dst using FICLONE where possible.
   Handles regular files, directories, and symlinks.
   Returns 0 on success, -1 on failure with errno set. */
int
ficlone_move(const char *src, const char *dst)
{
  struct stat st;
  if (lstat(src, &st) == -1)
    return -1;

  if (S_ISREG(st.st_mode))
    return do_ficlone(src, dst);

  if (S_ISDIR(st.st_mode))
    return do_ficlone_dir(src, dst);

  if (S_ISLNK(st.st_mode))
  {
    gchar *src_dir = g_path_get_dirname(src);
    int src_dir_fd = open(src_dir, O_RDONLY | O_DIRECTORY);
    g_free(src_dir);
    if (src_dir_fd == -1)
      return -1;

    gchar *src_name = g_path_get_basename(src);

    char target[PATH_MAX + 1];
    ssize_t len = readlinkat(src_dir_fd, src_name, target, PATH_MAX);
    if (len == -1)
    {
      g_free(src_name);
      close(src_dir_fd);
      return -1;
    }
    if (len == PATH_MAX)
    {
      g_free(src_name);
      close(src_dir_fd);
      errno = ENAMETOOLONG;
      return -1;
    }
    target[len] = '\0';
    if (symlink(target, dst) != 0)
    {
      g_free(src_name);
      close(src_dir_fd);
      return -1;
    }
    if (unlinkat(src_dir_fd, src_name, 0) != 0)
    {
      int err = errno;
      unlink(dst);
      g_free(src_name);
      close(src_dir_fd);
      errno = err;
      return -1;
    }
    g_free(src_name);
    close(src_dir_fd);
    return 0;
  }

  errno = ENOTSUP;
  return -1;
}
