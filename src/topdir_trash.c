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
 * Filesystem types that should not get a NEW trash directory. Two groups,
 * handled differently during discovery:
 *
 * Pseudo / kernel-virtual (pseudo_fs_list): proc, sysfs, devtmpfs, devpts,
 * cgroup*, bpf, securityfs, debugfs, tracefs, pstore, mqueue, hugetlbfs,
 * fusectl, configfs, autofs, binfmt_misc, rpc_pipefs, nsfs, efivarfs. These
 * can never hold a real file, so discovery skips them entirely -- it does
 * not even probe them for an existing trash. Probing is pointless and, on
 * some (e.g. bpf), lstat() of an arbitrary path fails with EPERM, which root
 * would hit since access() is bypassed for the superuser.
 *
 * Excluded but file-capable (extra_exclude_list): tmpfs (reboot wipes it),
 * overlay, squashfs, and the network shares the spec leaves undefined for
 * lack of a numeric uid model (cifs, smbfs, smb3). All "fuse.*" types are
 * excluded too. These never receive a NEW trash, but an existing one is
 * honoured, so they ARE probed.
 *
 * NFS is NOT excluded: it has a native uid model and the spec permits it.
 */
static const char *const pseudo_fs_list[] = {
  "proc", "sysfs", "devtmpfs", "devpts",
  "cgroup", "cgroup2", "bpf", "securityfs", "debugfs", "tracefs",
  "pstore", "mqueue", "hugetlbfs", "fusectl", "configfs", "autofs",
  "binfmt_misc", "rpc_pipefs", "nsfs", "efivarfs",
  NULL,
};

static const char *const extra_exclude_list[] = {
  "tmpfs", "overlay", "squashfs",
  "cifs", "smbfs", "smb3",
  NULL,
};

static bool
fs_type_in_list(const char *fs_type, const char *const *list)
{
  for (const char *const *p = list; *p != NULL; p++)
    if (g_str_equal(fs_type, *p))
      return true;
  return false;
}

static bool
fs_type_is_pseudo(const char *fs_type)
{
  return fs_type != NULL && fs_type_in_list(fs_type, pseudo_fs_list);
}

static bool
fs_type_is_eligible(const char *fs_type)
{
  if (fs_type == NULL)
    return false;
  if (g_str_has_prefix(fs_type, "fuse."))
    return false;
  return !fs_type_is_pseudo(fs_type)
    && !fs_type_in_list(fs_type, extra_exclude_list);
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
    /* Any error other than "not found" (e.g. EACCES/EPERM on an unusual
       mount) just means there is no usable shared trash here. Note it under
       -vv for debugging, but don't error out during best-effort discovery. */
    verbose_printf(2, "lstat %s: %s\n", dot_trash, strerror(errno));
    free(dot_trash);
    return NULL;
  }

  free(dot_trash);

  char dot_trash_uid[PATH_MAX];
  sn_check(snprintf(dot_trash_uid, sizeof dot_trash_uid, ".Trash-%s", uid),
           sizeof dot_trash_uid);
  return join_paths(topdir, dot_trash_uid);
}


/* True if mount_path is "/" or a whole-component prefix of real_path
 * ("/a/b" prefixes "/a/b/c" but not "/a/bc"). */
static bool
mount_path_prefixes(const char *mount_path, size_t len,
                    const char *real_path)
{
  if (strncmp(real_path, mount_path, len) != 0)
    return false;
  return len == 1 || real_path[len] == '/' || real_path[len] == '\0';
}

