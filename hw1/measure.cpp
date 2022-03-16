#include <cstdio>
#include <chrono>
#include <atomic>
#include <thread>

void worker(std::atomic<long> *acc_atomic)
{
    while (acc_atomic->fetch_add(1L, std::memory_order_relaxed) < 1000000000L);
}

int main()
{
    long acc = 0L;
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000000; i++) acc++;
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    printf("single thread: %.9lf\n", elapsed.count() * 1e-9);

    std::atomic<long> acc_atomic{0};
    begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000000; i++) (void)acc_atomic.fetch_add(1L, std::memory_order_relaxed);
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    printf("signle thread with atomic op: %.9lf\n", elapsed.count() * 1e-9);

    acc_atomic.exchange(0L, std::memory_order_relaxed);
    begin = std::chrono::high_resolution_clock::now();
    std::thread helper0(worker, &acc_atomic), helper1(worker, &acc_atomic),
        helper2(worker, &acc_atomic), helper3(worker, &acc_atomic), helper4(worker, &acc_atomic),
        helper5(worker, &acc_atomic), helper6(worker, &acc_atomic), helper7(worker, &acc_atomic);
    helper0.join(); helper1.join(); helper2.join(); helper3.join();
    helper4.join(); helper5.join(); helper6.join(); helper7.join();
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    printf("multi thread with atomic op: %.9lf\n", elapsed.count() * 1e-9);

    return 0;
}
