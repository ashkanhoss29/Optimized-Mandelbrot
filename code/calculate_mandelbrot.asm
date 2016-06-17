.data
realVal REAL8 +1024.0 ; (1 << 16)
two REAL8 +2.0

.code
PUBLIC mandelbrot_iteration
mandelbrot_iteration PROC
	;RCX    	YMM0	x_scaled_zero_x4 
	;RDX    	YMM1	y_scaled_zero_x4

	; TODO(Ashkan): Localize
	;R8     	YMM2	n_x4
	;R9     	YMM3	m_x4
	;RSP+28H	YMM4	iteration_x4
	;RSP+30H	YMM5	result_iteration_x4
	;RSP+38H	YMM6	negative_one_x4
	;RSP+40H	YMM7	one_x4
	;RSP+48H	YMM8	two_x4
	;RSP+50H	YMM9	s1024_x4

	VMOVUPD	YMM0, YMMWORD PTR [RCX]
	VMOVUPD	YMM1, YMMWORD PTR [RDX]
	VMOVUPD	YMM2, YMMWORD PTR [R8]
	VMOVUPD	YMM3, YMMWORD PTR [R9]

	MOV	RAX, QWORD PTR [RSP+28H]
	VMOVUPD	YMM4, YMMWORD PTR [RAX]
	MOV	RAX, QWORD PTR [RSP+30H]
	VMOVUPD	YMM5, YMMWORD PTR [RAX]
	MOV	RAX, QWORD PTR [RSP+38H]
	VMOVUPD	YMM6, YMMWORD PTR [RAX]
	MOV	RAX, QWORD PTR [RSP+40H]
	VMOVUPD	YMM7, YMMWORD PTR [RAX]
	MOV	RAX, QWORD PTR [RSP+48H]
	VMOVUPD	YMM8, YMMWORD PTR [RAX]
	MOV	RAX, QWORD PTR [RSP+50H]
	VMOVUPD	YMM9, YMMWORD PTR [RAX]

	XOR	RAX, RAX

	Label0:

	; iteration++
	VADDPD	YMM4, YMM4, YMM7
	; if(++iteration >= 500) finish
	INC		EAX ; ++iteration
	CMP		EAX, 500 ; iteration >= 500 ?
	JGE		Label1 ; jumps if the flag SF == 0

	; ntemp = n*n - m*m + x0
	VMULPD	YMM10, YMM2, YMM2
	VMULPD	YMM11, YMM3, YMM3
	VSUBPD	YMM10, YMM10, YMM11
	VADDPD	YMM10, YMM10, YMM0

	; m = 2*n*m + y0
	VMULPD	YMM11, YMM8, YMM2
	VMULPD	YMM3, YMM3, YMM11
	VADDPD	YMM3, YMM3, YMM1

	; n = tempn
	VMOVUPD	YMM2, YMM10

	; mask = n*n + m*m < 1024 && !(result_iteration == -1)
	VMULPD	YMM10, YMM2, YMM2
	VMULPD	YMM11, YMM3, YMM3
	VADDPD	YMM10, YMM10, YMM11
	VCMPLT_OQPD	YMM10, YMM10, YMM9
	VCMPEQPD	YMM11, YMM5, YMM6
	VANDNPD	YMM10, YMM10, YMM11

	; result_iteration = mask ? iteration : result_iteration
	VBLENDVPD	YMM5, YMM5, YMM4, YMM10

	; if (result_iteration == -1) loop; else finish;
	VCMPEQPD	YMM10, YMM5, YMM6 ; check == -1 for each value; we get 0 if false
	VXORPD	YMM11, YMM11, YMM11 ; get 0 since we need to check against 0 with vtestpd
	VTESTPD	YMM11, YMM10 ; sets CF flag to 1 if all four values are 0 (none are == -1)
	JB	Label1 ; jump if CF == 1

	JMP Label0

	Label1:

	; result_iteration = result_iteration == -1 ? iteration : result_iteration
	VCMPEQPD	YMM10, YMM5, YMM6
	VBLENDVPD	YMM0, YMM5, YMM4,YMM10

	RET
mandelbrot_iteration ENDP
END
