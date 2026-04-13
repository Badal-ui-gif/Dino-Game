#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#define WIDTH 90
#define HEIGHT 28

#define MAX_ENEMIES 4
#define MAX_BULLETS 5
#define MAX_BOSS_BULLETS 6

#define BASE_FPS 22
#define SHOOT_COOLDOWN 6

// -------- FUNCTION PROTOTYPES --------
void showGameOver();

// -------- STRUCTS --------
typedef struct{
    int x,y,speed,type;
}Enemy;

typedef struct{
    int x,y,active;
}Bullet;

// -------- GLOBALS --------
Enemy enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];
Bullet bossBullets[MAX_BOSS_BULLETS];

char screen[HEIGHT][WIDTH+1];

HANDLE console;

int dinoX=10;
int dinoY=HEIGHT-6;

int velocity=0;
int gravity=1;

int score=0;
int health=100;
int level=1;

int shootTimer=0;

int bossHP=200; // reduced
int bossX=65;
int bossY=8;
int bossDir=1;
int bossDirY=1;

int terrainOffset=0;

int bg1=0,bg2=0,bg3=0;

int highscore=0;

int gameSpeed=BASE_FPS;
int gameOver=0;

// -------- SPRITES --------
char *dino[4]={
"  __ ",
" (oo)",
"/|__|",
" /  \\"
};

char *enemy1[3]={
" /V\\ ",
"(oo)",
"/||\\"
};

char *enemy2[3]={
" /W\\ ",
"(xx)",
"/||\\"
};

// -------- FUNCTIONS --------

void setLevelColor()
{
if(level==1) SetConsoleTextAttribute(console,10);
if(level==2) SetConsoleTextAttribute(console,11);
if(level==3) SetConsoleTextAttribute(console,14);
if(level==4) SetConsoleTextAttribute(console,13);
if(level>=5) SetConsoleTextAttribute(console,12);
}

void levelDialogue()
{
system("cls");

printf("\n\nYou found another shard of your egg...\n");

if(level==1) printf("The journey begins.\n");
if(level==2) printf("Enemies grow stronger.\n");
if(level==3) printf("The skies darken.\n");
if(level==4) printf("You feel the boss nearby.\n");
if(level==5) printf("The final shard awaits.\n");

printf("\nPress any key...");
getch();
}

void clearBuffer()
{
for(int y=0;y<HEIGHT;y++)
{
for(int x=0;x<WIDTH;x++)
screen[y][x]=' ';
screen[y][WIDTH]='\0';
}
}

void drawText(int x,int y,char *text)
{
for(int i=0;text[i]!=0;i++)
if(x+i>=0 && x+i<WIDTH && y>=0 && y<HEIGHT)
screen[y][x+i]=text[i];
}

void drawSprite(char **sprite,int h,int x,int y)
{
for(int i=0;i<h;i++)
if(y+i>=0 && y+i<HEIGHT)
drawText(x,y+i,sprite[i]);
}

void render()
{
COORD c={0,0};
SetConsoleCursorPosition(console,c);

for(int y=0;y<HEIGHT;y++)
printf("%s\n",screen[y]);
}

void hideCursor()
{
CONSOLE_CURSOR_INFO ci;
ci.dwSize=1;
ci.bVisible=FALSE;
SetConsoleCursorInfo(console,&ci);
}

void loadHighscore()
{
FILE *f=fopen("highscore.dat","r");
if(f){ fscanf(f,"%d",&highscore); fclose(f); }
}

void saveHighscore()
{
if(score>highscore)
{
FILE *f=fopen("highscore.dat","w");
fprintf(f,"%d",score);
fclose(f);
}
}

void initEnemies()
{
for(int i=0;i<MAX_ENEMIES;i++)
{
enemies[i].x=WIDTH+rand()%60;
enemies[i].y=HEIGHT-6;
enemies[i].speed=1+level/2;
enemies[i].type=rand()%2;
}
}

void initBullets()
{
for(int i=0;i<MAX_BULLETS;i++)
bullets[i].active=0;

for(int i=0;i<MAX_BOSS_BULLETS;i++)
bossBullets[i].active=0;
}

