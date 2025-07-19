#if !defined( __DISPLAY_H__ )
#define __DISPLAY_H__

extern void init_color_attr(void);
extern int put_colorpair_number(char ch);
extern void get_colorpair_number(char ch, int num);
extern int utf8len(unsigned char ch);
extern int convert_eucjp_to_utf8(const char *src, char *dest, size_t dest_size);
extern int convert_utf8_to_eucjp(const char *src, char *dest, size_t dest_size);
extern int addch_rogue(const chtype ch);
extern int mvaddch_rogue(int y, int x, const chtype ch);
extern int addstr_rogue(const char *str);
extern int mvaddstr_rogue(int y, int x, const char *str);
extern chtype mvinch_rogue(int y, int x);

extern char *color_str;

#endif /* not __DISPLAY_H__ */
