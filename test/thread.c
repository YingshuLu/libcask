#include <pthread.h>
#include "co_define.h"
#include "co_mutex.h"
#include "atomic.h"
#include "co_cond.h"
#include "co_bar.h"
#include "co_await.h"

#define THREAD_MAX 6
#define CO_MAX 10000
co_mutex_t g_mutex;
co_bar_t g_bar;
atomic_t am;
int g_count = 0;


int add(void *ip) {
    int i;
    for(i = 0; i < 1000000; i++) {}
    return i;

}

void task(void *ip, void *op) {
    
    sleep(10000); 
    INF_LOG("task cond wait");
    co_bar_wait(&g_bar);
    INF_LOG("task cond start");

    while(1) {
        atomic_inc(&am);
        INF_LOG("thread task: %d, atomic: %d", getcid(), atomic_get(&am));
        co_mutex_lock(&g_mutex);
        if(g_count >= 10) goto end;
        g_count++;
        INF_LOG("[count] g_count : %d", g_count);
        co_mutex_unlock(&g_mutex);
        co_yield();
    }
end:
    co_mutex_unlock(&g_mutex);
}

void wait_task(void *ip, void *op) {
    
    int ret = co_await(add, NULL);

    INF_LOG("co wait return: %d", ret);
}

void *routine(void *param) {
    INF_LOG("pthread %d start", tid());
   
    int i = 0;
    for(; i<CO_MAX; i++) {
       co_create(task, NULL, NULL);
    }
    co_create(wait_task, NULL, NULL);
    schedule();
}

int main() {
    
    co_mutex_init(&g_mutex);
    co_bar_init(&g_bar, 6);
    pthread_t pid[THREAD_MAX];
    int i = 0;
    for(; i < THREAD_MAX; i++) {
        pthread_create(&(pid[i]), NULL, routine, NULL);
    }

    for(i = 0; i < THREAD_MAX; i++) {
        pthread_join(pid[i], NULL);
        INF_LOG("main thread waited  thread %d", pid[i]);
    }
    return 0;
}
