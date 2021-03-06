/*
 * RUN: clang %cflags -emit-llvm -S %s -o %t.ll
 * RUN: soaap --soaap-sandbox-platform=annotated -o %t.soaap.ll %t.ll > %t.out
 * RUN: FileCheck %s -input-file %t.out
 *
 * CHECK: Running Soaap Pass
 */
#include "soaap.h"
#include <fcntl.h>
#include <unistd.h>

void foo();

int main(int argc, char** argv) {
  foo();
  return 0;
}

__soaap_sandbox_persistent("sandbox")
void foo() {
  // CHECK: *** Sandbox "sandbox" performs system call "open" but it is not allowed to,
  // CHECK-NEXT: *** based on the current sandboxing restrictions
  // CHECK-NEXT: +++ Line 24 of file {{.*}}
  int fd = open("somefile", O_CREAT);
  __soaap_limit_syscalls(read, write);
  __soaap_limit_fd_syscalls(fd, read);
  write(fd, NULL, 0);
  // CHECK: *** Sandbox "sandbox" performs system call "close" but it is not allowed to,
  // CHECK-NEXT: *** based on the current sandboxing restrictions.
  // CHECK-NEXT: +++ Line 31 of file {{.*}}
  close(fd);
  
  int fds[2];
  // CHECK: *** Sandbox "sandbox" performs system call "open" but it is not allowed to,
  // CHECK-NEXT: *** based on the current sandboxing restrictions.
  // CHECK-NEXT: +++ Line 37 of file {{.*}}
  fds[0] = open("somefile", O_CREAT);
  __soaap_limit_fd_syscalls(fds[0], read);
  read(fds[0], NULL, 0);
  write(fds[0], NULL, 0);
  // CHECK: *** Sandbox "sandbox" performs system call "close" but it is not allowed to,
  // CHECK-NEXT: *** based on the current sandboxing restrictions.
  // CHECK-NEXT: +++ Line 44 of file {{.*}}
  close(fds[0]);
  // CHECK-NOT: *** Sandbox "sandbox" performs system call "read" but is not allowed to for the given fd arg.
  
  // CHECK: *** Sandbox "sandbox" performs system call "write" but is not allowed to for the given fd arg.
  // CHECK-NEXT: +++ Line 27 of file {{.*}}
  
  // CHECK: *** Sandbox "sandbox" performs system call "write" but is not allowed to for the given fd arg.
  // CHECK-NEXT: +++ Line 40 of file {{.*}}
}
