#ifndef BARRIER_H_
#define BARRIER_H_

#include <atomic>

class TournamentBarrier {
  private:
    class Node {
      public:
        Node *parent_;
        Node *partner_;
        bool valid_;
        bool flag_;
        bool top_;
        Node() : parent_(nullptr), partner_(nullptr), valid_(false), flag_(false), top_(false) {};
        void await(bool sense);
    } *nodes_;
    int num_threads_;
    int base_;

  public:
    TournamentBarrier(int num_threads);
    ~TournamentBarrier();
    void await(int id, bool sense);
};

class StaticTreeBarrier {
  private:
    class Node {
      public:
        std::atomic<int> count_;
        Node() : count_(0) {};
    } *nodes_;
    int num_threads_;
    bool done_;

  public:
    StaticTreeBarrier(int num_threads);
    ~StaticTreeBarrier();
    void await(int id, bool sense);
};

class StaticTreeBarrierNoCAS {
  private:
    class Node {
      public:
        bool left_, right_;
        Node() : left_(false), right_(false) {};
    } *nodes_;
    int num_threads_;
    bool done_;

  public:
    StaticTreeBarrierNoCAS(int num_threads);
    ~StaticTreeBarrierNoCAS();
    void await(int id, bool sense);
};

#endif  // BARRIER_H_
