#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    thread_func_args->thread_complete_success = true;
    // usleep is microseconds
    usleep(thread_func_args->wait_to_obtain_ms * 1000);    
    // This block of test code will lead to test fail, no idea why 
    /*if(pthread_mutex_trylock(thread_func_args->pt_shared_mutex) != 0)
        printf("It's locked\n");*/
    if(pthread_mutex_lock((thread_func_args->pt_shared_mutex)) != 0)
        ERROR_LOG("Error while locking the mutex");

    usleep(thread_func_args->wait_to_release_ms * 1000);

    if(pthread_mutex_unlock((thread_func_args->pt_shared_mutex)) != 0)
        ERROR_LOG("Error while releasing the mutex");

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    // allocate memory for thread_data
    struct thread_data *thread_param = (struct thread_data *)malloc(sizeof(struct thread_data));
    
    thread_param->thread_complete_success = false;
    thread_param->pt_shared_mutex = mutex;
    thread_param->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_param->wait_to_release_ms = wait_to_release_ms;

    int rc = pthread_create(thread, NULL, threadfunc, (void*)thread_param);
    if(!rc){
        thread_param->thread_complete_success = true;
        //return true;  // this will lead to dangling memory
    }
    else{
        // if thread did not create, clean allocated memory    
        free(thread_param);
    }
    // join the thread, but it's unnecessary for this test
    /*if(pthread_join(*thread, thread_param) != 0){
        printf("joining\n");
        ERROR_LOG("Error while joining thread");
        ((struct thread_data *)thread_param)->thread_complete_success = false;
    }*/
    // return true if the thread could be started
    if(!rc)
        return true;
    else
        return false;
    //retrurn rc == 0
}

