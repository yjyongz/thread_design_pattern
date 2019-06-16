#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include "thread_pool.h"
using namespace std;

void *taskExecution(void *str) {
    char *args = (char*)str;
    cout<<"Task is executing "<<args<<"\n";
    return NULL;
}

void *assignTask(void *p) {
    assert(p);
    Pool *pool = (Pool*)p;
    while (!(pool->m_flags & POOL_SHUTDOWN)) {
        char *words = (char*)calloc(10, sizeof(char));
        cout<<"please input task name and its argument\n";
        cin>>words;
        if (strncmp(words, "END", 3) == 0) {
            delete pool;
        } else {
            Task *t = new Task(taskExecution, (void*)words);
            pool->addTask(t);
        }
    }
    return NULL;
}
extern pthread_cond_t   g_cond;
extern pthread_mutex_t  g_lock;
int main () {
    Pool *pool = new Pool(5);
    pthread_t t1;
    pthread_create(&t1, NULL, assignTask, (void*) pool);
    pthread_cond_init(&g_cond, NULL);
    pthread_mutex_init(&g_lock, NULL);

    pthread_cond_wait(&g_cond, &g_lock);
    cout<<"Exiting \n";
    return 0;
}
