//
//  とりあえず、GPL v3です。
//

//
//    Copyright 2013,2014 Mercy Yamada.
//    for personal use only. not for commercial use. 
//
//    This file is part of "Neil and Eliza".
//
//    "Neil and Eliza" is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    "Neil and Eliza" is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with "Neil and Eliza".  If not, see <http://www.gnu.org/licenses/>.
//


// 参考：
//http://d.hatena.ne.jp/nodamushi/20101223/1293136222
//
// これでいけちゃうかな？
//
// こっちも参考：
//http://d.hatena.ne.jp/ousttrue/20090224/1235499257
//

//
// できた？
//
// こっからtest_01.cppとペア
//


//
// 水平と垂直も付けたけど、ちょっと変。
//
// 垂直
// XT = 0;YT =1000;GuptaSproull(0,0);
// 水平
// XT = 1000;YT = 0;GuptaSproull(0,0);
//
//

//何故か、11時方向傾きでverticalが01交互になる。(どっかへん)
//

// VRAM直接で9秒が8秒に。むーん。

/*
 //グローバル変数
 extern double pitch;
 extern int view_num;
 extern int mirror;
 extern int print;
 
 extern int WX,WY;//マスクのサイズ
 extern int XT,YT;//傾き基準
 
 extern double Lvalue;//線の明るさ(間に合わせ？)
 
 
 //プロトタイプ宣言
 //double init_table(double pitch, int view_num, int mirror);
 //void GuptaSproull(double x,double y);
 
 void mergePt(unsigned int *s,unsigned int *d, int map, int x,int y);
 //void mergePt_prn(unsigned int *s,unsigned int *d, int map, int x,int y);
 
 int alloc_MAP();
 void free_MAP();
 void make_MAP();
 
 //メモリ確保
 switch (alloc_MAP()){
	case 2:
 ESP_Printf("\n---error:malloc:1---\n");
 goto end;
 break;
	case 1:
 ESP_Printf("\n---out of memory---\n");
 goto end;
 break;
 }
 //マップ作成
 make_MAP();
 
 
 //画像作成
 for (int i=0;i<view_num;i++){
	//
	for (int y=0;y<WY;y++){
 for (int x=0;x<WX;x++){
 mergePt(ESP_VramPtr[SOURCE],ESP_VramPtr[DISP],i, x,y);
 }
	}
 }
 
 //メモリ開放
 free_MAP();
 
 */


//#include <esplib.h>
//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include "ESPlib_def.h"

/*
 extern double pitch;
 extern int view_num;
 extern int mirror;
 extern int print;
 
 extern int WX,WY;//マスクのサイズ
 extern int XT,YT;//傾き基準
 
 extern double Lvalue;//線の明るさ(間に合わせ？)
 
 extern int mapPos;
 extern int mapW;
 */

//外部グローバル変数
double pitch=7;
int view_num =8;
int mirror =0;
int print =0;

//int WX=1920,WY=1080;//マスクのサイズ
//int XT=354,YT=918;//傾き基準
int WX,WY;//マスクのサイズ
int XT,YT;//傾き基準

double Lvalue=1.0;//線の明るさ(間に合わせ？)



//内部グローバル変数
int mapPos;
int mapW;

int dx,dy;
double xx0,xx1,yy0,yy1;

int XT0,YT0;

//extern unsigned char **mapP;
unsigned char **mapP;

static int table0[]=
{ 0xc7, 0xc6, 0xc2, 0xbc,
    0xb3, 0xa9, 0x9c, 0x8e,
    0x7f, 0x70, 0x62, 0x54,
    0x46, 0x3a, 0x2f, 0x25,
    0x1c, 0x15, 0x0e, 0x09,
    0x05, 0x03, 0x01, 0x00 };
static int table[24];

//線分(xx0,yy0)-(xx1,yy1)から座標(x,y)への距離？
//#define distance(x,y) abs((yy1-yy0)*(x)-(xx1-xx0)*(y)-yy1*xx0+xx1*yy0)/hypot((xx1-xx0), (yy1-yy0))
//    double distance(int x,int y){


