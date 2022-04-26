#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <thread>
#include <atomic>

class AbortableMCSLock;
typedef AbortableMCSLock* LockPtr_t;

class AbortableMCSLock {
enum State {
    FREE,  // Thread can lock, or already locked.
    LIVE,  // Thread is alive, but other thread gets locked.
    DEAD   // Thread is aborted.
};

public:
    /* Tail should be an atomic variable. */
    AbortableMCSLock(std::atomic<LockPtr_t> &tail_) : tail(&tail_) {}

    /* Destructor is called when thread is aborted (timeout), but
     * memory space is not freed util garbage collector collects. */
    void abort() {
        state.store(State::DEAD);
        /* It means the thread dies with locked or being about to lock.
         * It should try to pick the next thread by modifying state (LIVE -> FREE). */
        if (state.load() == State::FREE) unlock();
    }

    /* After timeout, thread dies */
    bool lock(long timeout) {
        succ = nullptr;
        LockPtr_t expected = nullptr, before = nullptr;
        state.store(State::LIVE);

        while (!tail->compare_exchange_strong(expected, this)) before = expected;
        if (before == nullptr) state.store(State::FREE); // Currently linked list empty
        else before->succ = this; // Fail means there are some LIVE threads
        /* Other threads can be dead before the thread is inserted into linked list,
         * the dead threads must wait until this is inserted. Then change it to FREE. */

        time_t start = time(NULL);
        while (time(NULL) - start < timeout)
            if (state.load() == State::FREE)  // Only check out its state, MCS lock
                return true;
        abort();
        return false;
    }

    void unlock() {
        state = State::DEAD;
        LockPtr_t curr = succ, before = this;
        while (true) {
            while (curr && curr->state.load() == State::DEAD) { // Go find left-most LIVE
                before = curr;
                curr = curr->succ;  // Garbage collector can collect old curr.
            }
            if (curr) { // We’ve found LIVE thread, but could be already modified.
                State expected = State::LIVE;
                if (curr->state.compare_exchange_strong(expected, State::FREE)) break;
                /* LIVE -> FREE. If DEAD, go find left-most LIVE again. */
            } else {
                /* No LIVE, but there is possibility some threads not yet been inserted.
                   This case: tail != before (right-most of linked list) */
                LockPtr_t expected = before;
                if (tail->compare_exchange_strong(expected, nullptr)) break;
                /* tail -> nullptr. If not yet inserted, go find left-most LIVE again. */
            }
        }
    }
    // fields
    std::atomic<LockPtr_t> *tail;
    std::atomic<State> state;
    LockPtr_t succ;
};


void worker(std::atomic<LockPtr_t> *tail, long timeout) {
    LockPtr_t mylock = new AbortableMCSLock(*tail);
    if (mylock->lock(timeout)) {
        sleep(5);
        mylock->unlock();
        if (mylock->lock(timeout)) {
            sleep(5);
            mylock->unlock();
        }
    }
}

int main() {
    std::atomic<LockPtr_t> tail(nullptr);
    std::thread t1(worker, &tail, 2147483647L);
    sleep(1);
    std::thread t2(worker, &tail, 1L);
    sleep(1);
    std::thread t3(worker, &tail, 2147483647L);

    t1.join();
    t2.join();
    t3.join();
    return 0;
}
