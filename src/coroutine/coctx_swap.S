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
	leal 4(%esp), %eax
	movl 4(%esp), %esp 
	leal 32(%esp), %esp
	pushl %eax
	pushl -4(%eax)
	pushl %ebp
	pushl %esi
	pushl %edi
	pushl %ebx

	movl 4(%eax), %esp
	popl %ebx  
	popl %edi
	popl %esi
	popl %ebp
	popl %eax  //ret func addr
	popl %esp
	pushl %eax //set ret func addr

	ret

#elif defined(__x86_64__)
	leaq 8(%rsp),%rax //default rsp 
	leaq 96(%rdi),%rsp //rsp point to param a
	pushq %rax
	pushq -8(%rax) //ret func addr rip
	pushq %rdi
	pushq %rbx
	pushq %rdx
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movq %rsi, %rsp // rsp point to param b
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r9
	popq %r8
	popq %rbp
	popq %rdx
	popq %rbx
	popq %rdi //read need
	popq %rax
	popq %rsp
	pushq %rax

	ret
#endif
