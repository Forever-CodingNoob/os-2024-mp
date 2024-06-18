#include "kernel/types.h"
#include "user/user.h"
#include "user/list.h"
#include "user/threads.h"
#include "user/threads_sched.h"

#define NULL 0
#define MIN(_a, _b)\
({\
	typeof(_a) __a = (_a);\
	typeof(_b) __b = (_b);\
	__a <= __b ? __a : __b;\
})
#define MAX(_a, _b)\
({\
	typeof(_a) __a = (_a);\
	typeof(_b) __b = (_b);\
	__a >= __b ? __a : __b;\
})

int get_least_release_time(struct list_head *release_queue){
    int closest_release_time=-1;
    struct release_queue_entry *rqe = NULL;
    list_for_each_entry(rqe, release_queue, thread_list){
        if (closest_release_time==-1 || rqe->release_time < closest_release_time)
            closest_release_time = rqe->release_time;
    }    
    return closest_release_time;
}



/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args)
{
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID)
            thread_with_smallest_id = th;
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

/* MP3 Part 1 - Non-Real-Time Scheduling */
/* Weighted-Round-Robin Scheduling */
struct threads_sched_result schedule_wrr(struct threads_sched_args args)
{
    struct threads_sched_result r;
    // TODO: implement the weighted round-robin scheduling algorithm

    if (list_empty(args.run_queue)){
        int least_release_time = get_least_release_time(args.release_queue);
        r.scheduled_thread_list_member = args.run_queue;   
        r.allocated_time=(least_release_time!=-1)?least_release_time-args.current_time:1;
    }else{
        struct thread *th = list_entry(args.run_queue->next, struct thread, thread_list);
        r.scheduled_thread_list_member = &th->thread_list;
        int max_accocated_time = args.time_quantum * th->weight;
        r.allocated_time = MIN(th->remaining_time, max_accocated_time);
    }

    return r;
}

/* Shortest-Job-First Scheduling */
struct threads_sched_result schedule_sjf(struct threads_sched_args args)
{
    struct threads_sched_result r;
    // TODO: implement the shortest-job-first scheduling algorithm

    struct thread *thread_with_shortest_remaining_time = NULL;
    int min_remain_time = -1;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_shortest_remaining_time == NULL 
                || th->remaining_time < min_remain_time 
                || (th->remaining_time == min_remain_time && th->ID < thread_with_shortest_remaining_time->ID)
            ){
            thread_with_shortest_remaining_time = th;
            min_remain_time = th->remaining_time;
        }
    }

    if (thread_with_shortest_remaining_time == NULL) {
        int least_release_time = get_least_release_time(args.release_queue);
        r.scheduled_thread_list_member = args.run_queue;   
        r.allocated_time=(least_release_time!=-1)?least_release_time-args.current_time:1;
    }else{
        // find the time that the next thread with processing time < min_remain_time-(release_time-current_time) (or equal but having smaller ID) is appended into the run queue 
        int closest_release_time=-1;
        struct release_queue_entry *rqe = NULL;
        list_for_each_entry(rqe, args.release_queue, thread_list){
            if (!(rqe->thrd->processing_time < min_remain_time-(rqe->release_time-args.current_time) 
                    || (rqe->thrd->processing_time == min_remain_time-(rqe->release_time-args.current_time && rqe->thrd->ID < thread_with_shortest_remaining_time->ID)))
                )
                continue;
            if (closest_release_time==-1 || rqe->release_time < closest_release_time)
                closest_release_time = rqe->release_time;
        }

        r.scheduled_thread_list_member = &thread_with_shortest_remaining_time->thread_list;
        r.allocated_time = (closest_release_time==-1)?min_remain_time:MIN(min_remain_time, closest_release_time-args.current_time);
    } 

    return r;
}







/* MP3 Part 2 - Real-Time Scheduling*/
/* Least-Slack-Time Scheduling */
struct threads_sched_result schedule_lst(struct threads_sched_args args)
{
    struct threads_sched_result r;
    // TODO: implement the least-slack-time scheduling algorithm
    
    struct thread *thread_missed_deadline = NULL;
    struct thread *th = NULL;

