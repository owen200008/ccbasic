.model FLAT, C

.code

coctx_swap PROC
	mov		eax,		[esp + 4]
	mov		ecx,		[esp + 0]
	mov		[eax + 4],	ecx
	lea		ecx,		[esp] + 4
	mov		[eax + 0],	ecx
	mov		[eax + 8],	ebp
	mov		[eax + 12],	ebx
	mov		[eax + 16],	esi
	mov		[eax + 20],	edi

	mov		eax, [esp + 8]
	mov		ecx, [eax + 4]
	mov		esp, [eax + 0]
	push	ecx

	mov		ebp,		[eax + 8]
	mov		ebx,		[eax + 12]
	mov		esi,		[eax + 16]
	mov		edi,		[eax + 20]

	ret

coctx_swap ENDP

end
