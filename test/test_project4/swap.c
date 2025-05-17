#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#define MAX_REQ_NUM 15
uint64_t va[MAX_REQ_NUM];
typedef struct{
    int cur;
    int data;
}Write_Hist;
Write_Hist hist[MAX_REQ_NUM][20];
int hcur[MAX_REQ_NUM];

int main(){
    int i, j, k;
    int* arr;
    int arr_len = 4096/sizeof(int);
    int wri_times, data, cur;
    uint64_t pa;
    srand(clock());
    // 历史记录指针初始化
    for(int i=0; i<MAX_REQ_NUM; i++)
        hcur[i]=0;
    printf("--------------------------Start writing: Vaddr-Pa----------------------\n", i);
    for(i=0; i<MAX_REQ_NUM; i++){
        // printf("--------------------------ROUND %d----------------------\n", i);
        int dest = rand()%(MAX_REQ_NUM);
        va[i] = 0x4000000lu + (dest<<14);
        
        arr = (int*) va[i];
        wri_times = rand()%4+1;
        for(j=0; j<wri_times; j++){
            data = rand()+1;
            cur = rand()%arr_len;
            arr[cur] = data;
            for(k=0; k<hcur[dest]; k++)
                if(hist[dest][k].cur==cur)
                    break;
            // 该位置从未被修改过
            if(k==hcur[dest]){
                hist[dest][hcur[dest]].cur = cur;
                hist[dest][hcur[dest]].data = data;
                hcur[dest]++;
            }
            else{
                hist[dest][k].data = data;
            }
            // printf("Write data %d in %d\n", data, cur);
        }
        if((i+1)%3==0)
            printf("\n");
    }
    printf("--------------------------Start checking: Vaddr-Pa----------------------\n", i);
    for(i=0; i<MAX_REQ_NUM; i++){
        if(hcur[i]==0)
            continue;
        uint64_t addr = 0x4000000lu + (i<<14);
        arr = (int*)addr;
        for(k=0; k<hcur[i]; k++){
            if(arr[hist[i][k].cur]!=hist[i][k].data){
                printf("Error! BaseAddr: %lx. Hcur:%d. Expect %d in %d, but get %d\n", addr, hcur[i], hist[i][k].data, hist[i][k].cur, arr[hist[i][k].cur]);
                return 0;
            }
        }
    }
    printf("Pass!\n");
}