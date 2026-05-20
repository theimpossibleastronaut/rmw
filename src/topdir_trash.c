/*
This file is part of rmw<https://theimpossibleastronaut.github.io/rmw-website/>

Copyright (C) 2012-2026  Andy Alt (arch_stanton5995@proton.me)
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

#include <sys/stat.h>
#include <unistd.h>
#include <gio/gunixmounts.h>

#include "ficlone.h"
#include "topdir_trash.h"
#include "messages.h"
#include "strings_rmw.h"
#include "utils.h"


static char *trash_path_for_topdir(const char *topdir, const char *uid);


/*
 * Filesystem types that should not get a trash directory.
 *
 * Pseudo-filesystems (no real on-disk storage): proc, sysfs, devtmpfs,
 * devpts, cgroup*, bpf, securityfs, debugfs, tracefs, pstore, mqueue,
 * hugetlbfs, fusectl, configfs, autofs, binfmt_misc, rpc_pipefs, nsfs.
 *
 * Ephemeral or container-style: tmpfs (reboot wipes it), overlay, squashfs.
 *
 * Network shares the spec explicitly leaves undefined (no real numeric uid
 * model): cifs, smbfs, smb3. All "fuse.*" fs types are excluded too
 * (uid-mapping semantics vary too widely between fuse backends).
 *
 * NFS is NOT excluded: it has a native uid model and the spec permits it.
 */
static const char *const fs_type_exclude_list[] = {
  "proc", "sysfs", "devtmpfs", "devpts", "tmpfs",
  "cgroup", "cgroup2", "bpf", "securityfs", "debugfs", "tracefs",
  "pstore", "mqueue", "hugetlbfs", "fusectl", "configfs", "autofs",
  "binfmt_misc", "rpc_pipefs", "nsfs", "efivarfs",
  "overlay", "squashfs",
  "cifs", "smbfs", "smb3",
  NULL,
};

static bool
fs_type_is_eligible(const char *fs_type)
{
  if (fs_type == NULL)
    return false;
  if (g_str_has_prefix(fs_type, "fuse."))
    return false;
  for (const char *const *p = fs_type_exclude_list; *p != NULL; p++)
  {
    if (g_str_equal(fs_type, *p))
      return false;
  }
  return true;
}

static bool
mount_is_eligible(GUnixMountEntry *entry)
{
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  bool readonly = g_unix_mount_is_readonly(entry);
  const char *fs_type = g_unix_mount_get_fs_type(entry);
  const char *mount_path = g_unix_mount_get_mount_path(entry);
  G_GNUC_END_IGNORE_DEPRECATIONS
  if (readonly)
    return false;
  if (!fs_type_is_eligible(fs_type))
    return false;
  /* Skip mounts the current user cannot write to (e.g. /boot/efi).
   * access() resolves effective uid/gid against the actual ACL. */
  if (access(mount_path, W_OK) != 0)
    return false;
  return true;
}

static st_mount_trash *
alloc_mount_trash_node(void)
{
  st_mount_trash *node = malloc(sizeof *node);
  if (node == NULL)
    fatal_malloc();
  node->mount_path = NULL;
  node->trash_dir = NULL;
  node->files_dir = NULL;
  node->info_dir = NULL;
  node->dev_num = 0;
  node->is_home_trash = false;
  node->is_ficlone_fs = false;
  node->next = NULL;
  return node;
}

/* The home trash dir may not exist yet at startup. Walk up the path until
 * we find an existing ancestor so callers (is_ficlone_fs, stat) get a real
 * filesystem path to probe. Returns a newly-g_malloc'd string, or NULL if
 * no ancestor up to "/" exists. */
static char *
first_existing_ancestor(const char *path)
{
  char *probe = g_strdup(path);
  struct stat st;
  while (lstat(probe, &st) != 0)
  {
    char *parent = g_path_get_dirname(probe);
    if (g_str_equal(parent, probe))
    {
      g_free(probe);
      g_free(parent);
      return NULL;
    }
    g_free(probe);
    probe = parent;
  }
  return probe;
}

static st_mount_trash *
new_home_trash_node(const char *home_trash_dir, dev_t home_dev)
{
  st_mount_trash *node = alloc_mount_trash_node();
  node->trash_dir = strdup(home_trash_dir);
  node->files_dir = join_paths(home_trash_dir, "files");
  node->info_dir = join_paths(home_trash_dir, "info");
  node->dev_num = home_dev;
  node->is_home_trash = true;

  char *probe = first_existing_ancestor(home_trash_dir);
  if (probe != NULL)
  {
    node->is_ficlone_fs = is_ficlone_fs(probe);
    g_free(probe);
  }
  return node;
}

