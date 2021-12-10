/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
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

#include "config_rmw.h"
#include "utils_rmw.h"
#include "strings_rmw.h"
#include "canfigger.h"

static const int DEFAULT_EXPIRE_AGE = 0;
static const char *lit_files = "files";

const char *expire_age_str = "expire_age";

/*!
 * Prints a copy of the default config file to the specified stream.
 *
 * If any changes are made to rmwrc.example, the text here will need to be updated
 * to match
 */
static void
print_config (FILE * stream)
{
  fputs (_("\
# rmw default waste directory, separate from the desktop trash\n"), stream);
  fputs ("\
WASTE = $HOME/.local/share/Waste\n", stream);
  fputs (_("\n\
# The directory used by the FreeDesktop.org Trash spec\n\
# Note to macOS and Windows users: moving files to 'Desktop' trash\n\
# doesn't work yet\n"), stream);
  fputs ("\
# WASTE=$HOME/.local/share/Trash\n", stream);
  fputs ("\n", stream);
  fputs (_("\
# A folder can use the $UID variable.\n"), stream);
  fputs (_("\
# See the README or man page for details about using the 'removable' attribute\n"), stream);
  fputs ("\
# WASTE=/mnt/flash/.Trash-$UID, removable\n", stream);
  fputs (_("\n\
# How many days should items be allowed to stay in the waste\n\
# directories before they are permanently deleted\n\
#\n\
# use '0' to disable purging (can be overridden by using --purge=N_DAYS)\n\
#\n"), stream);
  fprintf (stream, "\
%s = %d\n", expire_age_str, DEFAULT_EXPIRE_AGE);
  fputs (_("\n\
# purge is allowed to run without the '-f' option. If you'd rather\n\
# require the use of '-f', you may uncomment the line below.\n\
#\n"), stream);
  fputs ("\
# force_required\n", stream);
}


/*
 * replace part of a string, adapted from code by Gazl
 * https://www.linuxquestions.org/questions/showthread.php?&p=5794938#post5794938
*/
static char *
strrepl (char *src, const char *str, char *repl)
{
  // The replacement text may make the returned string shorter or
  // longer than src, so just add the length of all three for the
  // mallocation.
  int req_len = multi_strlen (src, str, repl, NULL) + 1;
  char *dest = malloc (req_len);
  chk_malloc (dest, __func__, __LINE__);

  char *s, *d, *p;

  s = strstr (src, str);
  if (s && *str != '\0')
  {
    d = dest;
    for (p = src; p < s; p++, d++)
      *d = *p;
    for (p = repl; *p != '\0'; p++, d++)
      *d = *p;
    for (p = s + strlen (str); *p != '\0'; p++, d++)
      *d = *p;
    *d = '\0';
  }
  else
    strcpy (dest, src);

  dest = realloc (dest, strlen (dest) + 1);
  chk_malloc (dest, __func__, __LINE__);
  return dest;
}

/*!
 * If "$HOME", "~", or "$UID" is used in the configuration file, convert it
 * to the literal value.
 */
void
realize_waste_line (char *str)
{
  trim_char ('/', str);
  /*
   *
   * FIXME: This will need some work in order to be implemented
   * on Windows
   *
   */

  uid_t uid = geteuid ();
  struct passwd *pwd = getpwuid (uid);  /* don't free, see getpwnam() for details */

  if (pwd == NULL)
  {
    print_msg_error ();
    fputs ("Unable to get $UID\n", stderr);
    exit (EXIT_FAILURE);
  }

  /* What's a good length for this? */
  char UID[40];
  sn_check (snprintf (UID, sizeof UID, "%u", pwd->pw_uid), sizeof UID, __func__, __LINE__);

  struct st_vars_to_check st_var[] = {
    {"~", HOMEDIR},
    {"$HOME", HOMEDIR},
    {"$UID", UID},
    {NULL, NULL}
  };

  int i = 0;
  while (st_var[i].name != NULL)
  {
    if (strstr (str, st_var[i].name) != NULL)
    {
      char *dest = strrepl (str, st_var[i].name, (char *) st_var[i].value);
      bufchk_len (strlen (dest) + 1, LEN_MAX_PATH, __func__, __LINE__);
      strcpy (str, dest);
      free (dest);

      /* check the string again, in case str contains something like
       * $HOME/Trash-$UID (which would be rare, if ever, but... */
      i--;
    }

    i++;
  }

  return;
}


/*!
 * This function is called when the "WASTE" option is encountered in the
 * config file. The line is parsed and added to the linked list of WASTE
 * folders.
 */
