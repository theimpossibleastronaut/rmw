
void get_config (const char *alt_config);

bool pre_rmw_check (const char *cmdargv);

int remove_to_waste (void);

int mkinfo (bool dup_filename);

void Restore (int argc, char *argv[], int optind);

bool purgeD (void);

int purge (void);

void undo_last_rmw (void);

int getch (void);

/* reads from keypress, echoes */
int getche (void);

/* Before copying or catting strings, make sure str1 won't exceed
 * PATH_MAX + 1
 * */
bool buf_check_with_strop (char *s1, const char *s2, bool mode);

bool buf_check (const char *str, unsigned short boundary);

int rmdir_recursive (char *path, bool isTop);

void mkdirErr (const char *dirname, const char *text_string);

void waste_check (const char *p);

bool isProtected (void);

void restore_select (void);

bool file_exist (const char *filename);

void get_time_string (char *tm_str, unsigned short len, const char *format);
