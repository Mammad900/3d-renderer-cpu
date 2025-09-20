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
bool init = false;

void startThreads(Camera *camera, bool secondPass) {
    if(!init) {
        for (uint i = 0; i < numThreads; i++)
            threads[i] = std::thread(threadLoop, numThreads, i);
        init = true;
    }
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (uint i = 0; i < numThreads; i++)
            jobReady[i] = camera;
        jobSecondPass = secondPass;
    }

    for (uint i = 0; i < numThreads; i++)
        cvs[i].notify_one();

    bool allDone = false;
    while(!allDone) {
        std::this_thread::yield();
        std::lock_guard<std::mutex> lock(mtx);
        allDone = std::all_of(jobReady.begin(), jobReady.end(), [](Camera *v) {return !v;});
    }
}

void threadLoop(uint n, uint i) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cvs[i].wait(lock, [&] { return jobReady[i] || shutdown; });
        if(shutdown) break;
        lock.unlock();
        if(jobSecondPass)
            fogPass(n, i, jobReady[i]);
        else
            deferredPass(n, i, jobReady[i]);
        lock.lock();
        jobReady[i] = nullptr;
    }
}

void shutdownThreads() {
    if(!init)
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