static st_waste *
parse_line_waste (st_waste * waste_curr, st_canfigger_node * node,
                  const rmw_options * cli_user_options, bool fake_media_root)
{
  bool removable = 0;
  if (strcmp ("removable", node->attribute) == 0)
    removable = 1;
  else if (*node->attribute != '\0')
  {
    print_msg_warn ();
    printf ("ignoring invalid attribute: '%s'\n", node->attribute);
  }

  bufchk_len (strlen (node->value) + 1, LEN_MAX_PATH, __func__, __LINE__);
  char tmp_waste_parent_folder[LEN_MAX_PATH];
  strcpy (tmp_waste_parent_folder, node->value);
  realize_waste_line (tmp_waste_parent_folder);

  bool is_attached = exists (tmp_waste_parent_folder);
  if (removable && !is_attached)
  {
    if (cli_user_options->list)
      show_folder_line (tmp_waste_parent_folder, removable, is_attached);

    return NULL;
  }

  st_waste *temp_node = malloc (sizeof (st_waste));
  chk_malloc (temp_node, __func__, __LINE__);

  if (waste_curr != NULL)
  {
    waste_curr->next_node = temp_node;
    temp_node->prev_node = waste_curr;
    waste_curr = waste_curr->next_node;
  }
  else
  {
    waste_curr = temp_node;
    waste_curr->prev_node = NULL;
  }
  waste_curr->next_node = NULL;

  waste_curr->removable = removable ? true : false;

  /* make the parent... */
  waste_curr->parent = malloc (strlen (tmp_waste_parent_folder) + 1);
  chk_malloc (waste_curr->parent, __func__, __LINE__);
  strcpy (waste_curr->parent, tmp_waste_parent_folder);

  /* and the files... */
  waste_curr->files = join_paths (waste_curr->parent, lit_files, NULL);
  waste_curr->len_files = strlen (waste_curr->files);

  if (!exists (waste_curr->files))
  {
    if (!rmw_mkdir (waste_curr->files, S_IRWXU))
      msg_success_mkdir (waste_curr->files);
    else
    {
      msg_err_mkdir (waste_curr->files, __func__);
      exit (errno);
    }
  }

  waste_curr->info = join_paths (waste_curr->parent, lit_info, NULL);
  waste_curr->len_info = strlen (waste_curr->info);

  if (!exists (waste_curr->info))
  {
    if (!rmw_mkdir (waste_curr->info, S_IRWXU))
      msg_success_mkdir (waste_curr->info);
    else
    {
      msg_err_mkdir (waste_curr->info, __func__);
      exit (errno);
    }
  }

  // get device number to use later for rename
  struct stat st, mp_st;
  if (!lstat (waste_curr->parent, &st))
  {
    waste_curr->dev_num = st.st_dev;

    char tmp[LEN_MAX_PATH];
    strcpy (tmp, waste_curr->parent);
    char *media_root_ptr = rmw_dirname (tmp);
    waste_curr->media_root = malloc (strlen (media_root_ptr) + 1);
    chk_malloc (waste_curr->media_root, __func__, __LINE__);
    strcpy (waste_curr->media_root, media_root_ptr);
    strcpy (tmp, waste_curr->media_root);
    if (!lstat (rmw_dirname (tmp), &mp_st))
    {
      if (mp_st.st_dev == waste_curr->dev_num && !fake_media_root)
      {
        free (waste_curr->media_root);
        waste_curr->media_root = NULL;
      }
    }
    else
      msg_err_lstat (waste_curr->parent, __func__, __LINE__);
  }
  else
    msg_err_lstat (waste_curr->parent, __func__, __LINE__);

  return waste_curr;
}


const char *
get_config_home_dir (void)
{
  const char rel_default[] = "/.config";

  const char *xdg_config_home = getenv ("XDG_CONFIG_HOME");

  static const char *ptr;
  const char *enable_test = getenv (ENV_RMW_FAKE_HOME);

  if (enable_test != NULL || (xdg_config_home == NULL && enable_test == NULL))
  {
    char *_config_home = join_paths (HOMEDIR, rel_default, NULL);
    static char config_home[LEN_MAX_PATH];
    strcpy (config_home, _config_home);
    free (_config_home);
    ptr = &config_home[0];
    return ptr;
  }

  bufchk_len (strlen (xdg_config_home) + 1, LEN_MAX_PATH, __func__, __LINE__);
  return xdg_config_home;
}


/*!
 * Get configuration data (parse the config file)
 */