void shoot()
{
if(shootTimer>0) return;

for(int i=0;i<MAX_BULLETS;i++)
if(!bullets[i].active)
{
bullets[i].active=1;
bullets[i].x=dinoX+5;
bullets[i].y=dinoY+1;
shootTimer=SHOOT_COOLDOWN;
break;
}
}

void bossShoot()
{
if(rand()%25!=0) return;

for(int i=0;i<MAX_BOSS_BULLETS;i++)
if(!bossBullets[i].active)
{
bossBullets[i].active=1;
bossBullets[i].x=bossX-1;
bossBullets[i].y=bossY+2;
break;
}
}

void physics()
{
velocity += gravity;
if(velocity > 5) velocity = 5;

dinoY += velocity;

if(dinoY>HEIGHT-6)
{
dinoY=HEIGHT-6;
velocity=0;
}
}

void updateEnemies()
{
for(int i=0;i<MAX_ENEMIES;i++)
{
enemies[i].x -= enemies[i].speed;

if(enemies[i].x<0)
enemies[i].x = WIDTH + rand()%60;

if(abs(enemies[i].x-dinoX)<2 &&
   abs(enemies[i].y-dinoY)<2)
{
health -= 10;

if(health<=0)
{
gameOver=1;
showGameOver();
}

enemies[i].x=WIDTH+rand()%60;
}
}
}

void updateBullets()
{
for(int i=0;i<MAX_BULLETS;i++)
if(bullets[i].active)
{
bullets[i].x += 2;

if(bullets[i].x>WIDTH)
bullets[i].active=0;

for(int j=0;j<MAX_ENEMIES;j++)
if(abs(bullets[i].x-enemies[j].x)<2 &&
   abs(bullets[i].y-enemies[j].y)<2)
{
score+=20;
enemies[j].x=WIDTH+rand()%80;
bullets[i].active=0;
}

if(abs(bullets[i].x-bossX)<6 &&
   abs(bullets[i].y-bossY)<3)
{
bossHP-=10;
bullets[i].active=0;
score+=50;
}
}

if(shootTimer>0) shootTimer--;
}

void updateBossBullets()
{
for(int i=0;i<MAX_BOSS_BULLETS;i++)
if(bossBullets[i].active)
{
bossBullets[i].x--;

if(bossBullets[i].x<0)
bossBullets[i].active=0;

if(abs(bossBullets[i].x-dinoX)<2 &&
   abs(bossBullets[i].y-dinoY)<2)
{
health-=10;
bossBullets[i].active=0;

if(health<=0)
{
gameOver=1;
showGameOver();
}
}
}
}

void updateTerrain()
{
terrainOffset++;
bg1++;
bg2+=2;
bg3+=3;
}

void drawParallax()
{
for(int x=0;x<WIDTH;x++)
{
screen[(x+bg1)%6][x]='.';
screen[(x+bg2)%10][x]='*';
screen[(x+bg3)%14][x]='+';
}
}

void drawGround()
{
for(int x=0;x<WIDTH;x++)
{
int p=(x+terrainOffset)%6;
screen[HEIGHT-3][x]= (p<3)?'_':'-';
}
}

void drawGame()
{
clearBuffer();

drawParallax();
drawGround();

drawSprite(dino,4,dinoX,dinoY);

for(int i=0;i<MAX_ENEMIES;i++)
{
if(enemies[i].type==0)
drawSprite(enemy1,3,enemies[i].x,enemies[i].y);
else
drawSprite(enemy2,3,enemies[i].x,enemies[i].y);
}

for(int i=0;i<MAX_BULLETS;i++)
if(bullets[i].active)
drawText(bullets[i].x,bullets[i].y,">>");

render();

printf("LEVEL:%d SCORE:%d HEALTH:%d HIGHSCORE:%d\n",
level,score,health,highscore);
}