double p1,p2,p3,p4;
//    double distance(double x,double y){
inline double distance(double x,double y){
    /*
     p1 = (yy1-yy0);
     p2 = (xx1-xx0);
     p3 = yy1*xx0+xx1*yy0;
     p4 = hypot((xx1-xx0), (yy1-yy0));
     */
    return abs(p1*x-p2*y-p3)/p4;
    //        return abs((yy1-yy0)*x-(xx1-xx0)*y-yy1*xx0+xx1*yy0)/hypot((xx1-xx0), (yy1-yy0));
}

//	int getAlph(double distance){
inline int getAlph(double distance){
    if(distance>=1)return 0;
    //        if(distance>=1.5)return 0;
    //        return table[(int)(distance*table.length)];
    return table[(int)(distance*23)];
    
}


int col;

#define XY2PT(x,y,widex) (((y)*(widex))+(x))

inline void aapset_vertical(double xx, double yy){
    int x = int(xx);
    int y = int(yy);
    if ((x<1)||(x>=(mapW-1))||(y<1)||(y>=(WY-1))) goto skipV;
    xx = double(x);
    yy = double(y);
				col = getAlph(distance(xx, yy));
				mapP[mapPos][XY2PT(x,y,mapW)]=(unsigned char)col;
				col = getAlph(distance(xx+1, yy));
				mapP[mapPos][XY2PT(x+1,y,mapW)]=(unsigned char)col;
				col = getAlph(distance(xx-1, yy));
				mapP[mapPos][XY2PT(x-1,y,mapW)]=(unsigned char)col;
skipV:;
}

inline void aapset_horizontal(double xx, double yy){
    int x = int(xx);
    int y = int(yy);
    if ((x<1)||(x>=(mapW-1))||(y<1)||(y>=(WY-1))) goto skipH;
    xx = double(x);
    yy = double(y);
				col = getAlph(distance(xx, yy));
				mapP[mapPos][XY2PT(x,y,mapW)]=(unsigned char)col;
				col = getAlph(distance(xx, yy+1));
				mapP[mapPos][XY2PT(x,y+1,mapW)]=(unsigned char)col;
				col = getAlph(distance(xx, yy-1));
				mapP[mapPos][XY2PT(x,y-1,mapW)]=(unsigned char)col;
skipH:;
}


//void GuptaSproull(double SX) {
inline void GuptaSproull(double SX, double SY) {
    
    xx0 = SX;
    yy0 = SY;
    xx1 = dx+xx0;
    yy1 = dy+yy0;
    //   ESP_Printf("(%f,%f)-(%f,%f)\n",xx0,yy0,xx1,yy1);
    
    
    p1 = (yy1-yy0);
    p2 = (xx1-xx0);
    p3 = yy1*xx0+xx1*yy0;
    p4 = hypot((xx1-xx0), (yy1-yy0));
    
    double xl,yl;//abs(xx0-xx1),abs(yy0-yy1)
    //    boolean straight;//水平方向または垂直方向に一直線かどうか
    //    boolean vertical;//垂直方向に多く進むかどうか
    bool straight;//水平方向または垂直方向に一直線かどうか
    bool vertical;//垂直方向に多く進むかどうか
    
    //        xl=xx1-xx0<0? -xx1+xx0:xx1-xx0;
    //        yl=yy1-yy0<0? -yy1+yy0:yy1-yy0;
    xl=abs(dx);
    yl=abs(dy);
    straight = yl==0 ||xl==0;
    vertical = straight?xl==0:yl>xl;
    
    
    straight = yl==0 ||xl==0;
    vertical = straight?xl==0:yl>xl;
    
    //今回は、いらない。
    if(straight){//垂直か水平に一直線
        if(vertical){
            for(double y=yy0;y!=yy1;y++) aapset_vertical(SX, y);
        }else{
            for(double x=xx0;x<WX;x++) aapset_horizontal(x, SY);
        }
        return;
    }
    
    //ESP_Printf("vertical:%d",vertical);
    
    if (yl<xl) vertical=!vertical;
    if (mirror) vertical=!vertical;
    
    //		ESP_Printf("vertical:%d\n",vertical);
    //これでいいのかな
    if(vertical){
        double dt=xl/yl;
        double x,yy;
        if (mirror){
            x=xx1;
            yy=yy1;
        } else {
            x=xx0;
            yy=yy0;
        }
        
        //			for(double y=0;y<WY;y++) {
        //			for(double y=yy;y<WY*3;y++) {
        for(double y=yy;y<WY;y++) {
            
            //center = (x,y)
            //0.5でいちばん明るい
            
            //				ESP_Pset(int(x),y,0xfffff);
            
            aapset_vertical(x, y);
            
            if (mirror){
                x += dt;
            } else {
                x -= dt;
            }
            
        }
    } else {
        double dt = yl/xl;
        double y,xx;
        if (mirror){
            y=yy0;
            xx=xx0;
        } else {
            y=yy1;
            xx=xx1;
        }
        //			for(double x=0;x<WX;x++) {
        for(double x=xx;x<mapW;x++) {
            
            //center = (x,y)
            //				ESP_Pset(int(x),int(y),0xfffff);
            
            aapset_horizontal(x, y);
            
            if (mirror){
                y += dt;
            }else{
                y -= dt;
            }
            
        }
    }
    
}