    // find the thread with smallest ID that has already misses its current deadline
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (args.current_time >= th->current_deadline && (thread_missed_deadline == NULL || th->ID < thread_missed_deadline->ID))
            thread_missed_deadline = th;
    }
    if(thread_missed_deadline!=NULL){
        r.scheduled_thread_list_member = &thread_missed_deadline->thread_list;
        r.allocated_time = 0;
        return r;
    }

    struct thread *thread_with_lst = NULL;
    int lst = -1;
    list_for_each_entry(th, args.run_queue, thread_list) {
        int st=th->current_deadline - args.current_time - th->remaining_time;
        if (thread_with_lst == NULL 
                || st < lst 
                || (st == lst && th->ID < thread_with_lst->ID)
            ){
            thread_with_lst = th;
            lst = st;
        }
    }

    if(thread_with_lst==NULL){
        int least_release_time = get_least_release_time(args.release_queue);
        r.scheduled_thread_list_member = args.run_queue;   
        r.allocated_time=(least_release_time!=-1)?least_release_time-args.current_time:1;
    }else{
        int allocated_time = MIN(thread_with_lst->remaining_time, thread_with_lst->current_deadline-args.current_time);
        int closest_release_time=-1;
        struct release_queue_entry *rqe = NULL;
        list_for_each_entry(rqe, args.release_queue, thread_list){
            int st=rqe->thrd->period - rqe->thrd->processing_time;
            if (!(st < lst 
                    || (st==lst && rqe->thrd->ID < thread_with_lst->ID)
                ))
                continue;
            if (closest_release_time==-1 || rqe->release_time < closest_release_time)
                closest_release_time = rqe->release_time;
        }

        r.scheduled_thread_list_member = &thread_with_lst->thread_list;
        r.allocated_time = (closest_release_time==-1)?allocated_time:MIN(allocated_time, closest_release_time-args.current_time);
    }
    return r;
}




/* Deadline-Monotonic Scheduling */
struct threads_sched_result schedule_dm(struct threads_sched_args args)
{
    struct threads_sched_result r;
    // TODO: implement the deadline-monotonic scheduling algorithm

    struct thread *thread_missed_deadline = NULL;
    struct thread *th = NULL;

    // find the thread with smallest ID that has already misses its current deadline
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (args.current_time >= th->current_deadline && (thread_missed_deadline == NULL || th->ID < thread_missed_deadline->ID))
            thread_missed_deadline = th;
    }
    if(thread_missed_deadline!=NULL){
        r.scheduled_thread_list_member = &thread_missed_deadline->thread_list;
        r.allocated_time = 0;
        return r;
    }

    struct thread *thread_with_shortest_deadline = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_shortest_deadline == NULL 
                || th->deadline < thread_with_shortest_deadline->deadline 
                || (th->deadline == thread_with_shortest_deadline->deadline && th->ID < thread_with_shortest_deadline->ID)
            )
            thread_with_shortest_deadline = th;
    }

    if(thread_with_shortest_deadline==NULL){
        int least_release_time = get_least_release_time(args.release_queue);
        r.scheduled_thread_list_member = args.run_queue;   
        r.allocated_time=(least_release_time!=-1)?least_release_time-args.current_time:1;
    }else{
        int allocated_time = MIN(thread_with_shortest_deadline->remaining_time, thread_with_shortest_deadline->current_deadline-args.current_time);
        int closest_release_time=-1;
        struct release_queue_entry *rqe = NULL;
        list_for_each_entry(rqe, args.release_queue, thread_list){
            if (!(rqe->thrd->deadline < thread_with_shortest_deadline->deadline 
                    || (rqe->thrd->deadline == thread_with_shortest_deadline->deadline  &&  rqe->thrd->ID < thread_with_shortest_deadline->ID)
                ))
                continue;
            if (closest_release_time==-1 || rqe->release_time < closest_release_time)
                closest_release_time = rqe->release_time;
        }

        r.scheduled_thread_list_member = &thread_with_shortest_deadline->thread_list;
        r.allocated_time = (closest_release_time==-1)?allocated_time:MIN(allocated_time, closest_release_time-args.current_time);
    }
    
    return r;
}
