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

/* Include the full translation unit to access trash_path_for_topdir (static). */
#include "../src/topdir_trash.c"

#include "test.h"
#include "purging.h"

#include <fcntl.h>


static void
test_trash_path_for_topdir(const char *tmpdir, const char *uid)
{
  char expected[PATH_MAX];

  /* no .Trash dir → .Trash-$uid */
  {
    char *result = trash_path_for_topdir(tmpdir, uid);
    sn_check(snprintf(expected, sizeof expected, "%s/.Trash-%s", tmpdir, uid),
             sizeof expected);
    assert(result != NULL);
    assert(strcmp(result, expected) == 0);
    free(result);
  }

  char dot_trash[PATH_MAX];
  sn_check(snprintf(dot_trash, sizeof dot_trash, "%s/.Trash", tmpdir),
           sizeof dot_trash);

  /* .Trash is a symlink → .Trash-$uid */
  {
    assert(symlink(tmpdir, dot_trash) == 0);
    char *result = trash_path_for_topdir(tmpdir, uid);
    sn_check(snprintf(expected, sizeof expected, "%s/.Trash-%s", tmpdir, uid),
             sizeof expected);
    assert(result != NULL);
    assert(strcmp(result, expected) == 0);
    free(result);
    assert(remove(dot_trash) == 0);
  }

  /* .Trash without sticky bit → .Trash-$uid */
  {
    assert(mkdir(dot_trash, 0755) == 0);
    char *result = trash_path_for_topdir(tmpdir, uid);
    sn_check(snprintf(expected, sizeof expected, "%s/.Trash-%s", tmpdir, uid),
             sizeof expected);
    assert(result != NULL);
    assert(strcmp(result, expected) == 0);
    free(result);
    assert(rmdir(dot_trash) == 0);
  }

  /* .Trash with sticky bit, not a symlink → .Trash/$uid */
  {
    assert(mkdir(dot_trash, 0755) == 0);
    /* mkdir strips the sticky bit on macOS. Set it through a descriptor so
       the path is not resolved a second time. */
    int fd = open(dot_trash, O_RDONLY | O_DIRECTORY);
    assert(fd >= 0);
    assert(fchmod(fd, 01755) == 0);
    assert(close(fd) == 0);
    char *result = trash_path_for_topdir(tmpdir, uid);
    sn_check(snprintf(expected, sizeof expected, "%s/.Trash/%s", tmpdir, uid),
             sizeof expected);
    assert(result != NULL);
    assert(strcmp(result, expected) == 0);
    free(result);
    assert(rmdir(dot_trash) == 0);
  }
}


static void
test_fs_type_is_eligible(void)
{
  assert(fs_type_is_eligible(NULL) == false);

  /* pseudo / system fs */
  assert(fs_type_is_eligible("proc") == false);
  assert(fs_type_is_eligible("sysfs") == false);
  assert(fs_type_is_eligible("tmpfs") == false);
  assert(fs_type_is_eligible("cgroup2") == false);
  assert(fs_type_is_eligible("overlay") == false);
  assert(fs_type_is_eligible("squashfs") == false);
  assert(fs_type_is_eligible("efivarfs") == false);

  /* network shares the spec leaves undefined */
  assert(fs_type_is_eligible("cifs") == false);
  assert(fs_type_is_eligible("smbfs") == false);
  assert(fs_type_is_eligible("smb3") == false);

  /* fuse.* prefix → excluded */
  assert(fs_type_is_eligible("fuse.sshfs") == false);
  assert(fs_type_is_eligible("fuse.rclone") == false);
  assert(fs_type_is_eligible("fuse.gvfsd-fuse") == false);

  /* eligible: real on-disk filesystems and nfs */
  assert(fs_type_is_eligible("ext4") == true);
  assert(fs_type_is_eligible("btrfs") == true);
  assert(fs_type_is_eligible("xfs") == true);
  assert(fs_type_is_eligible("bcachefs") == true);
  assert(fs_type_is_eligible("vfat") == true);
  assert(fs_type_is_eligible("ntfs") == true);
  assert(fs_type_is_eligible("nfs") == true);
  assert(fs_type_is_eligible("nfs4") == true);
}


