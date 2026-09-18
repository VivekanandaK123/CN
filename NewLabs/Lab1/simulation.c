/// gcc simulation.c -o simulation -lm
// ./simulation
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX 100000

double expo(double l) {
    double u = (rand() + 1.0) / (RAND_MAX + 2.0);
    return -log(u) / l;
}

int main() {
    double pb, bw1, bw2, p1, p2, proc;
    int cap, n, seed;

    printf("Packet length: "); scanf("%lf", &pb);
    printf("Source-Router bandwidth: "); scanf("%lf", &bw1);
    printf("Router-Destination bandwidth: "); scanf("%lf", &bw2);
    printf("Source-Router propagation delay: "); scanf("%lf", &p1);
    printf("Router-Destination propagation delay: "); scanf("%lf", &p2);
    printf("Router processing delay: "); scanf("%lf", &proc);
    printf("Queue capacity: "); scanf("%d", &cap);
    printf("Number of packets: "); scanf("%d", &n);
    printf("Seed: "); scanf("%d", &seed);

    srand(seed);

    FILE *f = fopen("results.csv", "w");
    fprintf(f, "rho,lambda,delivered,dropped,drop_probability,"
               "avg_queue_delay,avg_end_to_end_delay,max_queue_occupancy\n");

    double bits = pb * 8, tx1 = bits / bw1, tx2 = bits / bw2;
    double rhos[] = {.1,.2,.3,.4,.5,.6,.7,.8,.9,.95,1,1.1,1.2};

    for (int r = 0; r < 13; r++) {
        double rho = rhos[r];
        double lambda = rho * bw2 / bits;
        double dep[MAX], arrival = 0, last = 0;
        int front = 0, rear = 0, delivered = 0, dropped = 0, maxq = 0;
        double qsum = 0, esum = 0;

        for (int i = 0; i < n; i++) {
            arrival += expo(lambda);

            double ra = arrival + tx1 + p1;

            while (front < rear && dep[front] <= ra)
                front++;

            int q = rear - front;
            if (q > maxq) maxq = q;

            if (q >= cap) {
                dropped++;
                continue;
            }

            double qdelay = last > ra ? last - ra : 0;
            double depart = (last > ra ? last : ra) + proc + tx2;

            dep[rear++] = depart;
            last = depart;

            qsum += qdelay;
            esum += tx1 + p1 + qdelay + proc + tx2 + p2;
            delivered++;
        }

        fprintf(f, "%.2f,%.6f,%d,%d,%.6f,%.6f,%.6f,%d\n",
                rho, lambda, delivered, dropped,
                (double)dropped / n,
                delivered ? qsum / delivered : 0,
                delivered ? esum / delivered : 0,
                maxq);
    }

    fclose(f);
    printf("Simulation complete. results.csv generated.\n");
    return 0;
}