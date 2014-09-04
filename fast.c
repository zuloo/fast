#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <fast.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <curses.h>
#include <locale.h>

static const char MARKTOP[] = ".";
static const char MARKBOT[] = "'";
/*
static const char MARKTOP[] = "\u2304";
static const char MARKBOT[] = "\u2303";
*/
static const char INFO[] = "fast v1.0";
static const char USAGE[] = "[SPACE] play/pause  [g] |<   [h] <   [j] --   [k] ++   [l] >   [q] quit";

int main(void)
{
    int x,y,height,width;
    char *data = inputString(stdin, 1024);

    freopen("/dev/tty", "rw", stdin);

    // enabling ncurses mode
    setlocale(LC_ALL, "");
    WINDOW *win = initscr();
    keypad(win, TRUE);
    noecho();
    cbreak();
    curs_set(0);

    start_color();
    use_default_colors();
    short fg,bg;
    pair_content(0,&fg,&bg);
    init_pair(1,COLOR_RED,-1);


    getmaxyx(win,height,width);
    y=height/2;
    x=width/2;

    mvaddstr(0,0,INFO);
    mvaddstr(height-1,0,USAGE);

    attron(COLOR_PAIR(1));
    mvaddstr(y-1,x,MARKTOP);
    mvaddstr(y+1,x,MARKBOT);
    attroff(COLOR_PAIR(1));

    refresh();

    int speed = 350;

    printSpeed(speed);

    fastread(data,x,y,speed);

    free(data);
    endwin();
    return 0;
}

void printSpeed(int speed)
{
    mvprintw(0,15,"words / minute: %i",speed);
    clrtoeol();
}

// read all the stuff from the pipe
char *inputString(FILE* fp, size_t size){
    //The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp))){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=1024));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

// which letter to paint red
int pivotLetter(int l)
{
    if(l == 1)
        return 0;
    else if(l >=2 && l<= 5)
        return 1;
    else if(l >= 6 && l <= 9)
        return 2;
    else if(l >= 10 && l <= 13)
        return 3;
    else 
        return 4;
}

// strpbrk backwards, data marks minimal index
char *backwards(char *data, char *index, char *accept)
{
    if((int)(index-data)<0)
        return (char *) data;

    while((int)(index-data)>0)
    {
        const char *a = accept;
        while (*a != '\0')
            if(*a++ == *index)
                return index;
        index--;
    }
    return index;
}

// fast read looper
void fastread(char *data, int x, int y, int speed)
{
    bool run = false;
    bool pause = false;
    bool last = false;
    char mid[2];
    int ch;
    char *curr = data;
    int length = 0;
    int pivot = 0;
    int maxlen = strlen(data)-1;
    while(true)
    {
        // reset line
        mvprintw(y,0," ");
        clrtoeol();
        
        char *next = curr;
        next = strpbrk(next,"\n\t ");

        // did we hit the end of the string?
        if(!next || (int)(next-data)>=maxlen){
            run = false;
            last = true;
        }

        length = (int)(next-curr);
        pivot = pivotLetter(length);
        
        // print characters before pivot letter, if any
        if(pivot > 0)
        {
            char pre[pivot+1];
            strncpy(pre, curr, pivot);
            pre[pivot] = '\0';
            mvprintw(y,x-pivot,pre);
        }

        // print pivot letter
        strncpy(mid, curr+pivot, 1);
        mid[1] = '\0';
        attron(COLOR_PAIR(1));
        mvprintw(y,x,mid);
        attroff(COLOR_PAIR(1));
        
        // print characters after pivot letter, if any
        if(length>pivot+1)
        {
            char end[length-pivot];
            strncpy(end, curr+pivot+1, length-pivot-1);
            end[length-pivot-1] = '\0';
            mvprintw(y,x+1,end);
        }
    
        refresh();

        if(pause)
        {
            // pause was set -> stop run, reset pause flag
            pause = false;
            run = false;
        }

        do
        {
            // nonblocking keyboard input if running, blocking otherwise
            if(run && !last)
                timeout(0);
            ch = getch();

            switch(ch)
            {
                case ' ':
                    // toggle pause
                    if(run)
                        run = false;
                    else if(!last)
                        run = true;
                    break;
                case 'q':
                    // quit
                    return;
                    break;
                case 'k':
                    // less speed
                    speed+=50;
                    printSpeed(speed);
                    break;
                case 'j':
                    // more speed
                    speed-=50;
                    if(speed < 50)
                        speed = 50;
                    printSpeed(speed);
                    break;
                case 'h':
                    // previous word, pause
                    run = true;
                    pause = true;
                    last = false;
                    next = backwards(data, next-2, "\n\t ");
                    do
                        next = backwards(data, --next, "\n\t ");
                    while(strchr("\n\t ",*(--next)) && (int)(next-data)>0);
                    break;
                case 'l':
                    // next word, pause
                    run = true;
                    pause = true;
                    break;
                case 'g':
                    // back to start
                    next = data;
                    --next;
                    last = false;
                    pause = true;
                    run = true;
                    break;
                default:
                    break;
            }
            if(run)
                usleep(1000*1000*60/speed);
        }
        while(last || !run);

        // skip multiple whitespaces until a word begins
        while(next && (int)(next-data)<maxlen && strchr("\t\n ",*(++next)))
            next = strpbrk(next,"\n\t ");
        // if we hit the end go back until last word begins
        if((int)(next-data)>=maxlen)
		{
			--next;
            do
                next = backwards(data, next, "\n\t ");
            while(strchr("\n\t ",*(--next)) && (int)(next-data)>0);
            next = backwards(data, next, "\n\t ");
            ++next;
		}
        curr = next;
    }
}

