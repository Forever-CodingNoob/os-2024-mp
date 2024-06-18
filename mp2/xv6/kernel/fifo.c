#include "fifo.h"

#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

void q_init(queue_t *q){
	q->size=0;
}

void q_push(queue_t *q, uint64 e){
	// check if e exists in q->bucket first
	if(q_find(q, e)>=0) return;
	else if(q->size==PG_BUF_SIZE){
		int i=0;
		while(i<q->size && *((pte_t*)(q->bucket[i])) & PTE_P) i++;
		if(i>=q->size) return; //all pages are pinned
		q_pop_idx(q, i);
	}
		
	// now q->size <= PG_BUF_SIZE-1
	q->bucket[q->size++]=e;
}

void q_pop_idx(queue_t *q, int idx){
	if(idx<0 || idx>=q->size) return;
	for(int i=idx; i<q->size-1; i++)
		q->bucket[i]=q->bucket[i+1];
	q->size--;
}

int q_empty(queue_t *q){
	return q->size==0;
}

int q_full(queue_t *q){
	return q->size==PG_BUF_SIZE;
}

void q_clear(queue_t *q){
	q->size=0;
}

int q_find(queue_t *q, uint64 e){
	for(int i=0;i<q->size;i++){
		if(q->bucket[i]==e) return i;
	}
	return -1;
}