char *
find_topdir_trash(const char *file_path, const char *uid,
                  char **mount_path_out)
{
  if (mount_path_out != NULL)
    *mount_path_out = NULL;

  /* g_unix_mount_for() locates a path's mount by walking up until st_dev
   * changes, which walks straight past a bind mount (same device as its
   * source) and lands on the parent mount. Instead, take the mount table
   * entry whose path is the longest prefix of the file's canonical path. */
  char *real = realpath(file_path, NULL);
  if (real == NULL)
    return NULL;

  /* g_unix_mounts_get/g_unix_mount_get_mount_path/g_unix_mount_free were
   * deprecated in GLib 2.78 in favour of g_unix_mount_entry_* equivalents.
   * Suppress warnings until we can set the project minimum to 2.78. */
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GList *mounts = g_unix_mounts_get(NULL);

  GUnixMountEntry *best = NULL;
  const char *topdir = NULL;
  size_t best_len = 0;
  for (GList *l = mounts; l != NULL; l = l->next)
  {
    const char *mp = g_unix_mount_get_mount_path(l->data);
    size_t len = strlen(mp);
    /* >= so the most recent entry wins when a path is overmounted */
    if (len >= best_len && mount_path_prefixes(mp, len, real))
    {
      best_len = len;
      best = l->data;
      topdir = mp;
    }
  }

  /* Never create a trash dir on an excluded filesystem (tmpfs, network
   * shares, ...). Desktop file managers refuse to trash there too; the
   * caller then reports that no suitable waste folder exists. An existing
   * trash on such a mount is still honored (see below). */
  char *result = NULL;
  if (best != NULL)
  {
    char *candidate = trash_path_for_topdir(topdir, uid);
    /* New trash dirs are only created on eligible mounts. On an excluded
       mount (tmpfs, network share) rmw never creates one, but an existing,
       writable trash there is still honored -- the same rule discovery
       (build_mount_trash_list) applies, scoped here to the file's own mount
       so no other mount is probed (issue #574). */
    if (candidate != NULL
        && (mount_is_eligible(best) || access(candidate, W_OK) == 0))
    {
      result = candidate;
      if (mount_path_out != NULL)
        *mount_path_out = strdup(topdir);
    }
    else
      free(candidate);
  }

  g_list_free_full(mounts, (GDestroyNotify) g_unix_mount_free);
  G_GNUC_END_IGNORE_DEPRECATIONS
  free(real);

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

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    const char *mount_path = g_unix_mount_get_mount_path(entry);
    G_GNUC_END_IGNORE_DEPRECATIONS

    /* Skip mount points that aren't directories. Containers (and some
     * systemd setups) bind-mount single files like /etc/resolv.conf;
     * a trash dir can't live under a file, and probing one would error. */
    struct stat mp_st;
    if (lstat(mount_path, &mp_st) != 0 || !S_ISDIR(mp_st.st_mode))
      continue;

    if (!mount_is_eligible(entry))
    {
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      const char *fs_type = g_unix_mount_get_fs_type(entry);
      G_GNUC_END_IGNORE_DEPRECATIONS
      /* Kernel-virtual filesystems can never hold a trash, so never probe
       * them -- and probing some (e.g. bpf) makes lstat() fail with EPERM,
       * which root hits since access() below is bypassed for the superuser. */
      if (fs_type_is_pseudo(fs_type))
        continue;
      /* New trash dirs are never created on excluded filesystems, but an
       * existing, writable one is evidence of intent (configured by the
       * user in the past, or created by an older rmw): keep it visible so
       * restore and purge can still reach its contents. access(W_OK)
       * covers existence, writability, and readonly mounts in one call.
       * Skip mount roots we can't read (docker overlays, ...) before
       * probing, or the probe spams lstat errors. */
      if (access(mount_path, R_OK | X_OK) != 0)
        continue;
      char *t = trash_path_for_topdir(mount_path, uid);
      bool keep = (t != NULL && access(t, W_OK) == 0);
      free(t);
      if (!keep)
        continue;
    }

    /* The mount that hosts the home trash already has a node at the head. */
    if (mp_st.st_dev == home_dev)
      continue;

    st_mount_trash *node = new_topdir_node(mount_path, uid, mp_st.st_dev);
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
