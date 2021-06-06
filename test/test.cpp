#include<iostream>
#include<ctime>
int main(void)
{
    double duration;
    clock_t start,end;
    time_t now =time(0);
    long long time_limit = now+5;
    start = clock();
    end = clock();
    while(end<5){
        end=clock();
    }
    end = clock();
    duration = (double)(end-start) / CLOCKS_PER_SEC;
    printf("%f seconds\n",duration);
    return 0;
}