#include <iostream>
#include <ctime>
#include <pthread.h>
#include <chrono>
#include <unistd.h>
using namespace std;
using namespace std::chrono;

uint64_t ssum = 0;
uint64_t tsum = 0;
uint32_t TMAX = 10;
const int MAX = 1000;
int matrix[MAX][MAX];

typedef struct margs_ {
    int srow;
    int erow;
    int sum;
} margs_t;

pthread_cond_t ccv;
pthread_cond_t wcv;
pthread_mutex_t wlock;
pthread_mutex_t cclock;
pthread_mutex_t mlock;
int gcount = 0;
uint32_t sum = 0;

void* process(void *args) {
    pthread_mutex_lock(&cclock);
    cout<<"calculating thread ID: "<<pthread_self()<<" "<<gcount<<"\n";
    gcount+=1;
    if (gcount == 11) {
        pthread_cond_signal(&ccv);
    }
    pthread_cond_wait(&wcv, &cclock);
    pthread_mutex_unlock(&cclock);
    margs_t *karg = (margs_t*)args;
    int start = karg->srow;
    int end = karg->erow;
    for (int i=start; i<end;i++) {
        for (int j=0; j< MAX; j++) {
            karg->sum += matrix[i][j];
            usleep(10);
        }
        //cout<<"round "<<i<<"\n";
    }
    pthread_mutex_lock(&cclock);
    tsum += karg->sum;
    pthread_mutex_unlock(&cclock);
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    cout<<"workcrew sum "<<tsum<<" "<<now<<"\n";
    return NULL;
}

void *processSingle(void *args) {
    pthread_mutex_lock(&cclock);
    cout<<"calculating matrix Thread ID: "<<pthread_self()<<" "<<gcount<<"\n";
    gcount+=1;
    if (gcount == 11) {
        pthread_cond_signal(&ccv);
    }
    pthread_cond_wait(&wcv, &cclock);
    pthread_mutex_unlock(&cclock);
    margs_t *karg = (margs_t*)args;
    int start = karg->srow;
    int end = karg->erow;
    for (int i=start; i<end;i++) {
        for (int j=0; j< MAX; j++) {
            //pthread_mutex_lock(&mlock);
            karg->sum += matrix[i][j];
            usleep(10);
            //pthread_mutex_unlock(&mlock);
        }
    }
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    cout<<"main sum "<<karg->sum<<" "<<now<<"\n";
    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t threads[TMAX];
    pthread_t maint;
    int start = 0;
    void *status = NULL;

    pthread_cond_init(&ccv, NULL);
    pthread_cond_init(&wcv, NULL);
    pthread_mutex_init(&wlock, NULL);
    pthread_mutex_init(&mlock, NULL);
    pthread_mutex_init(&cclock, NULL);

    for (int i=0; i<MAX;i++) {
        for (int j=0; j< MAX; j++) {
            matrix[i][j] = rand() % MAX;
        }
    }
    margs_t *ss = (margs_t*)calloc(1, sizeof(margs_t));
    ss->srow = 0;
    ss->erow = MAX;
    pthread_create(&maint, NULL, processSingle, ss);

    for (int i=0;i<10;i++) {
        margs_t *ss = (margs_t*)calloc(1, sizeof(margs_t));
        ss->srow = start;
        ss->erow = start+100;
        start += 100;
        pthread_create(&threads[i], NULL, process, ss);
    }

    while (gcount!=11) {
        pthread_cond_wait(&ccv, &cclock);
    }

    cout<<"broadcast "<<gcount<<"\n";
    pthread_mutex_unlock(&cclock);
    pthread_cond_broadcast(&wcv);
    
    for (int i=0;i<10;i++) {
        pthread_join(threads[i], &status);
    }
    pthread_join(maint, &status);
    return 0;
}