void showGameOver()
{
system("cls");

printf("\nGAME OVER\n");
printf("Score: %d\n",score);
printf("Highscore: %d\n",highscore);

saveHighscore();

printf("\nPress any key...");
getch();

exit(0);
}

void bossFight()
{
    initBullets();

    bossHP = 200;
    bossX = 65;
    bossY = 6;

    int moveTimer = 0;

    while(bossHP > 0)
    {
        clearBuffer();

        drawParallax();
        drawGround();

        drawSprite(dino,4,dinoX,dinoY);

        // -------- BOSS MOVEMENT --------
        moveTimer++;

        if(moveTimer % 20 == 0)
        {
            if(dinoY < bossY) bossY--;
            else if(dinoY > bossY) bossY++;
        }

        // -------- DRAW BOSS --------
        drawText(bossX, bossY,     "  /MMMM\\  ");
        drawText(bossX, bossY + 1, " | O  O | ");
        drawText(bossX, bossY + 2, " | ---- | ");
        drawText(bossX, bossY + 3, "  \\____/  ");

        // -------- BOSS SHOOT --------
        if(rand()%15 == 0)
        {
            for(int i=0;i<MAX_BOSS_BULLETS;i++)
            if(!bossBullets[i].active)
            {
                bossBullets[i].active = 1;
                bossBullets[i].x = bossX - 1;
                bossBullets[i].y = bossY + (rand()%3);
                break;
            }
        }

        updateBossBullets();

        // draw boss bullets
        for(int i=0;i<MAX_BOSS_BULLETS;i++)
        if(bossBullets[i].active)
            drawText(bossBullets[i].x,bossBullets[i].y,"<");

        // -------- PLAYER BULLETS --------
        for(int i=0;i<MAX_BULLETS;i++)
        if(bullets[i].active)
        {
            drawText(bullets[i].x,bullets[i].y,">>");

            if(bullets[i].x >= bossX-1 &&
               bullets[i].x <= bossX + 12 &&
               bullets[i].y >= bossY-1 &&
               bullets[i].y <= bossY + 4)
            {
                bossHP -= 10;
                bullets[i].active = 0;
                score += 50;
            }
        }

        render();

        // -------- UI --------
        printf("BOSS HP: [");
        int bar = bossHP / 10;
        for(int i=0;i<20;i++)
            printf(i < bar ? "#" : " ");
        printf("] %d\n", bossHP);

        printf("PLAYER HP: %d\n", health);

        // -------- INPUT --------
        if(kbhit())
        {
            char ch=getch();

            if(ch=='j' && velocity==0) velocity=-7;

            // ✅ FIX: manual shooting works now
            if(ch=='s') shoot();
        }

        // ✅ OPTIONAL: smoother auto-fire (can remove if unwanted)
        if(shootTimer == 0)
            shoot();

        // -------- UPDATE --------
        physics();
        updateTerrain();
        updateBullets(); // IMPORTANT: keeps cooldown working

        Sleep(1000/gameSpeed);
    }

    printf("\nBOSS DEFEATED!\n");
    Sleep(2000);
}

void gameLoop()
{
while(level<=5)
{
setLevelColor();
levelDialogue();

initEnemies();
initBullets();

gameSpeed=BASE_FPS+(level*1);

int target=level*500;

while(score<target)
{
if(kbhit())
{
char ch=getch();
if(ch=='j' && velocity==0) velocity=-7;
if(ch=='s') shoot();
}

physics();
updateEnemies();
updateBullets();
updateTerrain();

drawGame();

Sleep(1000/gameSpeed);
}

level++;
health+=20;
}

bossFight();
}

void menu()
{
while(1)
{
system("cls");

printf("==== ULTRA DINO ADVENTURE ====\n");
printf("1. Start Game\n");
printf("2. Exit\n");

char c=getch();

if(c=='1') break;
if(c=='2') exit(0);
}
}

int main()
{
console=GetStdHandle(STD_OUTPUT_HANDLE);

srand(time(0));
hideCursor();
loadHighscore();

menu();
gameLoop();

saveHighscore();

printf("\nFINAL SCORE: %d\n",score);

return 0;
}