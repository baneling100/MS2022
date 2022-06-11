#include <cstdio>
#include <thread>
#include "barrier.h"

#define MAX_NUM_THREADS 256

void worker(int id, TournamentBarrier *tb,
            StaticTreeBarrier *stb, StaticTreeBarrierNoCAS *stbnc, int num_iter) {
    bool sense = true;
    for (int i = 0; i < num_iter; i++) {
        printf("id: %d waits %d th TournamentBarrier\n", id, i);
        tb->await(id, sense);
        sense = !sense;
    }

    sense = true;
    for (int i = 0; i < num_iter; i++) {
        printf("id: %d waits %d th StaticTreeBarrier\n", id, i);
        stb->await(id, sense);
        sense = !sense;
    }

    sense = true;
    for (int i = 0; i < num_iter; i++) {
        printf("id: %d waits %d th StaticTreeBarrierNoCAS\n", id, i);
        stbnc->await(id, sense);
        sense = !sense;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: ./main <num_threads> <num_iteration>\n");
        return -1;
    }
    int num_threads = std::atoi(argv[1]);
    int num_iter = std::atoi(argv[2]);
    if (num_threads > MAX_NUM_THREADS) {
        printf("number of threads: %d exceeds maximum: %d\n", num_threads, MAX_NUM_THREADS);
        return -1;
    }
    printf("number of threads: %d, number of iterations: %d\n", num_threads, num_iter);

    TournamentBarrier tb(num_threads);
    StaticTreeBarrier stb(num_threads);
    StaticTreeBarrierNoCAS stbnc(num_threads);
    std::thread threads[MAX_NUM_THREADS];
    for (int i = 0; i < num_threads; i++)
        threads[i] = std::thread(worker, i, &tb, &stb, &stbnc, num_iter);
    for (int i = 0; i < num_threads; i++)
        threads[i].join();

    return 0;
}