double init_table(double pitch, int view_num, int mirror)
//void init_table(double pitch, int view_num, int mirror)
{
    //	double SizeOfLine = pitch/view_num;
    double SizeOfLine = (pitch/view_num)*Lvalue;
    //	double SizeOfLine = (pitch/view_num)*0.5;
    //	for (int i=0;i<24;i++) table[i] = int(double(table[i])*SizeOfLine +0.5);
    for (int i=0;i<24;i++) table[i] = int(double(table0[i])*SizeOfLine);
    //	ESP_Printf("SizeOfLine:%f\n",SizeOfLine);
    
    
    double ml;
    if (YT !=0)	ml=(double(WY)/double(YT)); else ml=(double(WX)/double(XT0));
    dx=double(XT0)*ml;
    dy=double(YT)*ml;
    //	ESP_Printf("dx:dy %f:%f\n",dx,dy);
    /*
     int ml;
     if (YT !=0)	ml=(WY/YT)+1; else ml=(WX/XT)+1;
     dx=XT*ml;
     dy=YT*ml;
     ESP_Printf("dx:dy %d:%d\n",dx,dy);
     */
    
    
    if (!mirror) dx = -dx;
    
    //	mapW = WX*3;
    /*
     int WX=256;
     int WY=256;
     ESP_CreateImage(255,"table",0,0,WX,WY,100);
     
     ESP_Pset(0,table[0],0xff0000);
     for (int i=1;i<24;i++){
     ESP_LineTo((255*i)/23,table[i],0xff);
     }
     ESP_Pset(0,table[0],0xff0000);
     for (int i=1;i<24;i++){
     ESP_Pset((255*i)/23,table[i],0xff0000);
     }
     ESP_Update_(255);
     */
    
    return dx;
}


#define C2R(c) ((int)(((c)&0xff0000)>>16))
#define C2G(c) ((int)(((c)&0xff00)>>8))
#define C2B(c) ((int)((c)&0xff))
#define RGB2C(r,g,b) ((r)*0x10000+(g)*0x100+(b))


void test_MAP_ch(unsigned char *d,int map)
{
    int c;
    for(int y=0;y<WY;y++) {
        for(int x=0;x<WX;x++) {
            c=mapP[map][XY2PT(x,  y,mapW)];
            d[XY2PT(x*3,y,WX*3)]=c;
            d[XY2PT(x*3+1,y,WX*3)]=c;
            d[XY2PT(x*3+2,y,WX*3)]=c;
        }
    }
}
void test_MAP(unsigned int *d,int map)
{
    int c;
    for(int y=0;y<WY;y++) {
        for(int x=0;x<WX;x++) {
            c=mapP[map][XY2PT(x,  y,mapW)];
            d[XY2PT(x,y,WX)]=RGB2C(c,c,c);
        }
    }
}

