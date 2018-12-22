#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
unsigned page[100000000];
int np=0;
int time[520];
int FIFO(int no)
{
    /*FILE *fstp;
    fstp=fopen("output2.txt","w");*/
    int m,n,s; //m、n迴圈計次用，s表示是否重複
    int miss=0;
    int frame[520];
    for(m=0; m<no; m++) //清空frame內為空值，以-1代替
    {
        frame[m]=-1;
    }
    for(m=0; m<np; m++)
    {
        s=0;
        for(n=0; n<no; n++) //linear search
        {
            if(page[m] ==frame[n])//重複則跳出迴圈
            {
                s=1;
                miss--;
                break;
            }
        }
        miss++;

        if((miss == 1) && (s == 0)) //針對第一個miss(首次輸入)
        {
            frame[m]=page[m];
        }
        else if(s == 0)
        {
            frame[(miss-1)%no]=page[m]; //以%no重複放入frame
        }
        else;
       /* fprintf(fstp,"\n");
        fprintf(fstp,"miss:%d\t",miss);
        for(n = 0; n < no; n++)
        {
            fprintf(fstp,"%x\t", frame[n]);
        }*/

    }
    return miss;


}
int min(int no) //計算LRU內，時間最久沒用的page(其time值應為最小)
{
    int i,change;
    int tmp=20000000;
    for(i=0; i<no; i++)
    {
        if(time[i]<tmp)
        {
            change=i;
            tmp=time[i];
        }
    }
   // printf("check=%d",change);
    return change;
}
int LRU(int no)
{
    /*FILE *ftp;
       ftp=fopen("output3.txt","w");*/
    int m,n,s,change;
    int miss=0;
    int frame[520];
    for(m=0; m<no; m++)
    {
        frame[m]=-1;
        time[m]=0;
    }
    for(m=0; m<np; m++)
    {
        s=0;
        for(n=0; n<no; n++) //linear search
        {
            if(page[m] ==frame[n]) //frame內已有重複page時，計入最新一次使用的時間
            {
                s=1;
                time[n]=m+1;
                miss--;
                break;
            }
        }
        miss++;

        if((miss <=no) && (s == 0))
        {
            frame[miss-1]=page[m];//初始取代frame
            time[miss-1]=m+1;
        }
        else if(s == 0)
        {
            change=min(no);
           // fprintf(ftp,"change=%d\n",change);
            frame[change]=page[m];//更新frame和新page的時間
            time[change]=m+1;
        }
        else;
      //    fprintf(ftp,"\n");
      //    fprintf(ftp,"miss:%d\t",miss);
       /* for(n = 0; n < no; n++)
        {
            fprintf(ftp,"%x(%d)\t", frame[n],time[n]);
        }
*/
    }
    return miss;

}


int main(int argc,char *argv[])
{
    FILE *fPtr;
   
    int miss[8],hit[8];
    double ratio[8];
    int sec,usec;
    int tt,i,frame_num=64;
    char patt,pbtt;
    struct timeval _start,_end;
    gettimeofday(&_start,0);
       printf("FIFO---\n");
       printf("size\tmiss\thit\t\tpage fault ratio\n");
       for(i=0; i<4; i++)
       {
 np=0; 
    fPtr=fopen("trace.txt","r");
    while(fscanf(fPtr, "%c%c",&patt,&pbtt)!=EOF)
    {
        fscanf(fPtr, "%x,%d\n",&page[np],&tt);//讀數據的格式
        page[np]=page[np]>>12; //取十六進位的前五碼
        np++;
    }
    fclose(fPtr);


           miss[i]=FIFO(frame_num);
           hit[i]=np-miss[i];
           ratio[i]=(double)miss[i]/(double)np;
           printf("%d\t%d\t%d\t%.9lf\n",frame_num,miss[i],hit[i],ratio[i]);
           frame_num*=2;
       }


    frame_num=64;
    printf("LRU---\n");
    printf("size\tmiss\thit\t\tpage fault ratio\n");
    for(i=4; i<8; i++)
    {
       
 np=0; 
    fPtr=fopen("trace.txt","r");
    while(fscanf(fPtr, "%c%c",&patt,&pbtt)!=EOF)
    {
        fscanf(fPtr, "%x,%d\n",&page[np],&tt);//讀數據的格式
        page[np]=page[np]>>12; //取十六進位的前五碼
        np++;
    }
    fclose(fPtr);
	   
	miss[i]=LRU(frame_num);
        hit[i]=np-miss[i];
        ratio[i]=(double)miss[i]/(double)np;
        printf("%d\t%d\t%d\t%.9lf\n",frame_num,miss[i],hit[i],ratio[i]);
        frame_num*=2;
    }

    gettimeofday(&_end,0);
    sec = _end.tv_sec - _start.tv_sec;
    usec = _end.tv_usec - _start.tv_usec;
    printf("time: %f sec\n",sec+(usec/1000000.0));

    return 0;
}
