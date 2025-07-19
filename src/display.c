/*
 * display.c
 * 表示系ラッパー関数を定義する
 */

#include <string.h>
#include <curses.h>
#include <iconv.h>
#include <stdlib.h>
#include <errno.h>

#include "rogue.h"
#include "display.h"
#include "machdep.h"
#include "message.h"

/*
 * Ncurses Colors
 */
enum rogue_colors
{
    WHITE = 1,
    RED = 2,
    GREEN = 3,
    YELLOW = 4,
    BLUE = 5,
    MAGENTA = 6,
    CYAN = 7,
    WHITE_REVERSE = 8,
    RED_REVERSE = 9,
    GREEN_REVERSE = 10,
    YELLOW_REVERSE = 11,
    BLUE_REVERSE = 12,
    MAGENTA_REVERSE = 13,
    CYAN_REVERSE = 14
};
#define RWHITE	 8
#define RRED	 9
#define RGREEN	 10
#define RYELLOW	 11
#define RBLUE	 12
#define RMAGENTA 13
#define RCYAN	 14
static int ch_attr[256];
char *color_str = "cbmyg";
extern boolean use_color;

/*
 * init_color_attr
 * カラー属性配列の初期化
 */
void
init_color_attr(void)
{
    char *chx, chy, *chz;
    int i, j, k;
    char color_type[] = "wrgybmcWRGYBMC";
    int colormap_list[5];

#if defined( COLOR )
    static boolean first_init = 1;

    /* 最初の一回のみ実行する命令 */
    if (first_init) {
	start_color();

	/* 背景色の定義 */
	assume_default_colors(COLOR_WHITE, COLOR_BLACK);

	/* 表示色の定義 */
	init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(RED, COLOR_RED, COLOR_BLACK);
	init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(WHITE_REVERSE, COLOR_BLACK, COLOR_WHITE);
	init_pair(RED_REVERSE, COLOR_BLACK, COLOR_RED);
	init_pair(GREEN_REVERSE, COLOR_BLACK, COLOR_GREEN);
	init_pair(YELLOW_REVERSE, COLOR_BLACK, COLOR_YELLOW);
	init_pair(BLUE_REVERSE, COLOR_BLACK, COLOR_BLUE);
	init_pair(MAGENTA_REVERSE, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(CYAN_REVERSE, COLOR_BLACK, COLOR_CYAN);
    }
#endif /* COLOR */

    /* 表示設定解析 */
    for (i = 0; i < 5 && color_str[i]; i++) {
	j = r_index(color_type, color_str[i], 0);
	if (j >= 0) {
	    switch (color_type[j]) {
	    default:
		colormap_list[i] = 0;
		break;
	    case 'w':
		colormap_list[i] = WHITE;
		break;
	    case 'r':
		colormap_list[i] = RED;
		break;
	    case 'g':
		colormap_list[i] = GREEN;
		break;
	    case 'y':
		colormap_list[i] = YELLOW;
		break;
	    case 'b':
		colormap_list[i] = BLUE;
		break;
	    case 'm':
		colormap_list[i] = MAGENTA;
		break;
	    case 'c':
		colormap_list[i] = CYAN;
		break;
	    case 'W':
		colormap_list[i] = WHITE_REVERSE;
		break;
	    case 'R':
		colormap_list[i] = RED_REVERSE;
		break;
	    case 'G':
		colormap_list[i] = GREEN_REVERSE;
		break;
	    case 'Y':
		colormap_list[i] = YELLOW_REVERSE;
		break;
	    case 'B':
		colormap_list[i] = BLUE_REVERSE;
		break;
	    case 'M':
		colormap_list[i] = MAGENTA_REVERSE;
		break;
	    case 'C':
		colormap_list[i] = CYAN_REVERSE;
		break;
	    }
	}
    }

    /* 文字のカラーマップの作成 */
    for (chx = "-|#+"; *chx; chx++) {
	get_colorpair_number((signed) *chx, colormap_list[0]);
    }
    get_colorpair_number((signed) '.', colormap_list[1]);
    for (chy = 'A'; chy <= 'Z'; chy++) {
	get_colorpair_number((signed) chy, colormap_list[2]);
    }
    for (chz = "%!?/=)]^*:,"; *chz; chz++) {
	get_colorpair_number((signed) *chz, colormap_list[3]);
    }
    get_colorpair_number(rogue.fchar, colormap_list[4]);

    if (!use_color) {
	for(k=0; k<128; k++) {
	    get_colorpair_number(k,0);
	}
    }

}

/*
 * put_color_pair
 * 指定の文字のカラーペア番号を出力する
 */
int
put_colorpair_number(char ch)
{
    return ch_attr[(signed) ch];
}

/*
 * get_color_pair
 * 指定の文字列のカラーペア番号を入力する
 */
void
get_colorpair_number(char ch, int num)
{
    ch_attr[(signed) ch] = num;
}

/*
 * addch_rogue
 * addch のラッパー関数
 * 文字毎のカラー表示も一括して請け負う
 */
int
addch_rogue(const chtype ch)
{
#if defined( COLOR )
    attrset(COLOR_PAIR(put_colorpair_number(ch)));
#endif /* COLOR */
    return addch(ch);
}

/*
 * mvaddch_rogue
 * mvaddch のラッパー関数
 * 文字毎のカラー表示も一括して請け負う
 */
int
mvaddch_rogue(int y, int x, const chtype ch)
{
#if defined( COLOR )
    attrset(COLOR_PAIR(put_colorpair_number(ch)));
#endif /* COLOR */
    return mvaddch(y, x, ch);
}

/*
 * utf8len
 * UTF8文字の先頭の1バイト ch から、UTF8文字のバイト数を返す
 *   0x00 - 0x7f: 1byte
 *   0x80 - 0xbf: - (マルチバイト文字の2バイト目以降、便宜上1bytes)
 *   0xc0 - 0xdf: 2bytes
 *   0xe0 - 0xef: 3bytes
 *   0xf0 - 0xf7: 4bytes
 *   0xf8 - 0xfb: 5bytes
 *   0xfc - 0xfd: 6bytes
 *   0xfe - 0xff: - (便宜上6bytes)
 */
int
utf8len(unsigned char ch)
{
    static unsigned char utf8len_table[] = { 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xff };
    int len = 1;

    while (len <= 6) {
	if (ch <= utf8len_table[len-1]) break;
	len++;
    }
    return len;
}

/*
 * convert_encoding
 * 文字エンコーディングの変換を行う
 */
int
convert_encoding(iconv_t conv, const char *src, char *dest, size_t dest_size)
{
    size_t inbytesleft;
    size_t outbytesleft = dest_size - 1;

    char *inbuf = (char *)src;
    char *outbuf = dest;
    char *outbuf_start = dest;

    size_t result;

    if (!src || !dest || dest_size == 0) {
        return -1;
    }

    inbytesleft = strlen(src);

    result = iconv(conv, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    if (result == (size_t)-1) {
        /* perror("iconv"); */
        /* iconv_close(conv); */
        return -1;
    }

    *outbuf = '\0';

    /* iconv_close(conv); */
    return 0;
}

/*
 * convert_eucjp_to_utf8
 * convert_encoding のラッパー関数
 * EUC および TERM_UTF8 が定義されている時のみ EUC を UTF-8 に変換する
 * それ以外の時は無変換で出力する。
 */
int
convert_eucjp_to_utf8(const char *src, char *dest, size_t dest_size)
{
#if defined( EUC ) && defined( TERM_UTF8 )
    static iconv_t conv_e2u = NULL;

    if (!conv_e2u) {
        conv_e2u = iconv_open("UTF-8//TRANSLIT", "EUC-JP");
        if (conv_e2u == (iconv_t)-1) {
            /* perror("iconv_open"); */
            return -1;
        }
    }
    return convert_encoding(conv_e2u, src, dest, dest_size);
#else
    memcpy(dest, src, dest_size);
    return 0;
#endif
}

/*
 * convert_utf8_to_eucjp
 * EUC および TERM_UTF8 が定義されている時のみ UTF-8 を EUC に変換する
 * それ以外の時は無変換で出力する。
 */
int
convert_utf8_to_eucjp(const char *src, char *dest, size_t dest_size)
{
#if defined( EUC ) && defined( TERM_UTF8 )
    static iconv_t conv_u2e = NULL;

    if (!conv_u2e) {
        conv_u2e = iconv_open("EUC-JP//TRANSLIT", "UTF-8");
        if (conv_u2e == (iconv_t)-1) {
            /* perror("iconv_open"); */
            return -1;
        }
    }
    return convert_encoding(conv_u2e, src, dest, dest_size);
#else
    memcpy(dest, src, dest_size);
    return 0;
#endif
}

/*
 * addstr_rogue
 * addstr のラッパー関数
 * 文字列のカラー表示も一括して請け負う
 */
int
addstr_rogue(const char *str)
{
#if defined( COLOR )
    attrset(COLOR_PAIR(0));
#endif /* COLOR */
    char msg[ROGUE_COLUMNS];
    convert_eucjp_to_utf8(str, msg, ROGUE_COLUMNS);
    return addstr(msg);
}

/*
 * mvaddstr_rogue
 * mvaddstr のラッパー関数
 * 文字列のカラー表示も一括して請け負う
 */
int
mvaddstr_rogue(int y, int x, const char *str)
{
#if defined( COLOR )
    attr_t attr_stats;
    int color_stats;

    attr_get(&attr_stats, &color_stats, NULL);
    if (attr_stats & A_REVERSE) {
	attrset(COLOR_PAIR(CYAN));
	attron(A_REVERSE);
    } else if (strcmp(str, mesg[187]) == 0) {
	attrset(COLOR_PAIR(YELLOW));
    } else if (strcmp(str, mesg[188]) == 0) {
	attrset(COLOR_PAIR(GREEN));
    } else {
	attrset(COLOR_PAIR(0));
    }
#endif /* COLOR */

    char msg[ROGUE_COLUMNS];
    convert_eucjp_to_utf8(str, msg, ROGUE_COLUMNS);
    return mvaddstr(y, x, msg);
}

/*
 * mvinch_rogue
 * mvinch のラッパー関数
 * 属性を除去して文字情報だけを抽出する
 */
chtype
mvinch_rogue(int y, int x)
{
#if defined( COLOR )
    return mvinch(y, x) & A_CHARTEXT;
#else /* not COLOR */
    return mvinch(y, x);
#endif /* not COLOR */
}
