#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
//variables
typedef struct color{
    int r,g,b;
}color;

char massive_buff[100000000];
//char *massive_buff;
int imgw,imgh,numchan;
int printmode = 0;
unsigned char *data;
int numelem;
int row_w;
int base_cutoff = 384;
char *printimg;
//pure funcs
char* mystrcat(char* dest,char* src )
{
     while (*dest) dest++;
     while ((*src)){
         (*dest) = (*src);
         dest++;
         src++;}
    (*dest) = (*src);
     return --dest;
}

//impure funcs
void setup(){
     (*massive_buff) = '\0';
     printimg = massive_buff;
     return;
}

unsigned int locbraille[] = {0,3,1,4,2,5,6,7};

void printmode0(){
    setup();
    row_w = imgw>>1;// width of element is 2
    color coloravg = {0,0,0};
    unsigned char printer = 0;
    int indxs[8];
    char elembuf[200];
    char* bufpointer = elembuf;
    int count = 0;
    char base[] = "⠀";
    char proj[4] = {base[0],0,0,base[3]};
    numelem = (imgw*imgh)>>3;
    for(int m = 0; m<numelem;m++){
        coloravg.r = 0; coloravg.g = 0; coloravg.b = 0;
        printer = 0; count = 0;
        indxs[0] = 4*imgw*(m/row_w)+2*(m%row_w);
        indxs[1] = indxs[0]+1;
        indxs[2] = indxs[0]+imgw;
        indxs[3] = indxs[2]+1;
        indxs[4] = indxs[2]+imgw;
        indxs[5] = indxs[4]+1;
        indxs[6] = indxs[4]+imgw;
        indxs[7] = indxs[6]+1;
        for(int j=0;j<8;j++){
            if(*(data+indxs[j]*numchan)+*(data+indxs[j]*numchan+1)+*(data+indxs[j]*numchan+2)>=base_cutoff){
                //printf("%08b " ,0b1<<locbraille[j]);
                printer = printer | 0b1<<locbraille[j];
                coloravg.r = coloravg.r + *(data+indxs[j]*numchan);
                coloravg.g = coloravg.g + *(data+indxs[j]*numchan+1);
                coloravg.b = coloravg.b + *(data+indxs[j]*numchan+2);
                count++;
            }
        }
        if(count !=0){
            coloravg.r = coloravg.r/count;
            coloravg.g = coloravg.g/count;
            coloravg.b = coloravg.b/count;
        }
        proj[2] = base[2] | (printer & 0b00111111);
        proj[1] = base[1] | (printer >> 6);

        snprintf(bufpointer,200,"\x1b[38;2;%03d;%03d;%03dm%s",coloravg.r ,coloravg.g ,coloravg.b,proj);
        printimg = mystrcat(printimg,bufpointer);
        bufpointer = elembuf;
        if(m%row_w==row_w-1){
            printimg = mystrcat(printimg,"\x1b[0m \n");
        }
    }
    //printimg = mystrcat(printimg,"\x1b[0m\0");
}
unsigned char block[]={ 0x88, 0x98, 0x9d, 0x80, 0x96, 0x8c, 0x9e, 0x9b,
                        0x97, 0x9a, 0x90, 0x9c, 0x84, 0x99, 0x9f, 0x88 };
