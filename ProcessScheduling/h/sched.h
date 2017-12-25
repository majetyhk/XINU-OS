#include<stdio.h>
#define EXPDISTSCHED 1
#define LINUXSCHED 2
#define DEFAULTSCHEDULER 0
#define LAMDA 0.1

int schedClass;
void setschedclass(int);
int getschedclass(void);
int getNextProcess(int,int);

