#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h> // for random()
#include "screen.h"
#include "tty.h"
#include "snake.h"
// The characters to be shown and their colors.
char scr[80][24];
char color[80][24];

// Data variables for the game...
int px;
int py;
int dx;
int dy;
int prevdx;
int prevdy;
int money;
int a = -1;
int b = -1;
int c = -1;
int temp;
int health;
enum { SPLASH, RUNNING} phase = SPLASH;
int splash_ticks;
//int x1, y1, x2, y2;
//int o,p,q,r,s,t;//rgb
//char buf1[], buf2[], buf3[];

#define MAXLEN 20
struct {
  int8_t x;
  int8_t y;
} body[MAXLEN];
int bodylen;

// Print a message at screen coordinate x,y in color c.
void msg(int x, int y, int c, const char *s)
{
  int n;
  int len = strlen(s);
  for(n=0; n<len && n+x<80; n++) {
    color[x+n][y] = c;
    scr[x+n][y] = s[n];
  }
}

// Check two points, (x1,y1) and (x2,y2) and return a 1
// if they are within 5 character cells of each other.
int tooclose(int x1, int y1, int x2, int y2)
{
  int x = x1 - x2;
  int y = y1 - y2;
  if (x*x + y*y < 25)
    return 1;
  return 0;
}

// Put a dollar sign in a random location of the screen.
// Make sure that there is nothing there though.
// Also, make sure it is not too close to the snake's head.
// Let's not make the game too easy.
void newmoney(void)
{
  int x,y;
  do {
    x = (random() % 60) + 1;
    y = (random() % 22) + 1;
  } while(scr[x][y] != ' ' || tooclose(x,y, px,py));
  scr[x][y] = '$';
  color[x][y] = 0xe0;
}

// Eat some money.  Increment the money counter.
// Display the total money in the upper right of the screen.
void getmoney(void)
{
  money++;
  newmoney();
  health = 100;
}

//void newport(void)
//{
//  int x,y;
//  do {
//    x = (random() % 60) + 1;
//    y = (random() % 22) + 1;
//  } while(scr[x][y] != ' ' || tooclose(x,y, px,py));
//  scr[x][y] = '#';
//  color[x][y] = 0x10;
//  x2 = x;
//  y2 = y;
//}

// Indicate a collision with a big red X.
// Also display a "Game over" statement.
void collision(void)
{
    raw_mode();
  msg(px,py,0x90, "X");
  msg(px-1,py-1,0x90, "\\");
  msg(px+1,py-1,0x90, "/");
  msg(px+1,py+1,0x90, "\\");
  msg(px-1,py+1,0x90, "/");
  char buf[100];
  if (py > 12) {
    msg(30,8, 0x0f, "   G A M E   O V E R   ");
    sprintf(buf, "      Your Score:   %d     ", money);
    msg(30,9,0x0f,buf);
    msg(30,10, 0x0f, " Press any key to restart");
  } else {
    msg(30,15, 0x0f,"    G A M E   O V E R    ");
    sprintf(buf, "      Your Score:   %d    ", money);
    msg(30,16,0x0f,buf);
    msg(30,17, 0x0f," Press any key to restart");
  }

  if (money > c) {
      if (py > 12) {
        msg(30,8, 0x0f, "      G A M E   O V E R     ");
        msg(30,9,0x0f,"   You got the new record!  ");
        msg(30,10,0x0f,"  Please enter your name:   ");
        msg(30,11,0x08,"(｡ì _ í｡)                   ");
      } else {
        msg(30,15, 0x0f, "      G A M E   O V E R     ");
        msg(30,16,0x0f,"   You got the new record!  ");
        msg(30,17,0x0f,"  Please enter your name:   ");
        //msg(30,18,0x08,"                            ");
        msg(30,18,0x08,"(｡ì _ í｡)                   ");
      }
      render();
      getname();
  }
}

// Initialize the game data structures.
void init(void)
{
  int x,y;
  for(y=0; y<24; y++)
    for(x=0; x<64; x++) {
      scr[x][y] = ' ';
      color[x][y] = 0xf0;
    }
  for(y=11; y<24; y++)
    for(x=0; x<80; x++) {
      scr[x][y] = ' ';
      color[x][y] = 0xf0;
    }

  //status area
  for(y=1; y<6; y++)
    for(x=66; x<78; x++) {
      scr[x][y] = ' ';
      color[x][y] = 0x06;
    }

msg(68, 7, 0x90, " T");
msg(70, 7, 0xa0, " O");
msg(72, 7, 0xc0, " P ");

  for(x=0; x<62; x++) {
    if (x==0 || x==61) {
      msg(x,0,0xb0,"+");
      msg(x,23,0xb0,"+");
    } else {
      msg(x,0,0xb0,"-");
      msg(x,23,0xb0,"-");
    }
  }
  for(y=1; y<23; y++) {
    msg(0,y, 0xb0, "|");
    msg(61,y, 0xb0, "|");
  }
  px=40;
  py=12;
  scr[px][py] = '@';
  color[px][py] = 0xa0;
  bodylen = 6;
  body[0].x = px;
  body[0].y = py;
  for(x=1; x<6; x++) {
    scr[px-x][py] = '-';
    color[px-x][py] = 0xa0;
    body[x].x = px-x;
    body[x].y = py;
  }
  dx=1;
  dy=0;
  prevdx = dx;
  prevdy = dy;

  money = 0;
  health = 100;
  newmoney();
}