static st_mount_trash *
new_topdir_node(const char *mount_path, const char *uid, dev_t dev_num)
{
  char *trash_dir = trash_path_for_topdir(mount_path, uid);
  if (trash_dir == NULL)
    return NULL;

  st_mount_trash *node = alloc_mount_trash_node();
  node->mount_path = strdup(mount_path);
  node->trash_dir = trash_dir;
  node->files_dir = join_paths(trash_dir, "files");
  node->info_dir = join_paths(trash_dir, "info");
  node->dev_num = dev_num;
  node->is_ficlone_fs = is_ficlone_fs(mount_path);
  return node;
}


/*
 * Per FreeDesktop Trash spec section 1.1:
 * If $topdir/.Trash exists, is not a symlink, and has the sticky bit set,
 * return "$topdir/.Trash/$uid". Otherwise return "$topdir/.Trash-$uid".
 * Returns NULL on unexpected lstat failure. Caller must free.
 */
static char *
trash_path_for_topdir(const char *topdir, const char *uid)
{
  char *dot_trash = join_paths(topdir, ".Trash");

  struct stat st;
  if (lstat(dot_trash, &st) == 0)
  {
    if (S_ISLNK(st.st_mode))
      diag(DIAG_WARN, "%s is a symbolic link; ignoring per trash spec\n",
           dot_trash);
    else if (!(st.st_mode & S_ISVTX))
      diag(DIAG_WARN, "%s lacks the sticky bit; ignoring per trash spec\n",
           dot_trash);
    else
    {
      char *result = join_paths(dot_trash, uid);
      free(dot_trash);
      return result;
    }
  }
  else if (errno != ENOENT)
  {
    diag(DIAG_ERR, "lstat %s: %s\n", dot_trash, strerror(errno));
    free(dot_trash);
    return NULL;
  }

  free(dot_trash);

  char dot_trash_uid[PATH_MAX];
  sn_check(snprintf(dot_trash_uid, sizeof dot_trash_uid, ".Trash-%s", uid),
           sizeof dot_trash_uid);
  return join_paths(topdir, dot_trash_uid);
}


char *
find_topdir_trash(const char *file_path, const char *uid,
                  char **mount_path_out)
{
  if (mount_path_out != NULL)
    *mount_path_out = NULL;

  /* g_unix_mount_for/g_unix_mount_get_mount_path/g_unix_mount_free were
   * deprecated in GLib 2.78 in favour of g_unix_mount_entry_* equivalents.
   * Suppress warnings until we can set the project minimum to 2.78. */
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GUnixMountEntry *entry = g_unix_mount_for(file_path, NULL);
  if (!entry)
    return NULL;

  const char *topdir = g_unix_mount_get_mount_path(entry);
  char *result = trash_path_for_topdir(topdir, uid);
  if (result != NULL && mount_path_out != NULL)
    *mount_path_out = strdup(topdir);
  g_unix_mount_free(entry);
  G_GNUC_END_IGNORE_DEPRECATIONS

  return result;
}


st_mount_trash *
build_mount_trash_list(const char *uid,
                       const char *home_trash_dir,
                       dev_t home_dev)
{
  st_mount_trash *head = new_home_trash_node(home_trash_dir, home_dev);
  st_mount_trash *tail = head;

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GList *mounts = g_unix_mounts_get(NULL);
  G_GNUC_END_IGNORE_DEPRECATIONS

  for (GList *l = mounts; l != NULL; l = l->next)
  {
    GUnixMountEntry *entry = l->data;
    if (!mount_is_eligible(entry))
      continue;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    const char *mount_path = g_unix_mount_get_mount_path(entry);
    G_GNUC_END_IGNORE_DEPRECATIONS

    struct stat st;
    if (lstat(mount_path, &st) != 0)
      continue;

    /* The mount that hosts the home trash already has a node at the head. */
    if (st.st_dev == home_dev)
      continue;

    st_mount_trash *node = new_topdir_node(mount_path, uid, st.st_dev);
    if (node == NULL)
      continue;

    tail->next = node;
    tail = node;
  }

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  g_list_free_full(mounts, (GDestroyNotify) g_unix_mount_free);
  G_GNUC_END_IGNORE_DEPRECATIONS

  return head;
}


void
free_mount_trash_list(st_mount_trash *head)
{
  while (head != NULL)
  {
    st_mount_trash *next = head->next;
    free(head->mount_path);
    free(head->trash_dir);
    free(head->files_dir);
    free(head->info_dir);
    free(head);
    head = next;
  }
}