static void
test_fs_type_is_pseudo(void)
{
  assert(fs_type_is_pseudo(NULL) == false);

  /* kernel-virtual: never probed for an existing trash */
  assert(fs_type_is_pseudo("bpf") == true);
  assert(fs_type_is_pseudo("sysfs") == true);
  assert(fs_type_is_pseudo("proc") == true);
  assert(fs_type_is_pseudo("cgroup2") == true);
  assert(fs_type_is_pseudo("efivarfs") == true);

  /* excluded from new trashes but file-capable, so still probed (not pseudo) */
  assert(fs_type_is_pseudo("tmpfs") == false);
  assert(fs_type_is_pseudo("overlay") == false);
  assert(fs_type_is_pseudo("squashfs") == false);
  assert(fs_type_is_pseudo("cifs") == false);

  /* real on-disk filesystems are not pseudo */
  assert(fs_type_is_pseudo("ext4") == false);
  assert(fs_type_is_pseudo("btrfs") == false);
  assert(fs_type_is_pseudo("nfs") == false);
}


static void
test_build_mount_trash_list(const char *tmpdir, const char *uid)
{
  char home_trash[PATH_MAX];
  sn_check(snprintf(home_trash, sizeof home_trash, "%s/Trash", tmpdir),
           sizeof home_trash);

  struct stat st;
  assert(lstat(tmpdir, &st) == 0);
  dev_t home_dev = st.st_dev;

  st_mount_trash *head = build_mount_trash_list(uid, home_trash, home_dev);
  assert(head != NULL);

  /* Head is the home-trash node. */
  assert(head->is_home_trash == true);
  assert(head->mount_path == NULL);
  assert(strcmp(head->trash_dir, home_trash) == 0);
  assert(head->dev_num == home_dev);

  char expected_files[PATH_MAX];
  char expected_info[PATH_MAX];
  sn_check(snprintf(expected_files, sizeof expected_files, "%s/files",
                    home_trash), sizeof expected_files);
  sn_check(snprintf(expected_info, sizeof expected_info, "%s/info",
                    home_trash), sizeof expected_info);
  assert(strcmp(head->files_dir, expected_files) == 0);
  assert(strcmp(head->info_dir, expected_info) == 0);

  /* No subsequent node should duplicate the home volume, and none should
   * be flagged as the home trash. */
  for (st_mount_trash *n = head->next; n != NULL; n = n->next)
  {
    assert(n->is_home_trash == false);
    assert(n->mount_path != NULL);
    assert(n->trash_dir != NULL);
    assert(n->files_dir != NULL);
    assert(n->info_dir != NULL);
    assert(n->dev_num != home_dev);
  }

  free_mount_trash_list(head);
}


#ifdef ANDROID_STORAGE
static void
test_is_android_emulated(void)
{
  assert(is_android_emulated("/storage/emulated", "fuse") == true);

  /* the per-user directory is not the mount, and only bare "fuse" is the
     Android shared-storage mount; "fuse.*" types are excluded elsewhere */
  assert(is_android_emulated("/storage/emulated/0", "fuse") == false);
  assert(is_android_emulated("/storage", "fuse") == false);
  assert(is_android_emulated("/storage/emulated", "fuse.sshfs") == false);
  assert(is_android_emulated("/storage/emulated", "ext4") == false);
  assert(is_android_emulated(NULL, "fuse") == false);
  assert(is_android_emulated("/storage/emulated", NULL) == false);
}


