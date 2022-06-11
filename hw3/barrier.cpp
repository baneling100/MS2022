#include "barrier.h"
#include <cstdio>

void TournamentBarrier::Node::await(bool sense) {
    if (this->top_) return;
    if (this->parent_) {  // winner
        if (this->partner_) {  // wait only when partner exists
            while (this->flag_ != sense);
        }
        this->parent_->await(sense);
        if (this->partner_)
            this->partner_->flag_ = sense;
    } else {  // loser
        this->partner_->flag_ = sense;
        while (this->flag_ != sense);
    }
}

TournamentBarrier::TournamentBarrier(int num_threads) : num_threads_(num_threads) {
    for (base_ = 1; base_ < num_threads_; base_ *= 2);
    // inner nodes: 1 ~ (base_ - 1)
    // terminals: base_ ~ (base_ + num_threads_ - 1)
    nodes_ = new Node[base_ + num_threads_];
    // set valid
    for (int i = base_; i < base_ + num_threads_; i++)
        nodes_[i].valid_ = true;
    for (int i = base_ - 1; i >= 1; i--)
        nodes_[i].valid_ = nodes_[2 * i].valid_ || nodes_[2 * i + 1].valid_;
    // initialize terminals first
    for (int i = 1; i < base_ + num_threads_; i++)
        if (nodes_[i].valid_) {
            if ((i % 2) == 0)
                nodes_[i].parent_ = &nodes_[i / 2];
            if (nodes_[i ^ 1].valid_)
                nodes_[i].partner_ = &nodes_[i ^ 1];
        }
    // set top flag
    nodes_[1].top_ = true;
}

TournamentBarrier::~TournamentBarrier() {
    delete[] nodes_;
}

void TournamentBarrier::await(int id, bool sense) {  // id: 0 ~ num_threads - 1
    nodes_[base_ + id].await(sense);
}

StaticTreeBarrier::StaticTreeBarrier(int num_threads)
  : num_threads_(num_threads), done_(false) {
    nodes_ = new Node[num_threads];
    for (int i = 0; i < num_threads; i++) {
        if (2 * i + 2 < num_threads)
            nodes_[i].count_.store(2);
        else if (2 * i + 1 < num_threads)
            nodes_[i].count_.store(1);
    }
}

StaticTreeBarrier::~StaticTreeBarrier() {
    delete[] nodes_;
}

void StaticTreeBarrier::await(int id, bool sense) {  // id: 0 ~ num_threads - 1
    while (nodes_[id].count_.load() != 0);  // spin until zero
    if (id == 0) {
        for (int i = 0; i < num_threads_; i++) {
            if (2 * i + 2 < num_threads_)
                nodes_[i].count_.store(2);
            else if (2 * i + 1 < num_threads_)
                nodes_[i].count_.store(1);
        }
        done_ = sense;
    } else {
        int pid = (id - 1) / 2;
        nodes_[pid].count_.fetch_sub(1);
        while (done_ != sense);
    }
}

StaticTreeBarrierNoCAS::StaticTreeBarrierNoCAS(int num_threads)
  : num_threads_(num_threads), done_(false) {
    nodes_ = new Node[num_threads];
    for (int i = 0; i < num_threads; i++) {
        if (2 * i + 2 < num_threads) {
            nodes_[i].left_ = true;
            nodes_[i].right_ = true;
        }
        else if (2 * i + 1 < num_threads)
            nodes_[i].left_ = true;
    }
}

StaticTreeBarrierNoCAS::~StaticTreeBarrierNoCAS() {
    delete[] nodes_;
}

void StaticTreeBarrierNoCAS::await(int id, bool sense) {
    while (nodes_[id].left_ || nodes_[id].right_);  // spin until zero
    if (id == 0) {
        for (int i = 0; i < num_threads_; i++) {
            if (2 * i + 2 < num_threads_) {
                nodes_[i].left_ = true;
                nodes_[i].right_ = true;
            }
            else if (2 * i + 1 < num_threads_)
                nodes_[i].left_ = true;
        }
        done_ = sense;
    } else {
        int pid = (id - 1) / 2;
        if ((id % 2) == 0)
            nodes_[pid].right_ = false;
        else
            nodes_[pid].left_ = false;
        while (done_ != sense);
    }
}
