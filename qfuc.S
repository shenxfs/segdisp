#include <avr/io.h>
.text
.global Qequation

Qequation:
	eor 	r0,r0 
	mov     r26,r0
	add 	r24,r20
	adc 	r25,r21
	ror		r25
	ror		r24
	movw 	r18,r22
	movw 	r22,r0
	mul		r24,r18
	movw	r20,r0	
	mul		r25,r18
	add		r21,r0
	adc		r22,r1
	adc		r23,r26
	mul		r24,r19
	add		r21,r0
	adc 	r22,r1
	adc		r23,r26
	mul		r25,r19
	add		r22,r0
	adc		r23,r1
	eor		r1,r1
	lsl		r21
	rol		r22
	rol		r23
	brcs    ll0
	lsl		r21
	rol		r22
	rol		r23
	brcs    ll0
	movw	r24,r22
	tst		r21
	brbc	4,ll1
	adiw	r24,1
	rjmp	ll1
ll0:	
	eor 	r24,r24
	eor 	r25,r25	
ll1:
	ret		
	