static void
test_android_user_dir_for(void)
{
  const char mount[] = "/storage/emulated";
  char *result;

  result = android_user_dir_for(mount, "/storage/emulated/0/Download/a.txt");
  assert(result != NULL);
  assert(strcmp(result, "/storage/emulated/0") == 0);
  free(result);

  /* the per-user directory itself */
  result = android_user_dir_for(mount, "/storage/emulated/0");
  assert(result != NULL);
  assert(strcmp(result, "/storage/emulated/0") == 0);
  free(result);

  /* a secondary profile is read from the path, never assumed to be 0 */
  result = android_user_dir_for(mount, "/storage/emulated/10/x");
  assert(result != NULL);
  assert(strcmp(result, "/storage/emulated/10") == 0);
  free(result);

  /* the mount itself has no per-user component */
  assert(android_user_dir_for(mount, "/storage/emulated") == NULL);
}


static void
test_android_primary_topdir(const char *tmpdir)
{
  char mount[PATH_MAX], user_dir[PATH_MAX], link[PATH_MAX];
  sn_check(snprintf(mount, sizeof mount, "%s/emulated", tmpdir),
           sizeof mount);
  sn_check(snprintf(user_dir, sizeof user_dir, "%s/0", mount),
           sizeof user_dir);
  sn_check(snprintf(link, sizeof link, "%s/primary", tmpdir), sizeof link);
  assert(rmw_mkdir(mount) == 0);
  assert(rmw_mkdir(user_dir) == 0);
  unlink(link);
  assert(symlink(user_dir, link) == 0);

  dev_t dev = 0;
  char *result = android_primary_topdir(link, mount, &dev);
  assert(result != NULL);
  assert(strcmp(result, user_dir) == 0);
  assert(dev != 0);
  free(result);

  /* a link pointing outside the mount is rejected */
  char outside[PATH_MAX], link_outside[PATH_MAX];
  sn_check(snprintf(outside, sizeof outside, "%s/elsewhere", tmpdir),
           sizeof outside);
  sn_check(snprintf(link_outside, sizeof link_outside, "%s/primary-outside",
                    tmpdir), sizeof link_outside);
  assert(rmw_mkdir(outside) == 0);
  unlink(link_outside);
  assert(symlink(outside, link_outside) == 0);
  assert(android_primary_topdir(link_outside, mount, &dev) == NULL);

  /* a dangling link is rejected */
  char dangling[PATH_MAX];
  sn_check(snprintf(dangling, sizeof dangling, "%s/primary-dangling", tmpdir),
           sizeof dangling);
  unlink(dangling);
  assert(symlink("/nonexistent-rmw-test-target", dangling) == 0);
  assert(android_primary_topdir(dangling, mount, &dev) == NULL);

  /* a file where a directory is expected is rejected */
  char regular[PATH_MAX], link_regular[PATH_MAX];
  sn_check(snprintf(regular, sizeof regular, "%s/notadir", mount),
           sizeof regular);
  sn_check(snprintf(link_regular, sizeof link_regular, "%s/primary-regular",
                    tmpdir), sizeof link_regular);
  int fd = open(regular, O_WRONLY | O_CREAT, 0644);
  assert(fd >= 0);
  close(fd);
  unlink(link_regular);
  assert(symlink(regular, link_regular) == 0);
  assert(android_primary_topdir(link_regular, mount, &dev) == NULL);
}
#endif


int
main(void)
{
  char tmpdir[PATH_MAX];
  sn_check(snprintf(tmpdir, sizeof tmpdir, "%s/test_topdir_trash",
                    RMW_FAKE_HOME), sizeof tmpdir);
  assert(rmw_mkdir(tmpdir) == 0);

  test_trash_path_for_topdir(tmpdir, "1000");
  test_fs_type_is_eligible();
  test_fs_type_is_pseudo();
  test_build_mount_trash_list(tmpdir, "1000");
#ifdef ANDROID_STORAGE
  test_is_android_emulated();
  test_android_user_dir_for();
  test_android_primary_topdir(tmpdir);
#endif

  assert(bsdutils_rm(tmpdir, false) == 0);
  return 0;
}