void test_MAP_full(unsigned int *d,int map)
{
    int c;
    for(int y=0;y<WY;y++) {
        for(int x=0;x<mapW;x++) {
            c=mapP[map][XY2PT(x,  y,mapW)];
            d[XY2PT(x,y,mapW)]=RGB2C(c,c,c);
        }
    }
}


//void mergePt(unsigned int *s,unsigned int *d,unsigned char *map, int x,int y)
void mergePt_int(unsigned int *s,unsigned int *d, int map, int x,int y)
{
    int rs,gs,bs;
    int rd,gd,bd;
    int rm,gm,bm;
    int r,g,b;
    
    int W;
    int col;
    W=mapW/3;
    
    
				col = s[XY2PT(x,y,W)];
				rs = C2R(col);
				gs = C2G(col);
				bs = C2B(col);
    
    
    //				rm=(int)map[XY2PT(x*3,  y,mapW)];
    //				gm=(int)map[XY2PT(x*3+1,y,mapW)];
    //				bm=(int)map[XY2PT(x*3+2,y,mapW)];
    
    //				rm=255-(int)mapP[map][XY2PT(x*3,  y,mapW)];
    //				gm=255-(int)mapP[map][XY2PT(x*3+1,y,mapW)];
    //				bm=255-(int)mapP[map][XY2PT(x*3+2,y,mapW)];
				rm=(int)mapP[map][XY2PT(x*3,  y,mapW)];
				gm=(int)mapP[map][XY2PT(x*3+1,y,mapW)];
				bm=(int)mapP[map][XY2PT(x*3+2,y,mapW)];
    
    //				if ((rm==0)&&(gm==0)&&(bm==0)) continue;
    
				r=(rs*rm)/255;
				g=(gs*gm)/255;
				b=(bs*bm)/255;
				//あんまり変わらないみたい
    //				r=(rs*rm)>>8;
    //				g=(gs*gm)>>8;
    //				b=(bs*bm)>>8;
				//こっちでもいけるけど、ちょっとおかしい。
    //							r=rs&rm;
    //							g=gs&gm;
    //							b=bs&bm;
    
				col = d[XY2PT(x,y,W)];
				rd = C2R(col);
				gd = C2G(col);
				bd = C2B(col);
    
				rd+=r;
				gd+=g;
				bd+=b;
    
				if (rd>255) rd=255;
				if (gd>255) gd=255;
				if (bd>255) bd=255;
    
				d[XY2PT(x,y,W)]=RGB2C(rd,gd,bd);
    //				return( RGB2C(rd,gd,bd));
}

void mergePt(unsigned char *s,unsigned char *d, int map, int x,int y)
{
    int rs,gs,bs;
    int rd,gd,bd;
    int rm,gm,bm;
    int r,g,b;
    
    int W;
//    int col;
    W=mapW/3;
    
				rs = s[XY2PT(x*3,  y,mapW)];
				gs = s[XY2PT(x*3+1,y,mapW)];
				bs = s[XY2PT(x*3+2,y,mapW)];
    /*
     rm=(int)mapP[map][XY2PT(x*3,  y,mapW)];
     gm=(int)mapP[map][XY2PT(x*3+1,y,mapW)];
     bm=(int)mapP[map][XY2PT(x*3+2,y,mapW)];
     */
				rm=(int)mapP[map][XY2PT(x*3+2,y,mapW)];
				gm=(int)mapP[map][XY2PT(x*3+1,y,mapW)];
				bm=(int)mapP[map][XY2PT(x*3,  y,mapW)];
    
				r=(rs*rm)/255;
				g=(gs*gm)/255;
				b=(bs*bm)/255;
    
				rd = d[XY2PT(x*3,  y,mapW)];
				gd = d[XY2PT(x*3+1,y,mapW)];
				bd = d[XY2PT(x*3+2,y,mapW)];
    
				rd+=r;
				gd+=g;
				bd+=b;
    
				if (rd>255) rd=255;
				if (gd>255) gd=255;
				if (bd>255) bd=255;
    
				d[XY2PT(x*3,  y,mapW)]=rd;
				d[XY2PT(x*3+1,y,mapW)]=gd;
				d[XY2PT(x*3+2,y,mapW)]=bd;
    
    
}

