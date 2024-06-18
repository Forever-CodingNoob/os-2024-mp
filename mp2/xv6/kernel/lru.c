#include "lru.h"

#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

void lru_init(lru_t *lru){
	lru->size=0;
}

void lru_push(lru_t *lru, uint64 e){
	// check if e exists in lru->bucket first
	int idx;
	if((idx=lru_find(lru, e))>=0) lru_pop(lru, idx);
	else if(lru->size==PG_BUF_SIZE){
		int i=lru->size-1;
		while(i>=0 && *((pte_t*)(lru->bucket[i])) & PTE_P) i--;
		if(i<0) return; //all pages are pinned
		lru_pop(lru, i);
	}
		
	// now lru->size <= PG_BUF_SIZE-1
	for(int i=lru->size-1; i>=0; i--)
		lru->bucket[i+1]=lru->bucket[i];
	
	lru->bucket[0]=e;
	lru->size++;
}

void lru_pop(lru_t *lru, int idx){
	if(idx<0 || idx>=lru->size) return;
	//uint64 tmp=lru->bucket[idx];
	//pte_t *pte=(pte_t*)tmp;
	for(int i=idx; i<lru->size-1; i++)
		lru->bucket[i]=lru->bucket[i+1];
	lru->size--;
}

int lru_empty(lru_t *lru){
	return lru->size==0;
}

int lru_full(lru_t *lru){
	return lru->size==PG_BUF_SIZE;
}

void lru_clear(lru_t *lru){
	lru->size=0;
}

int lru_find(lru_t *lru, uint64 e){
	for(int i=0;i<lru->size;i++){
		if(lru->bucket[i]==e) return i;
	}
	return -1;
}
