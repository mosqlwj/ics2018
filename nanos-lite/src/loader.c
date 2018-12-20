#include "proc.h"
#include <unistd.h>
#define DEFAULT_ENTRY 0x8048000

size_t get_ramdisk_size();
size_t ramdisk_write(const void *buf,size_t offset,size_t len);
size_t ramdisk_read(void *buf,size_t offset,size_t len);
extern uint8_t ramdisk_start;
extern int fs_open(const char *pathname,int flags,int mode);
extern ssize_t fs_read(int fd,void *buf,size_t len);
extern int fs_close(int fd);
extern int fs_filesz(int fd);
uintptr_t heapstart;
uintptr_t heapcur;
static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  Log("file %s",filename);
  int fd = fs_open(filename,0,0);
  int fz = fs_filesz(fd);
  void *va = (void *)DEFAULT_ENTRY;
  void *end = (void *)DEFAULT_ENTRY + fz;
  for(;va < end;va+=PGSIZE){
	  //Log("va: %x",(long)va);
	void *pa = new_page(1);
	Log("va: %x",(long)va);
	_map(&pcb->as,va,pa,1);
	fs_read(fd,pa,PGSIZE);
	//fs_read(fd,pa,(end-va)<PGSIZE?end-va:PGSIZE);
	//Log("pa1: %x",(uintptr_t)pa);
  }
  heapstart = (uintptr_t)va;
  heapcur = (uintptr_t)va;
  //current->cur_brk=current->max_brk = (uintptr_t)va;
  //Log("%x",current->cur_brk);
  fs_close(fd);
  return DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
	_protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);
//assert(0);
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);
  //Log("%x",(uint32_t)stack.start);
  //Log("%x",(uint32_t)stack.end);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
  if(pcb->cur_brk==0){
	pcb->cur_brk=pcb->max_brk = heapcur;
	Log("cur_brk: %x",pcb->cur_brk);
  }
}
