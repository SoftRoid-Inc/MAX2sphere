#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "bitmaplib.h"

#define LEFT   0
#define RIGHT  1
#define TOP    2
#define FRONT  3
#define BACK   4
#define DOWN   5

#define NEARLYONE 0.9999
typedef struct {
   double x,y,z;
} XYZ;

typedef struct {
	double u,v;
} UV;

typedef struct {
   double a,b,c,d;
} PLANE;

typedef struct {
   int outwidth,outheight; 
   int framewidth,frameheight;
   int antialias,antialias2;  
   PLANE faces[6];
	char outfilename[256];
   int debug;
   int read;
} PARAMS;
typedef struct {
   int face;
   float u, v;
} FUV;

typedef struct
{
   int width,height;
   int sidewidth;
   int centerwidth;
   int blendwidth;
	int equiwidth;
} FRAMESPECS;

// Prototypes
int CheckFrames(char *,char *,int *,int *);
int WriteSpherical(char *,BITMAP4 *,int,int);
int ReadFrame(BITMAP4 *,char *,int,int);
int FindFaceUV(double,double,UV *);
BITMAP4 GetColour(int,UV,BITMAP4 *,BITMAP4 *);

BITMAP4 ColourBlend(BITMAP4,BITMAP4,double);
void RotateUV90(UV *);
void Init(void);
double GetRunTime(void);
void GiveUsage(char *);

