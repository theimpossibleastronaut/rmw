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
    assert(chmod(dot_trash, 01755) == 0); /* mkdir strips sticky bit on macOS */
    char *result = trash_path_for_topdir(tmpdir, uid);
    sn_check(snprintf(expected, sizeof expected, "%s/.Trash/%s", tmpdir, uid),
             sizeof expected);
    assert(result != NULL);
    assert(strcmp(result, expected) == 0);
    free(result);
    assert(rmdir(dot_trash) == 0);
  }
}


int
main(void)
{
  char tmpdir[PATH_MAX];
  sn_check(snprintf(tmpdir, sizeof tmpdir, "%s/test_topdir_trash",
                    RMW_FAKE_HOME), sizeof tmpdir);
  assert(rmw_mkdir(tmpdir) == 0);

  test_trash_path_for_topdir(tmpdir, "1000");

  assert(bsdutils_rm(tmpdir, false) == 0);
  return 0;
}
