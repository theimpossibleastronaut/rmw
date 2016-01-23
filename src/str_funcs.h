
/* prototypes for str_funcs.c */
// string handling functions for rmw

#ifndef str_funcs_h
#define str_funcs_h

int trim (char s[]);

void erase_char (char c, char *str);

bool change_HOME (char *t);

int trim_slash (char s[]);

void truncate_str (char *str, short len);

#endif
