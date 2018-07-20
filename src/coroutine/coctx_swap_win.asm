.model FLAT, C

.code

coctx_swap PROC
	lea		eax,		[esp] + 4
	mov		esp,		[esp + 4]
	lea		esp,		[esp] + 32
	push	eax
	push	[eax - 4]
	push	ebp
	push	esi
	push	edi
    push    edx
    push    ecx
	push	ebx

	mov		esp,		[eax + 4]
	pop		ebx 
    pop     ecx
    pop     edx
	pop		edi
	pop		esi
	pop		ebp
	pop		eax  
	pop		esp
	push	eax 

	ret
coctx_swap ENDP

end
