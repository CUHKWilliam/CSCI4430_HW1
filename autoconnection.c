#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>

#define TESTNUM 1
int main(){
    int i;
    char argv[6][20] = {"./myftpclient\0","127.0.0.1\0","12345\0","put\0","file \0", NULL};
    for(i = 0;i < TESTNUM;i++){
        int fid = fork();
        if(fid == 0){
            continue;
        }else{
            printf("child:%d\n", i);
            argv[4][4] = i + '0';
            printf("%s\n", argv[4]);
            execv("myftpclient", argv);
        }
    }
    return 0;
}