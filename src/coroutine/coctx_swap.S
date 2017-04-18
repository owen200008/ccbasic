/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

#define _esp 0
#define _eip 4
/* ------ */
#define _rsp 0
#define _rip 8
#define _rbx 16
#define _rdi 24
#define _rsi 32

.globl coctx_swap 
.type  coctx_swap, @function
coctx_swap:

#if defined(__i386__)
	movl 4(%esp), %eax
	movl (%esp), %ecx
	movl %ecx, _eip(%eax)
	leal 4(%esp), %ecx
	movl %ecx, _esp(%eax)
	movl %ebp, 8(%eax)
	movl %ebx, 12(%eax)
	movl %esi, 16(%eax)
	movl %edi, 20(%eax)

	movl 8(%esp), %eax
	movl _eip(%eax), %ecx
	movl _esp(%eax), %esp
	pushl %ecx

	movl 8(%eax), %ebp
	movl 12(%eax), %ebx
	movl 16(%eax), %esi
	movl 20(%eax), %edi

	ret

#elif defined(__x86_64__)
	movq 8(%rsp), %rax
	movq (%rsp), %rcx
	movq %rcx, _rip(%rax)
	leaq 8(%esp), %rcx
	movq %rcx, _rsp(%rax)
	movq %rbx, _rbx(%rax)
	movq %rdi, _rdi(%rax)
	movq %rsi, _rsi(%rax)

	movq %rbp, 40(%rax)
	movq %rdx, 48(%rax)
	movq %r8, 56(%rax)
	movq %r9, 64(%rax)
	movq %r12, 72(%rax)
	movq %r13, 80(%rax)
	movq %r14, 88(%rax)
	movq %r15, 96(%rax)

	/* sp */
	movq _rip(%rsi), %rcx
	movq _rsp(%rsi), %rsp
	pushq %rcx

	movq _rbx(%rsi),%rbx
	movq _rdi(%rsi),%rdi
	movq _rsi(%rsi),%rsi
	
	movq 40(%rax), %rbp
	movq 48(%rax), %rdx
	movq 56(%rax), %r8
	movq 64(%rax), %r9
	movq 72(%rax), %r12
	movq 80(%rax), %r13
	movq 88(%rax), %r14
	movq 96(%rax), %r15
	
	ret
#endif