void printmode2(){
    setup();
    row_w = imgw>>1;// width of element is 2
    color colorlow = {0,0,0}; color colorhigh = {0,0,0}; color coloravg = {0,0,0};
    unsigned char printer = 0;
    int indxs[4];
    char elembuf[200];
    char* bufpointer = elembuf;
    int count = 0;
    char proj[] = "█";
    //char proj[4] = {base[0],0,0,base[3]};
    numelem = (imgw*imgh)>>2;
    int relative_cutoff;
    for(int m = 0; m<numelem;m++){
        colorlow.r = 0; colorlow.g = 0; colorlow.b = 0;
        colorhigh.r = 0; colorhigh.g = 0; colorhigh.b = 0;
        coloravg.r = 0; coloravg.g = 0; coloravg.b = 0;
        printer = 0; count = 0;
        indxs[0] = 2*imgw*(m/row_w)+2*(m%row_w);
        indxs[1] = indxs[0]+1;
        indxs[2] = indxs[0]+imgw;
        indxs[3] = indxs[2]+1;
        for(int j=0;j<4;j++){
                coloravg.r = coloravg.r + *(data+indxs[j]*numchan);
                coloravg.g = coloravg.g + *(data+indxs[j]*numchan+1);
                coloravg.b = coloravg.b + *(data+indxs[j]*numchan+2);
        }
        relative_cutoff = (coloravg.r+coloravg.g+coloravg.b)>>2;//div by 4
        for(int j=0;j<4;j++){
            if(*(data+indxs[j]*numchan)+*(data+indxs[j]*numchan+1)+*(data+indxs[j]*numchan+2)>=relative_cutoff){
                printer = printer | 0b1<<j;
                colorhigh.r = colorhigh.r + *(data+indxs[j]*numchan);
                colorhigh.g = colorhigh.g + *(data+indxs[j]*numchan+1);
                colorhigh.b = colorhigh.b + *(data+indxs[j]*numchan+2);
                count++;
            }else{
                colorlow.r = colorlow.r + *(data+indxs[j]*numchan);
                colorlow.g = colorlow.g + *(data+indxs[j]*numchan+1);
                colorlow.b = colorlow.b + *(data+indxs[j]*numchan+2);
            }
        }
        if(count==4){
            colorlow.r = coloravg.r;
            colorlow.g = coloravg.g;
            colorlow.b = coloravg.b;
            colorhigh.r = colorhigh.r>>2;
            colorhigh.g = colorhigh.g>>2;
            colorhigh.b = colorhigh.b>>2;
        }
        else if(count==0){
            colorhigh.r = coloravg.r;
            colorhigh.g = coloravg.g;
            colorhigh.b = coloravg.b;
            colorlow.r = colorlow.r>>2;
            colorlow.g = colorlow.g>>2;
            colorlow.b = colorlow.b>>2;
        }
        else{
            colorhigh.r = colorhigh.r/count;
            colorhigh.g = colorhigh.g/count;
            colorhigh.b = colorhigh.b/count;
            colorlow.r = colorlow.r/(4-count);
            colorlow.g = colorlow.g/(4-count);
            colorlow.b = colorlow.b/(4-count);
        }
        proj[2] = block[printer];
        snprintf(bufpointer,200,"\x1b[48;2;%03d;%03d;%03dm",colorlow.r,colorlow.g,colorlow.b);
        printimg = mystrcat(printimg,bufpointer);
        bufpointer = elembuf;
        snprintf(bufpointer,200,"\x1b[38;2;%03d;%03d;%03dm%s",colorhigh.r ,colorhigh.g ,colorhigh.b,proj);
        printimg = mystrcat(printimg,bufpointer);
        bufpointer = elembuf;
        if(m%row_w==row_w-1){
            printimg = mystrcat(printimg,"\x1b[0m \n");
        }
    }
    printimg = mystrcat(printimg,"\x1b[0m\0");
}


int main(int argc, char *argv[]) {
    if(argv[1]==NULL){
        printf("Proper usage is with filename as 1st argument!\n");
        return 0;
    }
    if(argv[2]!=0){
        printmode = atoi(argv[2]);
        if(argv[3]!=0){
            base_cutoff = atoi(argv[3]);
        }
    }
    data = stbi_load(argv[1], &imgw, &imgh, &numchan, 0);
    switch (printmode) {
        case 0:
           printmode0();
        break;
        case 2:
            printmode2();
        break;
        default:
            printmode0();
    }
    printf("%s\n",massive_buff);

    stbi_image_free(data);


}
