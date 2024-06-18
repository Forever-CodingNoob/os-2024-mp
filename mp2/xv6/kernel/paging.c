#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2024 */
/* Allocate eight consecutive disk blocks. */
/* Save the content of the physical page in the pte */
/* to the disk blocks and save the block-id into the */
/* pte. */
char *swap_page_from_pte(pte_t *pte) {
  char *pa = (char*) PTE2PA(*pte);
  uint dp = balloc_page(ROOTDEV);
  
  write_page_to_disk(ROOTDEV, pa, dp); // write this page to disk
  *pte = (BLOCKNO2PTE(dp) | PTE_FLAGS(*pte) | PTE_S) & ~PTE_V;

  return pa;
}

char *swap_page_to_pte(pte_t *pte){
  uint blockno= (uint) PTE2BLOCKNO(*pte);
  char* pa=(char*)kalloc();
  read_page_from_disk(ROOTDEV, pa, blockno);
  bfree_page(ROOTDEV, blockno);

  *pte = (PA2PTE(pa) | PTE_FLAGS(*pte) | PTE_V) & ~PTE_S;
  return pa;
}

/* NTU OS 2024 */
/* Page fault handler */
int handle_pgfault() {
  /* Find the address that caused the fault */
  /* uint64 va = r_stval(); */

  /* TODO */
  uint64 va = PGROUNDDOWN(r_stval());
  pagetable_t pgtbl = myproc()->pagetable;
  pte_t *pte = walk(pgtbl, va, 0);

  uint64 pa;
  if(pte!=0 && *pte & PTE_S){
    begin_op();
    pa=(uint64)swap_page_to_pte(pte);
    end_op();
    if(pa==0) return -1;
  }else if(pte==0 || *pte==0){
    pa = (uint64)kalloc();
    memset((void*)pa, 0, PGSIZE);
    mappages(pgtbl, va, PGSIZE, pa, PTE_U|PTE_R|PTE_W|PTE_X);
  }
  return 0;
}