void
parse_config_file (const rmw_options * cli_user_options,
                   st_config * st_config_data)
{
  /* If no alternate configuration was specifed (-c) */
  if (cli_user_options->alt_config == NULL)
  {
    const char rel_default_config[] = "rmwrc";
    char *tmp_str = join_paths (st_config_data->dir, rel_default_config, NULL);
    strcpy (st_config_data->file, tmp_str);
    free (tmp_str);
  }
  else
  {
    bufchk_len (strlen (cli_user_options->alt_config) + 1, LEN_MAX_PATH,
                __func__, __LINE__);
    strcpy (st_config_data->file, cli_user_options->alt_config);
  }

  if (!exists (st_config_data->file))
  {
    FILE *fd = fopen (st_config_data->file, "w");
    if (fd)
    {
      printf (_("Creating default configuration file:"));
      printf ("\n  %s\n\n", st_config_data->file);

      print_config (fd);
      close_file (fd, st_config_data->file, __func__);
    }
    else
    {
      open_err (st_config_data->file, __func__);
      printf (_("Unable to read or write a configuration file.\n"));
      exit (errno);
    }
  }

  st_canfigger_list *cfg_node =
    canfigger_parse_file (st_config_data->file, ',');
  st_canfigger_list *root = cfg_node;

  if (cfg_node == NULL)
  {
    perror (__func__);
    exit (errno);
  }

  enum
  {
    EXPIRE_AGE,
    WASTE,
    FORCE_REQUIRED,
    PURGE_AFTER,
    INVALID_OPTION
  };

  struct opt
  {
    const char *opt;
    const unsigned short int e_opt;
  };

  struct opt st_opt[] = {
    {"WASTE", WASTE},
    {expire_age_str, EXPIRE_AGE},
    {"force_required", FORCE_REQUIRED},
    {"purge_after", PURGE_AFTER},
    {NULL, INVALID_OPTION}
  };

  st_waste *waste_curr = st_config_data->st_waste_folder_props_head;
  while (cfg_node != NULL)
  {
    int i = 0;
    while (st_opt[i].opt != NULL)
    {
      if (strcasecmp (st_opt[i].opt, cfg_node->key) == 0)
        break;
      i++;
    }

    switch (st_opt[i].e_opt)
    {
    case EXPIRE_AGE:
    case PURGE_AFTER:
      if (cli_user_options->want_purge <= 0)
      {
        st_config_data->expire_age = atoi (cfg_node->value);
        if (i == PURGE_AFTER)
        {
          puts ("");
          print_msg_warn ();
          printf ("\
  The configuration option 'purge_after' will be deprecated.\n\
  Please replace it with '%s' instead.\n\n", expire_age_str);
        }
      }
      else
      {
        st_config_data->expire_age = cli_user_options->want_purge;
      }
      break;
    case FORCE_REQUIRED:
      st_config_data->force_required = 1;
      break;
    case WASTE:
      {
        st_waste *st_new_waste_ptr =
          parse_line_waste (waste_curr, cfg_node, cli_user_options,
                            st_config_data->fake_media_root);
        if (st_new_waste_ptr != NULL)
        {
          waste_curr = st_new_waste_ptr;
          if (st_config_data->st_waste_folder_props_head == NULL)
            st_config_data->st_waste_folder_props_head = waste_curr;
        }
      }
      break;
    default:
      print_msg_warn ();
      printf (_("Unknown or invalid option: '%s'\n"), cfg_node->key);
    }
    cfg_node = cfg_node->next;
  }

  canfigger_free (root);

  if (waste_curr == NULL)
  {
    print_msg_error ();
    printf (_("no usable WASTE folder could be found\n\
Please check your configuration file and permissions\n\
If you need further help, or to report a possible bug,\n\
visit the rmw web site at\n"));
    printf ("  " PACKAGE_URL "\n");
    printf ("Unable to continue. Exiting...\n");
    msg_return_code (-1);
    exit (EXIT_FAILURE);
  }

  return;
}


void
init_config_data (st_config * x)
{
  x->dir = get_config_home_dir ();

  if (!exists (x->dir))
  {
    if (!rmw_mkdir (x->dir, S_IRWXU))
      msg_success_mkdir (x->dir);
    else
    {
      msg_err_mkdir (x->dir, __func__);
      exit (errno);
    }
  }

  x->st_waste_folder_props_head = NULL;
  /*
   * The default value is only used as a last resort,
   * if for some reason unspecified in the config file.
   */
  x->expire_age = DEFAULT_EXPIRE_AGE;
  x->force_required = 0;

  const char *f = getenv ("RMW_FAKE_MEDIA_ROOT");
  if (f != NULL)
    x->fake_media_root = (strcasecmp (f, "true") == 0) ? true : false;
  else
    x->fake_media_root = false;
  if (verbose)
    printf ("RMW_FAKE_MEDIA_ROOT:%s\n",
            x->fake_media_root == false ? "false" : "true");
}

void
show_folder_line (const char *folder, const bool is_r, const bool is_attached)
{
  printf ("%s", folder);
  if (is_r && verbose)
  {
    /*
     * These lines are separated to ease translation
     *
     */
    printf (" (");
    printf (_("removable, "));
    /* TRANSLATORS: context - "a mounted device or filesystem is presently attached or mounted" */
    printf ("%s)", is_attached == true ? _("attached") : _("detached"));
  }

  printf ("\n");
}
