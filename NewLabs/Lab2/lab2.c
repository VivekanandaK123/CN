//gcc lab2.c -o lab2 -lm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX 100000

double expd(double l) {
    double u=(rand()+1.0)/(RAND_MAX+2.0);
    return -log(u)/l;
}

int main() {
    int N, cap;
    long n;
    double L,bw1,bw2,p1,p2,proc;

    printf("Packet length(bytes): "); scanf("%lf",&L);
    printf("Source-Router bandwidth: "); scanf("%lf",&bw1);
    printf("Router-Destination bandwidth: "); scanf("%lf",&bw2);
    printf("Source-Router delay: "); scanf("%lf",&p1);
    printf("Router-Destination delay: "); scanf("%lf",&p2);
    printf("Processing delay: "); scanf("%lf",&proc);
    printf("Queue capacity: "); scanf("%d",&cap);
    printf("Number of sources: "); scanf("%d",&N);

    double lam[N];
    for(int i=0;i<N;i++) {
        printf("Lambda%d: ",i+1);
        scanf("%lf",&lam[i]);
    }

    printf("Packets to simulate: "); scanf("%ld",&n);
    srand(1);

    FILE *f=fopen("results.csv","w");
    fprintf(f,"rho,lambda,generated,delivered,dropped,drop_prob,"
              "avg_queue_delay,avg_e2e_delay,max_queue\n");

    double bits=L*8, tx1=bits/bw1, tx2=bits/bw2;
    double next[N], dep[MAX], last=0, sq=0, se=0;
    long front=0,rear=0,del=0,drop=0,maxq=0;

    for(int i=0;i<N;i++) next[i]=expd(lam[i]);

    for(long k=0;k<n;k++) {
        int s=0;
        for(int i=1;i<N;i++)
            if(next[i]<next[s]) s=i;

        double a=next[s];
        next[s]+=expd(lam[s]);

        double ra=a+tx1+p1;

        while(front<rear && dep[front]<=ra) front++;

        long q=rear-front;
        if(q>maxq) maxq=q;

        if(q>=cap) {
            drop++;
            continue;
        }

        double qd=last>ra ? last-ra:0;
        double d=(last>ra?last:ra)+proc+tx2;

        dep[rear++]=d;
        last=d;
        sq+=qd;
        se+=tx1+p1+qd+proc+tx2+p2;
        del++;
    }

    double lambda=0;
    for(int i=0;i<N;i++) lambda+=lam[i];

    double rho=lambda*bits/bw2;

    fprintf(f,"%.2f,%.4f,%ld,%ld,%ld,%.4f,%.6f,%.6f,%ld\n",
            rho,lambda,n,del,drop,(double)drop/n,
            del?sq/del:0,del?se/del:0,maxq);

    fclose(f);

    printf("Simulation complete. results.csv generated.\n");
}