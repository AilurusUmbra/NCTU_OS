#include<stdio.h>
#include<stdlib.h>


int main(){

    FILE *f;
    f = fopen("input.txt", "w");
    int n;
    printf("Enter number:");
    scanf("%d", &n);
    fprintf(f, "%d \n", n);
    for(int i=0; i<n-1; ++i)
        fprintf(f, "%d ", rand()%10000);
    fprintf(f, "%d", rand()%10000);

    fclose(f);
    return 0;
}

