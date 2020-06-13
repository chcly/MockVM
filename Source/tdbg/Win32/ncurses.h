// Stub file for intellisense

#define KEY_DOWN 13
#define KEY_ENTER 13

typedef void*  WINDOW;
extern WINDOW* stdscr;

#define COLOR_PAIR(c) c
#define KEY_F(c) c

WINDOW* initscr(void);
int     endwin(void);
int     delch(void);
void    move(int x, int y);
int     attron(int attrs);
int     attroff(int attrs);
int     insch(char ch);
int     curs_set(int visibility);
int     refresh(void);
int     has_colors(void);
int     start_color(void);
int     noecho(void);
int     keypad(WINDOW* win, bool bf);
void    use_default_colors(void);
void    getmaxyx(WINDOW*, int r, int c);
void    init_pair(int, int, int);