void mergePt00(unsigned char *s,unsigned char *d, int map, int x,int y)
{
    int rs,gs,bs;
    int rd,gd,bd;
    int rm,gm,bm;
    int r,g,b;
    
    int W;
//    int col;
    W=mapW/3;
    
    /*
     col = s[XY2PT(x,y,W)];
     rs = C2R(col);
     gs = C2G(col);
     bs = C2B(col);
     */
				rs = s[XY2PT(x*3,  y,mapW)];
				gs = s[XY2PT(x*3+1,y,mapW)];
				bs = s[XY2PT(x*3+2,y,mapW)];
    
    
    //				rm=(int)map[XY2PT(x*3,  y,mapW)];
    //				gm=(int)map[XY2PT(x*3+1,y,mapW)];
    //				bm=(int)map[XY2PT(x*3+2,y,mapW)];
    
    //				rm=255-(int)mapP[map][XY2PT(x*3,  y,mapW)];
    //				gm=255-(int)mapP[map][XY2PT(x*3+1,y,mapW)];
    //				bm=255-(int)mapP[map][XY2PT(x*3+2,y,mapW)];
				rm=(int)mapP[map][XY2PT(x*3,  y,mapW)];
				gm=(int)mapP[map][XY2PT(x*3+1,y,mapW)];
				bm=(int)mapP[map][XY2PT(x*3+2,y,mapW)];
    
    //				if ((rm==0)&&(gm==0)&&(bm==0)) continue;
    
				r=(rs*rm)/255;
				g=(gs*gm)/255;
				b=(bs*bm)/255;
				//あんまり変わらないみたい
    //				r=(rs*rm)>>8;
    //				g=(gs*gm)>>8;
    //				b=(bs*bm)>>8;
				//こっちでもいけるけど、ちょっとおかしい。
    //							r=rs&rm;
    //							g=gs&gm;
    //							b=bs&bm;
    
    /*
     col = d[XY2PT(x,y,W)];
     rd = C2R(col);
     gd = C2G(col);
     bd = C2B(col);
     */
    
				rd = d[XY2PT(x*3,  y,mapW)];
				gd = d[XY2PT(x*3+1,y,mapW)];
				bd = d[XY2PT(x*3+2,y,mapW)];
    
				rd+=r;
				gd+=g;
				bd+=b;
    
				if (rd>255) rd=255;
				if (gd>255) gd=255;
				if (bd>255) bd=255;
    
    //				d[XY2PT(x,y,W)]=RGB2C(rd,gd,bd);
				d[XY2PT(x*3,  y,mapW)]=rd;
				d[XY2PT(x*3+1,y,mapW)]=gd;
				d[XY2PT(x*3+2,y,mapW)]=bd;
    
    //				return( RGB2C(rd,gd,bd));
}

//void mergePt_prn(unsigned int *s,unsigned int *d,unsigned char *map, int x,int y)
void mergePt_prn(unsigned int *s,unsigned int *d, int map, int x,int y)
{
    int rs,gs,bs;
    int rd,gd,bd;
    int rm,gm,bm;
    int r,g,b;
    
    int W;
    int col;
    W=mapW;
    //	W=mapW/3;
    
    
				col = s[XY2PT(x,y,W)];
				rs = C2R(col);
				gs = C2G(col);
				bs = C2B(col);
    
    
    //				rm=(int)map[XY2PT(x,y,mapW)];
    //				gm=(int)map[XY2PT(x,y,mapW)];
    //				bm=(int)map[XY2PT(x,y,mapW)];
				rm=(int)mapP[map][XY2PT(x*3,  y,mapW)];
				gm=(int)mapP[map][XY2PT(x*3+1,y,mapW)];
				bm=(int)mapP[map][XY2PT(x*3+2,y,mapW)];
    
    //				if ((rm==0)&&(gm==0)&&(bm==0)) continue;
    
				r=(rs*rm)/255;
				g=(gs*gm)/255;
				b=(bs*bm)/255;
				//あんまり変わらないみたい
    //				r=(rs*rm)>>8;
    //				g=(gs*gm)>>8;
    //				b=(bs*bm)>>8;
				//こっちでもいけるけど、ちょっとおかしい。
    //							r=rs&rm;
    //							g=gs&gm;
    //							b=bs&bm;
    
				col = d[XY2PT(x,y,W)];
				rd = C2R(col);
				gd = C2G(col);
				bd = C2B(col);
    
				rd+=r;
				gd+=g;
				bd+=b;
    
				if (rd>255) rd=255;
				if (gd>255) gd=255;
				if (bd>255) bd=255;
    
				d[XY2PT(x,y,W)]=RGB2C(rd,gd,bd);
    //				return( RGB2C(rd,gd,bd));
}


