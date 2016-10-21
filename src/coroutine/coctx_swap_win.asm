.model FLAT, C

.code

coctx_swap PROC

	lea		ESP,		[esp] - 4
	push	ebx
	push	ecx
	push	edx

	push	edi
	push	esi
	push	ebp

	lea		ESP,		[esp] + 28

	mov		eax,		dword ptr [esp + 4]

	mov		ecx,		dword ptr [esp + 0]
	mov		dword ptr [eax + 4],	ecx

	lea		ecx,		[esp] + 4
	mov		dword ptr [eax + 0],	ecx

	mov		eax, dword ptr [esp + 8]

	mov		ecx, dword ptr [eax + 4]
	mov		esp, dword ptr [eax + 0]
	push	ecx

	lea		esp,		[esp] - 28
	pop		ebp
	pop		esi
	pop		edi

	pop		edx
	pop		ecx
	pop		ebx
	lea		esp,		[esp] + 4	

	ret

coctx_swap ENDP

end