// Dump the scr and color arrays to the terminal screen.
void render(void)
{
  int x,y;
  home();
  int col = color[0][0];
  fgbg(col);
  for(y=0; y<24; y++) {
    setpos(0,y);
    for(x=0; x<80; x++) {
      if (color[x][y] != col) {
        col = color[x][y];
        fgbg(col);
      }
      putchar(scr[x][y]);
    }
  }
  fflush(stdout);
}

//void rgb(void)
//{
//  int x,y;
//  home();
//  int col = color[0][0];
//  fg(o,p,q);
//  bg(r,s,t);
//  for(y=0; y<24; y++) {
//    setpos(0,y);
//    for(x=0; x<80; x++) {
//      if (color[x][y] != col) {
//        col = color[x][y];
//        fg(o,p,q);
//        bg(r,s,t);
//      }
//      putchar(scr[x][y]);
//    }
//  }
//  fflush(stdout);
//}

// Display the initial splash screen.
void splash(void)
{
  clear();
  int x,y;
  for(y=0; y<24; y++)
    for(x=0; x<80; x++) {
      scr[x][y] = ' ';
      color[x][y] = 0x70;
    }
  msg(30,8, 0x0a, "                 ");
  msg(30,9, 0x0a, "  Hungry Snake   ");
  msg(30,10,0x0a, "  Press Any Key  ");
  msg(30,11,0x0a, "                 ");
  render();
//  o = 85; p = 148; q = 56;
//  r = 85; s = 148; t = 56;
//  rgb();

}

// Extend the snake into the new position.
void extend(void)
{
  scr[px][py] = '@'; // draw new head
  if (health<=50)
      color[px][py] = 0xe0;
  else
      color[px][py] = 0xa0;
  if (dx != 0) {
    if (prevdx != 0) // Did we turn a corner?
      scr[body[0].x][body[0].y] = '-'; // no
    else
      scr[body[0].x][body[0].y] = '+'; // yes
  } else {
    if (prevdy != 0) // Did we turn a corner?
      scr[body[0].x][body[0].y] = '|'; // no
    else
      scr[body[0].x][body[0].y] = '+'; // yes
  }
  int n;
  for(n=bodylen; n>=0; n--)
    body[n] = body[n-1];
  body[0].x = px;
  body[0].y = py;
  bodylen += 1;
}

// Move the snake into the new position.
// And erase the last character of the tail.
void move(void)
{
    scr[body[bodylen-1].x][body[bodylen-1].y] = ' '; // erase end of tail
    scr[px][py] = '@'; // draw new head
    if (health<=50)
        color[px][py] = 0xe0;
    else
        color[px][py] = 0xa0;
    if (dx != 0) {
      if (prevdx != 0) // Did we turn a corner?
        scr[body[0].x][body[0].y] = '-'; // no
      else
        scr[body[0].x][body[0].y] = '+'; // yes
    } else {
      if (prevdy != 0) // Did we turn a corner?
        scr[body[0].x][body[0].y] = '|'; // no
      else
        scr[body[0].x][body[0].y] = '+'; // yes
    }
    int n;
    for(n=bodylen-1; n>0; n--) {
      body[n] = body[n-1];
    }
    body[0].x = px;
    body[0].y = py;

    health--;
    char buf[100];
    sprintf(buf, "Money:%d  ", money);
        msg(68,1, 0xb6, buf);
    sprintf(buf, "Length:%d ", bodylen);
    msg(68,3,0xb6,buf);
    sprintf(buf, "Health:%d ", health);

    if (health < 50) {
        msg(68,5, 0x96, buf);
        if (health %2) {
            msg(68,5, 0x96, "          " );
        }
    }
    else
        msg(68,5,0xb6,buf);
    //treasure();
}

