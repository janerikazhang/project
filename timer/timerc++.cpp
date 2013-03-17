#include <algorithm>
#include <functional>
#include <vector>

#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

class scheduler {
public:
    scheduler();
    int events();
    void addEvent(const struct timeval, int (*)(void *), void *);
    int dispatchUntil(const struct timeval &);
    bool waitUntil(const struct timeval * = NULL);
    int loopUntil(const struct timeval * = NULL);

private:
    static bool tv_le(const struct timeval &, const struct timeval &);
    struct event {
        struct timeval when;
        int (*callback)(void *);
        void *data;
    };
    static struct _cmp
      : public binary_function<bool, const struct event &, const struct event &>
    {
        bool operator()(const struct event &a, const struct event &b) {
            return !tv_le(a.when, b.when);
        }
    } cmp;
    vector<struct event> heap;
};

bool scheduler::tv_le(const struct timeval &a, const struct timeval &b) {
    return a.tv_sec < b.tv_sec ||
        a.tv_sec == b.tv_sec && a.tv_usec <= b.tv_usec;
}

scheduler::scheduler() : heap() {}

int scheduler::events() {
    return heap.size();
}

void scheduler::addEvent(const struct timeval when, int (*callback)(void *), void *data) {
    struct event ev = {when, callback, data};
    heap.push_back(ev);
    push_heap(heap.begin(), heap.end(), cmp);
}

int scheduler::dispatchUntil(const struct timeval &tv) {
    int count = 0;
    while (heap.size() > 0 && tv_le(heap.front().when, tv)) {
        struct event ev = heap.front();
        pop_heap(heap.begin(), heap.end(), cmp);
        heap.pop_back();
        ev.callback(ev.data);
        count++;
    }
    return count;
}

bool scheduler::waitUntil(const struct timeval *tv) {
    if (heap.size() > 0 && (!tv || tv_le(heap.front().when, *tv)))
        tv = &heap.front().when;
    if (!tv)
        return false;
    struct timeval tv2;
    do {
        gettimeofday(&tv2, NULL);
        if (tv_le(*tv, tv2))
            break;
        tv2.tv_sec -= tv->tv_sec;
        if ((tv2.tv_usec -= tv->tv_usec) < 0) {
            tv2.tv_sec--;
            tv2.tv_usec += 1000000;
        }
    } while (select(0, NULL, NULL, NULL, &tv2) < 0 && errno == EINTR);
    return heap.size() > 0 && tv_le(*tv, heap.front().when);
}

int scheduler::loopUntil(const struct timeval *tv) {
    int counter = 0;
    while (waitUntil(tv))
        counter += dispatchUntil(heap.front().when);
    return counter;
}