int alloc_MAP()
// 正常：0
// 一段階エラー：1
// 二段階エラー：2
{
    
    if (print) mapW=WX;
    else {
        mapW=WX*3;
        //	XT *=3;
        XT0 = XT*3;
    }
    
    int sx=mapW;
    int sy=WY;
    
    if ((mapP = (unsigned char **)malloc( (view_num) * sizeof(unsigned char *) )) == NULL ) {
        //	ESP_Printf("\n---error:malloc:1---\n");
        //	goto end;
        return 1;
    }
    for (int i=0;i<view_num;i++){
        if ( (mapP[i] = (unsigned char *)malloc( ((sx)*(sy)) * sizeof(unsigned char) )) == NULL ) {
            //		ESP_Printf("\n---out of memory---\n");
            //		for (int ii=(i-1);i==0;i--) free(mapP[ii]);
            //		goto end;
            return 2;
        }
    }
    
    for(int i=0;i<view_num;i++)
        for(int p=0;p<(sx*sy);p++) 
            mapP[i][p] = 0;
    
    
    return 0;
}



void free_MAP()
{
    for (int i=0;i<view_num;i++) free(mapP[i]);
}


void make_MAP()
{
    double dx = init_table(pitch,view_num,mirror);
    
    //	printf("mapW:%d dt:%f\n",mapW,pitch);
    
//    int count=int((mapW)/pitch);
    
    double dt = pitch/double(view_num);
    //	ESP_Printf("count:%d dt:%f\n",count,dt);
    //	printf("count:%d dt:%f\n",count,dt);
    
    
    //plan B
    int ct=0;
    double pt=0;
    
//    double sp,ep;
    double ep;
    
    if (mirror){
        pt = -dx;
        //		ep = WX*3;
        ep = mapW;
    }else {
        pt = 0;
        //		ep = WX*3-dx;
        ep = mapW-dx;
    }
    
    
    //	ESP_Printf("pt:%f ep:%f\n",pt,ep);
    //	printf("pt:%f ep:%f\n",pt,ep);
    while(pt<ep){
        //		CM();
        //		mapPos=MAP+ct;
        mapPos=ct;
        
        GuptaSproull(pt,0);
        
        pt+=dt;
        ct++;if (ct==view_num) ct=0;
    }
    
    
}





/*
 void ESP_Ready(void)
 {
 ESP_OpenTextWindow(0,132,199,198,800);
 
 ESP_START=1;
 
 }
 
 
 void ESP_Main(void)
 {
	int WX=200;
	int WY=200;
	ESP_CreateImage(0,"Image0",0,0,WX,WY,500);
 
 
 //	int dx0=780;
 //	int dy0=940;
 
 //	double pitch=9.8;
 //	int view_num=32;
 
	init_table(pitch,view_num);
 
 
 
 
 
	GuptaSproull(100.375);
 
 
	ESP_Update_(0);
 
	WX=256;
	WY=256;
	ESP_CreateImage(1,"table",0,0,WX,WY,100);
 
	ESP_Pset(0,table[0],0xff0000);
	for (int i=1;i<24;i++){
 ESP_LineTo((255*i)/23,table[i],0xff);
	}
	ESP_Pset(0,table[0],0xff0000);
	for (int i=1;i<24;i++){
 ESP_Pset((255*i)/23,table[i],0xff0000);
	}
	ESP_Update_(1);
 
 }
 
 void ESP_Finish(void)
 {
 }
 
 */

