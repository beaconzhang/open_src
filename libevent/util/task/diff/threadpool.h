// @author Yu Hongjin (yuhongjin@meituan.com)

#include <stdio.h>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"

class Thread {
  public:
    Thread() {
        state = TSTATE_NONE;
        handle = 0;
    }

    virtual ~Thread() {
        assert(state != TSTATE_STARTED || joined);
    }

    void start() {
        assert(state == TSTATE_NONE);
        if (pthread_create(&handle, NULL, threadProc, this)) {
            abort();
        }
        state = TSTATE_STARTED;
    }

    void join() {
        assert(state == TSTATE_STARTED);
        pthread_join(handle, NULL);
        state = TSTATE_JOINED;
    }

  protected:
    virtual void run() = 0;

  private:
    static void* threadProc(void* param) {
        Thread* thread = reinterpret_cast<Thread*>(param);
        thread->run();
        return NULL;
    }

    enum TState {
        TSTATE_NONE,
        TSTATE_STARTED,
        TSTATE_JOINED
    };

    TState state;
    bool joined;
    pthread_t handle;
};

class Task {
  public:
    Task() {}
    virtual ~Task() {}
    virtual void run() = 0;
};

class WorkQueue {
  public:
    WorkQueue() {
        pthread_mutex_init(&qmtx, 0);
        pthread_cond_init(&wcond, 0);
    }

    ~WorkQueue() {
        pthread_mutex_destroy(&qmtx);
        pthread_cond_destroy(&wcond);
    }

    Task* nextTask() {
        Task* nt = 0;
        pthread_mutex_lock(&qmtx);
        while (tasks.empty()) {
            pthread_cond_wait(&wcond, &qmtx);
        }
        nt = tasks.front();
        tasks.pop();
        pthread_mutex_unlock(&qmtx);
        return nt;
    }

    void addTask(Task* nt) {
        pthread_mutex_lock(&qmtx);
        tasks.push(nt);
        pthread_cond_signal(&wcond);
        pthread_mutex_unlock(&qmtx);
    }

    size_t taskNum() const {
        size_t n = 0;
        pthread_mutex_lock(&qmtx);
        n = tasks.size();
        pthread_mutex_unlock(&qmtx);
        return n;
    }

  private:
    std::queue<Task*> tasks;
    pthread_mutex_t qmtx;
    pthread_cond_t wcond;
};

class PoolWorkerThread : public Thread {
  public:
    PoolWorkerThread(WorkQueue& _work_queue) : work_queue(_work_queue) {}

  protected:
    virtual void run() {
        while (Task* task = work_queue.nextTask()) {
            task->run();
        }
    }

  private:
    WorkQueue& work_queue;
};

class ThreadPool {
  public:
    ThreadPool(int n, int count=0) : total(count) {
        for (int i=0; i<n; ++i) {
            threads.push_back(new PoolWorkerThread(workQueue));
            threads.back()->start();
        }
        start_time = timestampNow();
        console_mutex = PTHREAD_MUTEX_INITIALIZER;
    }

    ~ThreadPool() {
        finish();
    }

    void addTask(Task* nt) {
        workQueue.addTask(nt);
    }

    void counting() const {
        int k = 1;
        if (total >= 10) {
            while (true) {
                int f = (int)(total - workQueue.taskNum()) * 100 / 10 / total;
                if (f >= k) {
                    pthread_mutex_lock(&console_mutex);
                    printf("   -- finish: %3d%%  %s %.2fs\n",
                           f * 10,
                           timeNowPrintf("%T").c_str(),
                           (float)(timestampNow() - start_time) / 1e6);
                    pthread_mutex_unlock(&console_mutex);
                    k++;
                }
                if (k >= 10) {
                    break;
                }
                sleep(0.1);
            }
        }
    }

    void finish() {
        for (size_t i=0,e=threads.size(); i<e; ++i) {
            workQueue.addTask(NULL);
        }
        for (size_t i=0,e=threads.size(); i<e; ++i) {
            threads[i]->join();
            delete threads[i];
        }
        threads.clear();
    }

  private:
    std::vector<PoolWorkerThread*> threads;
    WorkQueue workQueue;
    int total;
    uint64_t start_time;
    pthread_mutex_t console_mutex;
};

