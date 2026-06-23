#include "multithreading.h"
#include <thread>
#include <condition_variable>

void threadLoop(uint n, uint i0);
const uint numThreads = std::thread::hardware_concurrency();
std::vector<std::thread> threads(numThreads);
std::vector<Camera*> jobReady(numThreads, nullptr);
bool jobSecondPass = false;
std::vector<std::condition_variable> cvs(numThreads);
std::mutex mtx;
bool shutdown = false;
bool threadsInit = false;
std::condition_variable cv_done;
int jobs_remaining = 0;          // protected by mtx

void startThreads(Camera* camera, bool secondPass) {
    if (!threadsInit) {
        for (uint i = 0; i < numThreads; ++i)
            threads[i] = std::thread(threadLoop, numThreads, i);
        threadsInit = true;
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        jobs_remaining = numThreads;
        for (uint i = 0; i < numThreads; ++i) jobReady[i] = camera;
        jobSecondPass = secondPass;
    }
    for (auto& cv : cvs) 
        cv.notify_one();

    // Wait until all workers report done
    std::unique_lock<std::mutex> lock(mtx);
    cv_done.wait(lock, []{ return jobs_remaining == 0; });
}

void threadLoop(uint n, uint i)
{
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait for a job or shutdown
        cvs[i].wait(lock, [&]{ return jobReady[i] || shutdown; });

        if (shutdown) break;

        // We have a job – release the mutex so other threads can run
        Camera* cam = jobReady[i];   // copy it while still holding lock
        lock.unlock();

        if (jobSecondPass)
            fogPass(n, i, cam);
        else
            deferredPass(n, i, cam);

        // Re‑acquire the mutex to mark our slot as free and update counter
        std::lock_guard<std::mutex> lk(mtx);
        jobReady[i] = nullptr;
        if (--jobs_remaining == 0)
            cv_done.notify_one();
    }
}

void shutdownThreads() {
    if(!threadsInit)
        return;
    {
        std::lock_guard<std::mutex> lock(mtx);
        shutdown = true;
    }
    for(auto &cv : cvs)
        cv.notify_all();
    for(auto &t : threads)
        t.join();
}