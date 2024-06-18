#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0

static struct thread* current_thread = NULL;
static int id = 1;
static jmp_buf env_main;
//static jmp_buf env_st;

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    t->previous=NULL;
    t->next=NULL;
    t->current_task=NULL;
    t->task_count=0;
    t->current_task_id=-1;
    id++;
    return t;
}
void thread_add_runqueue(struct thread *t){
    if(current_thread == NULL){
        // TODO
        current_thread = t;
        current_thread->previous = current_thread;
        current_thread->next = current_thread;
    }
    else{
        // TODO
        t->next=current_thread;
        t->previous=current_thread->previous;
        current_thread->previous->next=t;
        current_thread->previous=t;
    }
}
void thread_yield(void){
    // TODO
  if(setjmp(current_thread->env)==0){
      current_thread->buf_set=1; 
      schedule();
      dispatch();
  }else{
      while(current_thread->current_task!=NULL && current_thread->current_task->id > current_thread->current_task_id){
          int tmp=current_thread->current_task_id;
          struct task* now = current_thread->current_task;
          current_thread->current_task=current_thread->current_task->previous;
          current_thread->current_task_id=now->id;
          (now->fp)(now->arg);
          current_thread->current_task_id=tmp;

          // remove task
          free(now);
      }
  }
}
void dispatch(void){
    // TODO
    // first time running a thread => assign stack pointer stored in jmp_buf "the address of the thread's stack"
    if(current_thread->buf_set==0){ 
        if(setjmp(current_thread->env)==0){
            (current_thread->env)[0].sp=(unsigned long)(current_thread->stack_p);
            longjmp(current_thread->env,1);    
        }
        while(current_thread->current_task!=NULL && current_thread->current_task->id > current_thread->current_task_id){
            int tmp=current_thread->current_task_id;
            struct task* now = current_thread->current_task;
            current_thread->current_task=current_thread->current_task->previous;
            current_thread->current_task_id=now->id;
            (now->fp)(now->arg);
            current_thread->current_task_id=tmp;

            // remove task
            free(now);
        }
        (current_thread->fp)(current_thread->arg);
        thread_exit();    
    }else{
        // back to yield
        longjmp(current_thread->env,1);
    }
}
void schedule(void){
    // TODO
    current_thread=current_thread->next;
}
void thread_exit(void){
    struct thread *next=current_thread->next, *previous=current_thread->previous;
    
    struct task* p;
    while(current_thread->current_task!=NULL){
        p=current_thread->current_task->previous;
        free(current_thread->current_task);
        current_thread->current_task=p;
    }

    free(current_thread->stack);
    free(current_thread);

    if(next != current_thread){
        // TODO
        current_thread=next;
        current_thread->previous=previous;
        previous->next=current_thread;
        dispatch();
    }else{
        // TODO
        // Hint: No more thread to execute
        current_thread=NULL;
        longjmp(env_main,1);
    }
}
void thread_start_threading(void){
    // TODO
    while(current_thread!=NULL){
        if(setjmp(env_main)==0) dispatch();
    }
}

// part 2
void thread_assign_task(struct thread *t, void (*f)(void *), void *arg){
    // TODO
    struct task *tk = (struct task*) malloc(sizeof(struct task));
    tk->fp = f;
    tk->arg = arg;
    tk->id=t->task_count++;
    tk->previous = t->current_task;
    t->current_task = tk;
}
