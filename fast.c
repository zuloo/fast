#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <fast.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <ncursesw/curses.h>
#include <locale.h>

/*
static const char MARKTOP[] = ".";
static const char MARKBOT[] = "'";
*/
static const char MARKTOP[] = "\u2304";
static const char MARKBOT[] = "\u2303";
static const char INFO[] = "fast v1.0";
static const char USAGE[] = "[SPACE] play/pause  [g] |<   [h] <   [j] --   [k] ++   [l] >   [q] quit";
static const char DELIM[] = "\n\t- ";

int main(void)
{
    setlocale(LC_ALL, "en_US.UTF8");
    int x,y,height,width;
    char *data = inputString(stdin, 1024);

    freopen("/dev/tty", "rw", stdin);

    // enabling ncurses mode
    WINDOW *win = initscr();
    keypad(win, TRUE);
    noecho();
    cbreak();
    curs_set(0);

    // create colorpair for pivot character
    start_color();
    use_default_colors();
    init_pair(1,COLOR_RED,-1);

    // get terminal size
    getmaxyx(win,height,width);
    y=height/2;
    x=width/2;

    // print info
    mvaddstr(0,0,INFO);
    mvaddstr(height-1,0,USAGE);

    // print pivot markers
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

// print the "words per minute" value
void printSpeed(int speed)
{
    mvprintw(0,15,"words / minute: %i",speed);
    clrtoeol();
}

// read all the stuff from the pipe
char *inputString(FILE* fp, size_t size)
{
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
char *backwards(char *data, char *index,const char *accept)
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

// get the number of bytes in utf8-char↲
int byteInChar(unsigned char v)
{
    if(v<128)
        return 1;
    else if(v<224)
        return 2;
    else if(v<240)
        return 3;
    else 
        return 4;
}

// fast read looper
void fastread(char *data, int x, int y, int speed)
{
    bool run = false;
    bool pause = false;
    bool last = false;
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
        next = strpbrk(next,DELIM);

        // did we hit the end of the string?
        if(!next || (int)(next-data)>=maxlen){
            run = false;
            last = true;
        }

        // length of the chararray (not neccessarely charachter count in utf8)
        length = (int)(next-curr);

        // calulate character count
        int charlength = 0;
        for(int i=0; i<length; i+=byteInChar((unsigned char)*(curr+i)))
            ++charlength;

        // which letter to highlight
        pivot = pivotLetter(charlength);
        int pos = pivot;
        // print characters before pivot letter, if any
        if(pivot > 0)
        {
            char pre[pivot*4+1];
            int offset=0;
            for(int i=0; i<pivot; i++)
            {
                pre[i+offset]=*(curr+i+offset);
                // if wide character copy the rest and alter offset
                int charsize = byteInChar((unsigned char)pre[i+offset]);
                for(int j=1; j<charsize; j++)
                {
                    ++offset;
                    pre[i+offset]=*(curr+i+offset);
                }
            }
            // apply offset
            pivot+=offset;

            pre[pivot] = '\0';
            mvprintw(y,x-pos,"%s",pre);
        }

        // print pivot letter
        char mid[5];
        int offset=0;
        mid[0]=*(curr+pivot);
        // if wide character copy the rest and alter offset
        int charsize = byteInChar((unsigned char)mid[0]);
        for(int i=1; i<charsize; i++)
        {
            ++offset;
            mid[i]=*(curr+pivot+offset);
        }
        // apply offset
        pivot+=offset;

        mid[1+offset] = '\0';
        attron(COLOR_PAIR(1));
        mvprintw(y,x,"%s",mid);
        attroff(COLOR_PAIR(1));

        // print characters after pivot letter, if any
        if(length>pivot+1)
        {
            char end[length-pivot];
            strncpy(end, curr+pivot+1, length-pivot-1);
            end[length-pivot-1] = '\0';
            mvprintw(y,x+1,"%s",end);
        }
        if(*next == '-')
            mvprintw(y,x+length-pivot,"-");
    
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
                    next = backwards(data, next-2, DELIM);
                    do
                        next = backwards(data, --next, DELIM);
                    while(strchr(DELIM,*(--next)) && (int)(next-data)>0);
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
        while(next && (int)(next-data)<maxlen && strchr(DELIM,*(++next)))
            next = strpbrk(next,DELIM);
        // if we hit the end go back until last word begins
        if((int)(next-data)>=maxlen)
        {
            --next;
            do
                next = backwards(data, next, DELIM);
            while(strchr(DELIM,*(--next)) && (int)(next-data)>0);
            next = backwards(data, next, DELIM);
            ++next;
        }
        curr = next;
    }
}

