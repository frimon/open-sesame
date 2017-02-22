.data
	.align 2

.global enable_interrupt

.text

enable_interrupt:
  	ei
  	jr $ra
