  # labwork.S
  # Written 2015 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1 2015-2016.
  # The macros PUSH and POP are in the public domain.

#.macro	PUSH reg
#	addi	$sp,$sp,-4
#	lw      $t7, 0\reg
#	sw	$t7,0($sp)
#.endm

#.macro	POP reg
#	lw	\reg,0($sp)
	#lw      $t7, 0($sp)
	#sw      $t7, 0\reg
#	addi	$sp,$sp,4
#.endm

.data
	.align 2

  # You can add your own code here
  #

.global time2string
.global delay

.text

.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm

delay:
	beq $a0, $0, delay_done
	nop
	addi $a0, $a0, -1
	li $t0, 14000 # Delay counter, 30 loops is about one millisecond
delay_loop:
	beq $t0, $0, delay
	nop
	addi $t0, $t0, -1
	j delay_loop
	nop

delay_done:
	jr $ra
	nop

time2string:
	## $a0: Minnesadress för dit strängen ska skrivas
	## $a1: Hexadecimala representationen av klockan som ska parsas

	PUSH $ra

	## First minute digit
	andi $t0, $a1, 0xf000
	PUSH $a0
	srl $a0, $t0, 12 # Shift 3 bytes to the right
	jal hexasc
	nop
	POP $a0
	sb $v0, 0($a0)

	## Second minute digit
	andi $t0, $a1, 0x0f00
	PUSH $a0
	srl $a0, $t0, 8 # Shift 2 bytes to the right
	jal hexasc
	nop
	POP $a0
	sb $v0, 1($a0)

	li $t1, 0x3a
	sb $t1, 2($a0)

	## First second digit
	andi $t0, $a1, 0x00f0
	PUSH $a0
	srl $a0, $t0, 4 # Shift 1 byte to the right
	jal hexasc
	nop
	POP $a0
	sb $v0, 3($a0)

	## Second second digit
	PUSH $a0
	andi $a0, $a1, 0x000f
	jal hexasc
	nop
	POP $a0
	sb $v0, 4($a0)

	li $t1, 0x0
	sb $t1, 5($a0)

	POP $ra
	jr $ra
	nop

  # you can write your code for subroutine "hexasc" below this line
hexasc: andi $t0, $a0, 0x0000000f # Ta med de fyra least significant bits
	addi $v0, $t0, 0x30 # Addera hexdec 30 eftersom ASCII 0 börjar från 30
	slti $t1, $v0, 0x3a # Om resultatet är mindre än hexdec 3a, dvs mindre än 10 så blir $t1 = 1
	bne $t1, $0, finish # Hoppa till finish om resultatet är mindre än 10, annars lägg på 7 för att komma till ASCII bokstäverna
	nop
	addi $v0, $v0, 7

	finish:
		jr $ra # Returnera, dvs gå tillbaka där vi var tidigare
		nop

