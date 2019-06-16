#include <pthread.h>
#include <list>
#include <string>
using namespace std;
pthread_cond_t g_cond;
pthread_mutex_t g_lock;
class Task {
    public:
        Task(void *(*routine)(void *), void* arg):m_routine(routine), m_args(arg) {
        }
        Task():m_routine(NULL), m_args(NULL) {
        }
        void* (*m_routine)(void *args);
        void *m_args;
};

class Pool {
#define QUEUE_CLOSED 1
#define POOL_SHUTDOWN QUEUE_CLOSED << 1
    public:
        Pool(uint32_t max=5, uint32_t max_q = 100):m_max_qsize(max_q), m_max_workers(max), m_threads(NULL), m_flags(0)
        {
            init();
        }
        ~Pool();
        uint32_t addTask(Task *task);
        pthread_mutex_t m_queue_lock;
        pthread_cond_t  m_q_full_cv;
        pthread_cond_t  m_q_empty_cv;
        pthread_cond_t  m_q_exit_cv;
        void init();
        pthread_t *m_threads;
        list<Task*> m_lists;
        uint32_t m_flags;
        uint32_t m_max_workers;
        uint32_t m_max_qsize; //task queue size
};

Pool::~Pool() {
    pthread_mutex_lock(&m_queue_lock);
    if (m_flags & POOL_SHUTDOWN) {
        cout<<"Already in shutdown state\n";
    }
    m_flags |= POOL_SHUTDOWN;
    cout<<"draining queue "<<m_lists.size()<<"\n";
    pthread_mutex_unlock(&m_queue_lock);
    pthread_cond_broadcast(&m_q_empty_cv);
    while (m_lists.size() != 0)  {
        pthread_cond_wait(&m_q_exit_cv, &m_queue_lock);
    }

    for (int idx= 0; idx < m_max_workers;idx++) {
        void *status = 0;
        pthread_join(m_threads[idx], &status);
    }
    pthread_mutex_unlock(&m_queue_lock);
    cout<<"Pool Exit\n";
    pthread_cond_signal(&g_cond);
}

uint32_t Pool::addTask(Task *task) {
    assert(task);
    pthread_mutex_lock(&m_queue_lock);
    if (m_flags & POOL_SHUTDOWN) {
        free(task);
        return 0;
    }
    while (m_lists.size() == m_max_qsize) {
        cout<<"queue is full "<<"\n";
        pthread_cond_wait(&m_q_full_cv, &m_queue_lock);
    }
    if (m_lists.size() == 0) {
        m_lists.push_back(task);
        pthread_cond_broadcast(&m_q_empty_cv);
    }
    pthread_mutex_unlock(&m_queue_lock);
    return 0;
}

void *worker_thread(void *pl) {
    assert(pl != NULL);
    Pool *pool = (Pool *)pl;
    while (true) {
        pthread_mutex_lock(&(pool->m_queue_lock));
        while (pool->m_lists.size() == 0 && !(pool->m_flags & POOL_SHUTDOWN)) {
            cout<<"queue size is zero waiting "<<pool->m_lists.size()<<"\n";
            pthread_cond_wait(&(pool->m_q_empty_cv), &(pool->m_queue_lock));
        }
        //cout<<pool->m_flags<<" "<<pool->m_lists.size()<<"\n";
        if ((pool->m_flags & POOL_SHUTDOWN) && pool->m_lists.size() == 0) {
            pthread_mutex_unlock(&(pool->m_queue_lock));
            break;
        }
        Task *task = pool->m_lists.front();
        pool->m_lists.pop_front();
        cout<<"Getting task from queue head remaining "<<pool->m_lists.size()<<"\n";
        pthread_mutex_unlock(&(pool->m_queue_lock));
        (*(task->m_routine))(task->m_args);
        free(task);
    }
    cout<<"Thread exiting \n";
    pthread_cond_broadcast(&(pool->m_q_exit_cv));
    pthread_exit(NULL);
    return NULL;
}

void Pool::init() {
    uint32_t retcode;

    m_threads = (pthread_t*)calloc(m_max_workers, sizeof(pthread_t));
    retcode = pthread_mutex_init(&m_queue_lock, NULL);
    if (retcode != 0) {
        cerr<<"pthread_mutext_init error: "<<strerror(retcode)<<"\n";
        exit(-1);
    }
    retcode = pthread_cond_init(&m_q_full_cv, NULL);
    if (retcode != 0) {
        cerr<<"pthread_cond_init m_q_full_cv error: "<<strerror(retcode)<<"\n";
        exit(-1);
    }
    retcode = pthread_cond_init(&m_q_empty_cv, NULL);
    if (retcode != 0) {
        cerr<<"pthread_cond_init m_q_empty_cv error: "<<strerror(retcode)<<"\n";
        exit(-1);
    }

    retcode = pthread_cond_init(&m_q_exit_cv, NULL);
    if (retcode != 0) {
        cerr<<"pthread_cond_init m_q_empty_cv error: "<<strerror(retcode)<<"\n";
        exit(-1);
    }
    for (uint32_t idx = 0; idx < m_max_workers; idx++) {
        retcode = pthread_create(&m_threads[idx], NULL, worker_thread, this);
        if (retcode != 0) {
            cerr<<"pthread_create "<<idx<<" error: "<<strerror(retcode)<<"\n";
            exit(-1);
        }
    }
}