void getname(void) {
    cooked_mode();
    char in[10];
    char buf[30];
    if (py > 12)
        setpos(40,11);
    else
        setpos(40,18);
    fgets(in, 10, stdin);
    setpos(0,0);
    raw_mode();
    if (py > 12) {
      msg(30,8, 0x0f, "                            ");
      msg(30,9,0x0f,"  Press any key to restart ");
      msg(30,10,0x0f,"                           ");
    } else {
      msg(30,15, 0x0f,"                            ");
      msg(30,16,0x0f,"  Press any key to restart ");
      msg(30,17,0x0f,"                           ");
    }
    //TOP
    //c 8 buf1
    //b 9 buf2
    //a 10 buf3
    if (money > b) {
        if (money > c){
            a = b;
            //*buf3 = *buf2;
            b = c;
            //*buf2 = *buf1;
            c = money;
            sprintf(buf, "  %d$", c);
            msg(64,8, 0xe0, buf);
            msg(70,8, 0xe0, "                ");
            render();
            sprintf(buf, "by %s", in);
            msg(70,8, 0xe0, buf);
//            *buf1 = *in;
//            if (b!=-1) {
//                sprintf(buf, "  %d$", b);
//                msg(64,9, 0xe0, buf);
//                sprintf(buf, "by %s     ", buf2);
//                msg(70,9, 0xe0, buf);
//                }
//            if (a!=-1) {
//                sprintf(buf, "  %d$", a);
//                msg(64,10, 0xe0, buf);
//                sprintf(buf, "by %s     ", buf3);
//                msg(70,10, 0xe0, buf);
//                }
        }
        else {
            a = b;
            //*buf3 = *buf2;
            b = money;
//            if (b!=-1) {
//                sprintf(buf, "  %d$", b);
//                msg(64,9, 0xe0, buf);
//                sprintf(buf, "by %s         ", in);
//                msg(70,9, 0xe0, buf);
//                *buf2=*in;}
//            if (a!=-1) {
//                sprintf(buf, "  %d$", a);
//                msg(64,10, 0xe0, buf);
//                sprintf(buf, "by %s         ", buf3);
//                msg(70,10, 0xe0, buf);}
        }
    }
    else {
        a = money;
//        sprintf(buf, "  %d$", a);
//        msg(64,10, 0xe0, buf);
//        sprintf(buf, "by %s         ", in);
//        msg(70,10, 0xe0, buf);
//        *buf3=*in;
    }
    render();
}

// Interpret a key press and update the data structures.
void update(char in)
{
  prevdx = dx;
  prevdy = dy;
  switch(in) {
    case 'a':
    case 'h': dx=-1; dy=0; break;
    case 's':
    case 'j': dx=0; dy=1; break;
    case 'w':
    case 'k': dx=0; dy=-1; break;
    case 'd':
    case 'l': dx=1; dy=0; break;
    default: break;
  }
  px += dx;
  py += dy;

  if (scr[px][py] == '$') {
    getmoney();
    if (bodylen == MAXLEN-1)
        msg(17,0,0x90,"Omega-Snake-Status-Achieved");
    if (bodylen < MAXLEN)
      extend();
    else {
      move();
    }
  }
  else if (scr[px][py] == ' ') {
    move();
  }

  //run into the wall
  else if (py == 0){
    py = 22;
    move();
  }
  else if (py == 12 && px >= 61 && scr[px-1][py] != '@'){
    py = 22;
    move();
  }
  else if (py ==23) {
      if (px>=61)
          py = 13;
      else
          py = 1;
    move();
  }
  else if (px == 0) {
      if (scr[17][23] != 'C')
          px = 60;
      else
          px = 78;
      move();
  }
  else if (px == 61&& scr[px-1][py] == '@') {
      if (py < 13)
          px = 1;
      move();
  }
  else if (px == 79) {
      px = 1;
      move();
  }
  else {
    collision();
    phase = SPLASH;
    splash_ticks=0;
  }
  if (health == 0 && scr[px][py] != '$') {
      collision();
      phase = SPLASH;
      splash_ticks=0;
  }
}

//treasure!!
void treasure(){
    int x, y;
   x = (random() % 16);
   y = (random() % 16);
   if (scr[17][23] == 'C')
           msg(17, 23, x*16+y, "Congrats, you found TREASURE!");

    if (px == 61 & py > 12 & scr[17][23] != 'C') {
         msg(17, 23, 0x90, "Congrats, you found TREASURE!");
         int x, y;
         for(y=15; y<19; y++)
           for(x=66; x<72; x++) {
               msg(x, y, 0xe0, "$");
           }
         for(x=61; x<80; x++) {
           if (x==61 || x==79) {
             msg(x,12,0xb0,"+");
             msg(x,23,0xb0,"+");
           } else {
             msg(x,12,0xb0,"-");
             msg(x,23,0xb0,"-");
           }
         }
         for(y=13; y<23; y++) {
           msg(61,y, 0xb0, " ");
           msg(79,y, 0xb0, "|");
         }
     }
}
void animate(void)
{
  if (phase == SPLASH) {
    if (splash_ticks < 10) {
      while (available())
        getchar();
      splash_ticks++;
      return;
    }
    // Stall waiting for a key.
    while (!available())
      ;
    getchar();

    // Get the timer counter value for the random seed.
    int seed=get_seed();
    srandom(seed);
    init();
    clear();
    phase = RUNNING;
  }
  char in=' ';
  if (phase == RUNNING) {
    while(available()) {
      in = getchar();
    }
    if (in == 'q') {
#ifdef __linux__
      cursor_on();
      cooked_mode();
      exit(0);
#else
      splash();
      render();
      phase = SPLASH;
      splash_ticks = 0;
#endif
    }
    if (in == 'p') {
      freeze();
    }
    update(in);
    treasure();

    render();
    return;
  }
}
