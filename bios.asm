org 0xf9c00000

section BIOS_START

define BOOTLOADER_ADDR 0x2800
define IVT_ADDR 0x1000
define KB_BUFF_ADDR 0x800
define TEXT_BUFF_ADDR 0x19000

define FB_ADDR 0xf9d10000

define FONT_WIDTH 8
define FONT_HEIGHT 16

define COLOR_WHITE	0xffffffff
define COLOR_GREEN 	0x00ff00ff
define COLOR_RED 	0xff0000ff
define COLOR_BLUE	0x0000ffff
define COLOR_BLACK	0x00000000

define IC_CMD_PORT 0x20
define IC_DATA_PORT 0x21

define DISP_WIDTH_PORT 0x23
define DISP_HEIGHT_PORT 0x24

; BIOS MEMORY MAP
; BIOS code: 0xf9c00000 => 0xf9c0ffff (64 KB)
; BIOS display cache: height 0x2f4, width 0x2f8 (4 bytes each)
; BIOS text cursor: Y 0x2fc, X 0x300 (4 bytes each)
; BIOS global stack: 0x400 => 0x7ff (1 KB)
; BIOS keyboard buffer: 0x800 => 0x9ff (512 Bytes)
; BIOS IVT setup: 0x1000 => 0x1400 (1 KB)
; Bootloader code: 0x2800 => 0x127ff (64 KB)
; BIOS text buffer: 0x19000 => 0x9311f (500 KB)

; Other memory regions
; MMIO space: 0xf9c10000 => 0xf9d0ffff (1 MB)
; framebuffer: 0xf9d10000 => 0xfa20ffff (5 MB)

section BIOS_CODE
	.bios_entry
		cli
		; global stack: 0x400 => 0x800 1K
		mov	bp, stack_base_addr
		mov	sp, bp
		add	sp, stack_len

		; Query the display once, before any framebuffer or text operations.
		; scrdim returns width in AX and height in BX (16-bit values).
		; Call the function directly because the IVT is not yet installed.
		call	scrdim

		; BIOS ROM stores pointers; the actual mutable values live in RAM.
		mov	ecx, dword[screen_width_ptr]
		mov	dword[ecx], ax
		mov	ecx, dword[screen_height_ptr]
		mov	dword[ecx], bx


		mov dword[0x300], 0
		mov dword[0x300 - 4], 0

		; String drawing uses EEX as the background color.
		mov	eex, COLOR_BLACK

		la	si, load_status
		mov	edx, COLOR_WHITE
		call	bios_println

		livt	IVT_ADDR
		mov	ecx, [interruptv_table_len]
		xor	eax, eax
		.bios_install_handlers
			mov	efx, [interruptv_table + eax * 4]
			mov	[IVT_ADDR + eax * 4], efx
			inc	eax
			loop	bios_install_handlers
		
		mov	edx, COLOR_GREEN
		la	si, load_finish
		call	bios_println

		la	si, bios_hello
		call	bios_println

		; clear all pending interrupts
		mov	al, IC_CMD_PORT
		outp	al, 3

		sli

		call	BOOTLOADER_ADDR

		; The bootloader may have changed EEX.
		mov	eex, COLOR_BLACK
		la	si, finish_txt
		mov	edx, COLOR_WHITE
		call	bios_println
		dbg
		ext


section BIOS_HELPERS

	;;;;;;;;;;;;;;;;   RETURN_POINT FUNCTION ;;;;;;;;;;;;;;;;;;;
	; a helper function to return from a callee function without defining a dedicated label for it.
	.return_point
		ret

	;;;;;;;;;;;;;;; PRINT_STR FUNCTION ;;;;;;;;;;;;;;;;;;;;;;;;;;
	; renders a string to the given x, y coordinates
	; Parameters (caller pushes in this order):
	;   push si   ; pointer to zero-terminated string
	;   push eax  ; x coordinate
	;   push ebx  ; y coordinate
	;   push edx  ; foreground color
	;   push eex  ; background color
	; Stack at entry (32-bit entries):
	;   [bp + 8]  = background color
	;   [bp + 12] = foreground color
	;   [bp + 16] = y coordinate
	;   [bp + 20] = x coordinate
	;   [bp + 24] = string pointer
	; Caller removes 5 * 4 bytes after return.
	.print_str
	    push bp
	    mov bp, sp
	    sub sp, 2 * 4

	    ; Preserve caller registers
	    push si
	    push eax
	    push ebx
	    push edx
	    push eex

	    ; Initialize local variables
	    mov eax, [bp + 24]
	    mov [bp - 4], eax

	    mov eax, [bp + 20]
	    mov [bp - 8], eax

	    .print_str_loop

		; Load current character
		mov si, [bp - 4]
		mov eax, byte[si]

		; Check for null terminator
		cmp eax, 0
		je print_str_end

		; Prepare drawchar arguments
		mov si, eax
		mov eax, [bp - 8]
		mov ebx, [bp + 16]
		mov edx, [bp + 12]
		mov eex, [bp + 8]

		; Draw current character
		push si
		push eax
		push ebx
		push edx
		push eex

		call drawchar
		add sp, 5 * 4

		; Advance string pointer
		inc dword[bp - 4]

		; Advance X coordinate by 8 pixels
		mov eax, [bp - 8]
		add eax, FONT_WIDTH
		mov [bp - 8], eax

		jmp print_str_loop

	    .print_str_end

		; Restore caller registers
		pop eex
		pop edx
		pop ebx
		pop eax
		pop si

		; Restore stack frame
		mov sp, bp
		pop bp
		ret

	;;;;;;;;;;;;;;; SETPIX FUNCTION ;;;;;;;;;;;;;;;;;;;;
	; sets a the pixel at x, y coordinates with the given color and the formula FB_ADDR + (y * width + x) * 4
	; parameters:
	; eax => x
	; ebx => y
	; edx => color
	.setpix
		push	bp
		mov	bp, sp

		; Preserve registers modified by the pixel calculations.
		push	eax
		push	ebx
		push	edx
		push	ecx
		push	efx

		; Load cached 32-bit screen dimensions from reserved RAM.
		mov	ecx, dword[screen_width_ptr]
		mov	ecx, dword[ecx]
		mov	efx, dword[screen_height_ptr]
		mov	efx, dword[efx]

		; Retrieve the pixel arguments.
		mov	edx, [bp + 2 * 4]
		mov	ebx, [bp + 3 * 4]
		mov	eax, [bp + 4 * 4]

		; Clip out-of-bounds pixels before addressing the framebuffer.
		cmp	eax, ecx
		jae	setpix_done
		cmp	ebx, efx
		jae	setpix_done

		; FB_ADDR + ((y * screen_width) + x) * 4
		mul	ebx, ecx
		add	ebx, eax
		mov	dword[FB_ADDR + ebx * 4], edx

		.setpix_done
		pop	efx
		pop	ecx
		pop	edx
		pop	ebx
		pop	eax

		mov	sp, bp
		pop	bp
		ret
	;;;;;;;;;;;;;;;;;; DRAWCHAR FUNCTION ;;;;;;;;;;;;;;;;;;;;;;;;;;
	; draws the pixels of a given character to the x,y coordinates using the given color
	; parameters:
	; si => char
	; eax => x
	; ebx => y
	; edx => color
	; eex => bg color
	.drawchar
		push	bp
		mov	bp, sp
		sub	sp, 10 * 4

		mov	eex, [bp + 2 * 4]
		mov	edx, [bp + 3 * 4]
		mov	ebx, [bp + 4 * 4]
		mov	eax, [bp + 5 * 4]
		mov	si, [bp + 6 * 4]
		
		; row loop counter
		mov	dword[bp - 1 * 4], 0
		.loop_row
			cmp	dword[bp - 1 * 4], FONT_HEIGHT ; My architecture allows it.
			je	enddraw

			push	eax
			; glyph map row value
			mov eax, si
			mov eax, dword[font_ascii_map + eax * 4]

			; Add row index to glyph address
			add eax, dword[bp - 1 * 4]

			; Load glyph row byte
			mov eax, byte[eax]
			mov dword[bp - 2 * 4], eax

			pop	eax


			; column loop counter
			mov	[bp - 3 * 4], 0
			.loop_column
				; pixel_value
				mov	[bp - 4 * 4], 0

				push	eax
				push	ebx

				mov	eax, 7
				mov	ebx, dword[bp - 2 * 4]
				sub	eax, dword[bp - 3 * 4]
				shr	ebx, eax
				and	ebx, 1
				mov	[bp - 4 * 4], ebx

				pop	ebx
				pop	eax

				cmp	[bp - 4 * 4], 1
				jne	bgpixel


				; temp x, y
				mov	[bp - 5 * 4], eax
				mov	[bp - 6 * 4], ebx

				add	eax, [bp - 3 * 4] ; x + c
				add	ebx, [bp - 1 * 4] ; y + r

				push	eax
				push	ebx
				push	edx
				call	setpix
				pop	edx
				add	sp, 8

				mov	eax, [bp - 5 * 4]
				mov	ebx, [bp - 6 * 4]
				jmp	next_col

				.bgpixel
					; temp x, y
					mov	[bp - 5 * 4], eax
					mov	[bp - 6 * 4], ebx

					add	eax, [bp - 3 * 4]
					add	ebx, [bp - 1 * 4]

					push	eax
					push	ebx
					push	eex
					call	setpix
					add	sp, 12

					mov	eax, [bp - 5 * 4]
					mov	ebx, [bp - 6 * 4]

				.next_col
					inc	[bp - 3 * 4]
					cmp	[bp - 3 * 4], FONT_WIDTH
					jl	loop_column
					inc	[bp - 1 * 4]
					jmp	loop_row

				.enddraw
					mov	sp, bp
					pop	bp
					ret


	
	;;;;;;;;;;;;;;;	IS_ALPHA FUNCTION ;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; returns a boolean to indicate if the given value is an alphabetic character
	; parameter list :
	; eax => given char
	; eax => return value (0 for false, 1 for true)

	.is_alpha
		cmp	eax, 'z'
		jg	is_alpha_false
		cmp	eax, 'A'
		jl	is_alpha_false
		
		cmp	eax, 'a'
		jl	uppercase_check
		cmp	eax, 'A'
		jg	lowercase_check

		.uppercase_check
			cmp	eax, 'A'
			jl	is_alpha_false
			cmp	eax, 'Z'
			jg	is_alpha_false
			jmp	is_alpha_true

		.lowercase_check
			cmp	eax, 'a'
			jl	is_alpha_false
			cmp	eax, 'z'
			jg	is_alpha_false
			jmp	is_alpha_true
	
	.is_alpha_true
		mov	eax, 1
		ret
	.is_alpha_false
		mov	eax, 0
		ret

	;;;;;;;;;;;;;;; STRLEN FUNCTION ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; returns the length of a given string (must be a zero-terminated string, terminator byte not counted)
	; parameter list :
	; si => pointer to source string
	; bx => returned length
	.strlen
		push	si
		xor	bx, bx
		.cmploop
			cmp	byte[si], 0
			je	endcmp
			inc	bx
			inc	si
			jmp	cmploop
		.endcmp
			pop	si
			ret
	
	.char_to_lower
	;;;;;;;;;;;;;;;;   CHAR_TO_LOWER FUNCTION ;;;;;;;;;;;;;;;;;;;
	; converts a given character to its lower case equivalent
	; parameter list :
	; eax => char
	; eax => output char
		cmp	eax, 'a'
		jge	return_point
		cmp	eax, 'A'
		jl	return_point
		add	eax, ' '
		ret

	.char_to_upper
	;;;;;;;;;;;;;;;; CHAR_TO_UPPER FUNCTION ;;;;;;;;;;;;;;;;;;
	; convert a given character to its upper case equivalent
	; parameter list :
	; eax => input char
	; eax => output char
		cmp	eax, 'A'
		jl	return_point
		cmp	eax, 'z'
		jge	return_point
		cmp	eax, 'a'
		jl	return_point
		sub	eax, ' '
		ret

	.char_to_int
	;;;;;;;;;;;;;;;;;  CHAR_TO_INT FUNCTION ;;;;;;;;;;;;;;;;;;;;;
	; convert the given ascii char to its numerical representation
	; parameter list :
	; eax => given char
	; dl => base
	; eax => returned result

		cmp	dl, 10
		je	decimal_convert
		cmp	dl, 16
		je	hex_convert
		cmp	dl, 2
		je	bin_convert
		jne	return_point

		.decimal_convert
			cmp	eax, '9'
			jg	return_point
			cmp	eax, '0'
			jl	return_point

			sub	eax, '0'
			ret
		.hex_convert
			push	eax

			call	is_alpha
			cmp	eax, 0
			pop	eax
			je	decimal_convert

			call	char_to_lower
			sub	eax, 87
			ret
		.bin_convert
			sub	eax, '0'
			ret
		
	;;;;;;;;;;;;;;;;;; BSTOI FUNCTION ;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; convert the given number string to a numerical representation
	; convert the given number string to a numerical representation
	.bstoi
		; si => source string
		; dl => base
		; ebx => result accumulator
		push	bp
		mov	bp, sp

		mov	si, [sp + 8]
		mov	edx, [sp + 12]

		xor	ebx, ebx

		call	strlen
		mov	ecx, bx

		xor	bx, bx

		mov	eax, byte[si]

		cmp	dl, 10
		je	conversion_loop
		cmp	dl, 16
		je	conversion_loop
		cmp	dl, 2
		je	conversion_loop
		jne	bstoi_invalid	

		.conversion_loop
			; result = result * base + char_to_int(s[i])
			; calculate single digit value
			mov	eax, byte[si]
			call	char_to_int
			mul	ebx, dl
			add	ebx, eax

			inc	si
			loop	conversion_loop

			mov	sp, bp
			pop	bp
			ret

		.bstoi_invalid
			mov	ebx, 0xffffffff

			mov	sp, bp
			pop	bp
			ret

		;;;;;;;;;; BITOA FUNCTION ;;;;;;;;;;;;;;;;
		; converts the given number into an ascii string and places the result in the given string pointer
		; function signature: bitoa(uint32 num, uint32 base, char* result)
		.bitoa
		    push bp
		    mov bp, sp

		    ; Preserve caller registers
		    push eax
		    push ebx
		    push ecx
		    push edx
		    push si
		    push di

		    ; Load arguments
		    mov si,  [bp + 8]     ; result pointer
		    mov ebx, [bp + 12]    ; base
		    mov eax, [bp + 16]    ; number

		    ; Validate base: 2 <= base <= 36
		    cmp ebx, 2
		    jb bitoa_invalid

		    cmp ebx, 36
		    ja bitoa_invalid

		    ; Generate digits in reverse order
		    .bitoa_convert
			; Calculate remainder
			mov ecx, eax
			rem ecx, ebx
			; Calculate quotient
			div eax, ebx

			; Convert remainder to ASCII
			cmp ecx, 10
			jb bitoa_decimal_digit

			; Digits 10-35 => 'A'-'Z'
			add ecx, 'A' - 10
			jmp bitoa_store_digit

		    .bitoa_decimal_digit
			; Digits 0-9 => '0'-'9'
			add ecx, '0'

		    .bitoa_store_digit

			; Store ASCII character
			mov byte[si], cl
			inc si

			; Continue until quotient becomes zero
			cmp eax, 0
			jne bitoa_convert

		    ; Append null terminator
		    mov byte[si], 0

		    ; Prepare to reverse the string
		    mov di, [bp + 8]    ; Start of string
		    dec si              ; Last character before NULL

		    .bitoa_reverse

			; Stop when pointers meet or cross
			cmp di, si
			jae bitoa_end

			; Swap characters
			mov dl, byte[di]
			mov cl, byte[si]

			mov byte[di], cl
			mov byte[si], dl

			; Move pointers toward the center
			inc di
			dec si

			jmp bitoa_reverse

		    .bitoa_invalid
			; Invalid base => empty string
			mov byte[si], 0

		    .bitoa_end

			; Restore caller registers
			pop di
			pop si
			pop edx
			pop ecx
			pop ebx
			pop eax

			mov sp, bp
			pop bp
			ret

		.helpers_end


section BIOS_INTERRUPTS
	
	section BIOS_EXCEPTIONS
		.exp_divzero
			push	edx
			push	eex
			push	si
			la	si, divzero
			mov	edx, COLOR_RED
			mov	eex, COLOR_BLACK
			call	bios_println
			pop	si
			pop	eex
			pop	edx
			ext
		.exp_invop
			mov	bp, stack_base_addr
			mov	sp, bp
			add	sp, stack_len

			la	si, invop
			outs
			mov	edx, COLOR_RED
			mov	eex, COLOR_BLACK
			mov	ah, 3
			int	0x2
			dbg
			ext

		.divzero defb "Division by Zero", 0xa, 0
		.invop defb "Invalid Opcode, Stopping the program.", 0xa, 0
	
	section BIOS_IO
		cmp	ah, byte[io_table_len]
		jae	invalid_interrupt_routine
		push	efx
		mov	efx, ah
		call	[io_table + efx * 4]
		pop	efx
		iret
		.chrout
			ret
		.chrin
			ret


		; IO services AH=2 (bios_print) and AH=3 (bios_println):
		;   SI  = zero-terminated string pointer
		;   EDX = foreground color
		;   EEX = background color
		.bios_print

		    push bp
		    mov bp, sp
		    sub sp, 4

		    ; Preserve registers (including background color in EEX).
		    push eax
		    push ebx
		    push edx
		    push eex
		    push si

		    ; Save string pointer
		    mov [bp - 4], si

		    .bios_print_loop

			; Read current character
			mov si, [bp - 4]
			mov eax, byte[si]

			; End of string
			cmp eax, 0
			je bios_print_end

			; Handle newline character
			cmp eax, 0x0a
			je bios_print_linefeed

			; Read cached width and find the last X for a complete glyph.
			mov ebx, dword[screen_width_ptr]
			mov ebx, dword[ebx]
			; Avoid underflow if the display is narrower than one glyph.
			cmp ebx, FONT_WIDTH
			jb bios_print_linefeed
			sub ebx, FONT_WIDTH

			; Read current X cursor from RAM
			mov eax, dword[x_cursor_ptr]
			mov eax, dword[eax]

			; Wrap if the next character will not fit.
			cmp eax, ebx
			jle bios_print_draw

			; Wrap to next line
			call bios_cursor_newline

		    .bios_print_draw

			; Load character into SI
			mov si, [bp - 4]
			mov eax, byte[si]
			mov si, eax

			; Read X cursor
			mov eax, dword[x_cursor_ptr]
			mov eax, dword[eax]

			; Read Y cursor
			mov ebx, dword[y_cursor_ptr]
			mov ebx, dword[ebx]

			; Call drawchar
			push si
			push eax
			push ebx
			push edx
			push eex

			call drawchar
			add sp, 20

			; Advance X cursor
			mov eax, dword[x_cursor_ptr]
			add dword[eax], FONT_WIDTH

			jmp bios_print_next

		    .bios_print_linefeed
			call bios_cursor_newline

		    .bios_print_next
			inc dword[bp - 4]
			jmp bios_print_loop

		    .bios_print_end

			; Restore registers
			pop si
			pop eex
			pop edx
			pop ebx
			pop eax

			; Restore stack frame
			mov sp, bp
			pop bp

			ret

		.bios_println
			call	bios_print
			call	bios_cursor_newline
			ret

		.scrdim
			inp	ax, DISP_WIDTH_PORT
			inp	bx, DISP_HEIGHT_PORT
			ret

		.io_table defdw chrout, chrin, bios_print, bios_println, scrdim
		.io_table_len defb ($ - io_table) / 4

		.bios_cursor_newline
		    push eax
		    push ebx

		    ; Read cached screen height from reserved RAM.
		    mov ebx, dword[screen_height_ptr]
		    mov ebx, dword[ebx]

		    ; Reset X cursor
		    mov eax, dword[x_cursor_ptr]
		    mov dword[eax], 0

		    ; Advance Y cursor
		    mov eax, dword[y_cursor_ptr]
		    add dword[eax], FONT_HEIGHT

		    ; Check bottom boundary using the current height.
		    cmp dword[eax], ebx
		    jl bios_cursor_newline_done

		    ; Wrap vertically
		    mov dword[eax], 0

		    .bios_cursor_newline_done
			pop ebx
			pop eax
			ret


	section BIOS_DISK
		cmp	ah, byte[disk_table_len]
		jae	invalid_interrupt_routine

		push	efx
		mov	efx, ah
		call	[disk_table + efx * 4]
		pop	efx
		iret

		.dskstat
			ret

		.rdsect
			ret


		.disk_table defdw dskstat, rdsect
		.disk_table_len defb ($ - disk_table) / 4

	section BIOS_TEST
		cmp	ah, 0
		jg	invalid_interrupt_routine
		mov	di, ah
		push	di
		call	[test_table + di * 4]
		pop	di
		iret
		.test_interrupt
			push	ebx
			push	ecx
			push	edx
			push	eex
			push	efx

			la	si, test_msg
			outs
			mov	eax, 10203040

			pop	efx
			pop	eex
			pop	edx
			pop	ecx
			pop	ebx

			ret

		.test_table defdw test_interrupt
		.test_table_len defb ($ - test_table) / 4
		.test_msg defb "Interrupt initiated", 0xa, 0

	section BIOS_TIMER

		.tick_handler
			
			push	eax
			mov	eax, dword[timer_ticks]
			add	[eax], 1

			pop	eax
			iret

		.timer_table defdw tick_handler
		.timer_table_len defb ($ - timer_table) / 4	
	
	.invalid_interrupt_routine
		push	si
		la	si, invalid_interrupt_msg
		outs
		pop	si
		iret

	.interruptv_table defdw
		exp_divzero, 	; 0x00 => Division by zero exception
		exp_invop, 	; 0x01 => Invalid opcode exception
		BIOS_IO, 	; 0x02 => BIOS IO services
		BIOS_DISK,	   ; 0x03 => BIOS disk services
		BIOS_TEST,	   ; 0x04 => Testing interrupt
		0,	   ; 0x05
		0,	   ; 0x06
		0,	   ; 0x07
		0,	   ; 0x08
		0,	   ; 0x09
		0,	   ; 0x0a
		0,	   ; 0x0b
		0,	   ; 0x0c
		0,	   ; 0x0d
		0,	   ; 0x0e
		0,	   ; 0x0f
		0,	   ; 0x10
		0,	   ; 0x11
		0,	   ; 0x12
		0,	   ; 0x13
		0,	   ; 0x14
		0,	   ; 0x15
		0,	   ; 0x16
		0,	   ; 0x17
		0,	   ; 0x18
		0,	   ; 0x19
		0,	   ; 0x1a
		0,	   ; 0x1b
		0,	   ; 0x1c
		0,	   ; 0x1d
		0,	   ; 0x1e
		0,	   ; 0x1f
		BIOS_TIMER, ; 0x20 => Bios timer interrupt handler
		0,	   ; 0x21
		0,	   ; 0x22
		0,	   ; 0x23
		0,	   ; 0x24
		0,	   ; 0x25
		0,	   ; 0x26
		0,	   ; 0x27
		0,	   ; 0x28
		0,	   ; 0x29
		0,	   ; 0x2a
		0,	   ; 0x2b
		0,	   ; 0x2c
		0,	   ; 0x2d
		0,	   ; 0x2e
		0,	   ; 0x2f
		0,	   ; 0x30
		0,	   ; 0x31
		0,	   ; 0x32
		0,	   ; 0x33
		0,	   ; 0x34
		0,	   ; 0x35
		0,	   ; 0x36
		0,	   ; 0x37
		0,	   ; 0x38
		0,	   ; 0x39
		0,	   ; 0x3a
		0,	   ; 0x3b
		0,	   ; 0x3c
		0,	   ; 0x3d
		0,	   ; 0x3e
		0,	   ; 0x3f
		0,	   ; 0x40
		0,	   ; 0x41
		0,	   ; 0x42
		0,	   ; 0x43
		0,	   ; 0x44
		0,	   ; 0x45
		0,	   ; 0x46
		0,	   ; 0x47
		0,	   ; 0x48
		0,	   ; 0x49
		0,	   ; 0x4a
		0,	   ; 0x4b
		0,	   ; 0x4c
		0,	   ; 0x4d
		0,	   ; 0x4e
		0,	   ; 0x4f
		0,	   ; 0x50
		0,	   ; 0x51
		0,	   ; 0x52
		0,	   ; 0x53
		0,	   ; 0x54
		0,	   ; 0x55
		0,	   ; 0x56
		0,	   ; 0x57
		0,	   ; 0x58
		0,	   ; 0x59
		0,	   ; 0x5a
		0,	   ; 0x5b
		0,	   ; 0x5c
		0,	   ; 0x5d
		0,	   ; 0x5e
		0,	   ; 0x5f
		0,	   ; 0x60
		0,	   ; 0x61
		0,	   ; 0x62
		0,	   ; 0x63
		0,	   ; 0x64
		0,	   ; 0x65
		0,	   ; 0x66
		0,	   ; 0x67
		0,	   ; 0x68
		0,	   ; 0x69
		0,	   ; 0x6a
		0,	   ; 0x6b
		0,	   ; 0x6c
		0,	   ; 0x6d
		0,	   ; 0x6e
		0,	   ; 0x6f
		0,	   ; 0x70
		0,	   ; 0x71
		0,	   ; 0x72
		0,	   ; 0x73
		0,	   ; 0x74
		0,	   ; 0x75
		0,	   ; 0x76
		0,	   ; 0x77
		0,	   ; 0x78
		0,	   ; 0x79
		0,	   ; 0x7a
		0,	   ; 0x7b
		0,	   ; 0x7c
		0,	   ; 0x7d
		0,	   ; 0x7e
		0,	   ; 0x7f
		0,	   ; 0x80
		0,	   ; 0x81
		0,	   ; 0x82
		0,	   ; 0x83
		0,	   ; 0x84
		0,	   ; 0x85
		0,	   ; 0x86
		0,	   ; 0x87
		0,	   ; 0x88
		0,	   ; 0x89
		0,	   ; 0x8a
		0,	   ; 0x8b
		0,	   ; 0x8c
		0,	   ; 0x8d
		0,	   ; 0x8e
		0,	   ; 0x8f
		0,	   ; 0x90
		0,	   ; 0x91
		0,	   ; 0x92
		0,	   ; 0x93
		0,	   ; 0x94
		0,	   ; 0x95
		0,	   ; 0x96
		0,	   ; 0x97
		0,	   ; 0x98
		0,	   ; 0x99
		0,	   ; 0x9a
		0,	   ; 0x9b
		0,	   ; 0x9c
		0,	   ; 0x9d
		0,	   ; 0x9e
		0,	   ; 0x9f
		0,	   ; 0xa0
		0,	   ; 0xa1
		0,	   ; 0xa2
		0,	   ; 0xa3
		0,	   ; 0xa4
		0,	   ; 0xa5
		0,	   ; 0xa6
		0,	   ; 0xa7
		0,	   ; 0xa8
		0,	   ; 0xa9
		0,	   ; 0xaa
		0,	   ; 0xab
		0,	   ; 0xac
		0,	   ; 0xad
		0,	   ; 0xae
		0,	   ; 0xaf
		0,	   ; 0xb0
		0,	   ; 0xb1
		0,	   ; 0xb2
		0,	   ; 0xb3
		0,	   ; 0xb4
		0,	   ; 0xb5
		0,	   ; 0xb6
		0,	   ; 0xb7
		0,	   ; 0xb8
		0,	   ; 0xb9
		0,	   ; 0xba
		0,	   ; 0xbb
		0,	   ; 0xbc
		0,	   ; 0xbd
		0,	   ; 0xbe
		0,	   ; 0xbf
		0,	   ; 0xc0
		0,	   ; 0xc1
		0,	   ; 0xc2
		0,	   ; 0xc3
		0,	   ; 0xc4
		0,	   ; 0xc5
		0,	   ; 0xc6
		0,	   ; 0xc7
		0,	   ; 0xc8
		0,	   ; 0xc9
		0,	   ; 0xca
		0,	   ; 0xcb
		0,	   ; 0xcc
		0,	   ; 0xcd
		0,	   ; 0xce
		0,	   ; 0xcf
		0,	   ; 0xd0
		0,	   ; 0xd1
		0,	   ; 0xd2
		0,	   ; 0xd3
		0,	   ; 0xd4
		0,	   ; 0xd5
		0,	   ; 0xd6
		0,	   ; 0xd7
		0,	   ; 0xd8
		0,	   ; 0xd9
		0,	   ; 0xda
		0,	   ; 0xdb
		0,	   ; 0xdc
		0,	   ; 0xdd
		0,	   ; 0xde
		0,	   ; 0xdf
		0,	   ; 0xe0
		0,	   ; 0xe1
		0,	   ; 0xe2
		0,	   ; 0xe3
		0,	   ; 0xe4
		0,	   ; 0xe5
		0,	   ; 0xe6
		0,	   ; 0xe7
		0,	   ; 0xe8
		0,	   ; 0xe9
		0,	   ; 0xea
		0,	   ; 0xeb
		0,	   ; 0xec
		0,	   ; 0xed
		0,	   ; 0xee
		0,	   ; 0xef
		0,	   ; 0xf0
		0,	   ; 0xf1
		0,	   ; 0xf2
		0,	   ; 0xf3
		0,	   ; 0xf4
		0,	   ; 0xf5
		0,	   ; 0xf6
		0,	   ; 0xf7
		0,	   ; 0xf8
		0,	   ; 0xf9
		0,	   ; 0xfa
		0,	   ; 0xfb
		0,	   ; 0xfc
		0,	   ; 0xfd
		0,	   ; 0xfe
		0	   ; 0xff

		.interruptv_table_len defdw ($ - interruptv_table) / 4
		.invalid_interrupt_msg defb "Invalid interrupt", 0xa, 0


section BIOS_MUTABLE_DATA
	; These ROM symbols hold pointers to writable, reserved RAM slots.
	.screen_height_ptr defdw 0x2f4
	.screen_width_ptr defdw 0x2f8
	.x_cursor_ptr defdw 0x300
	.y_cursor_ptr defdw 0x300 - 4
	.keybrd_buff defdw KB_BUFF_ADDR
	.timer_ticks defdw 0x00

section BIOS_DATA

	.bios_hello defb "=========TKY BIOS v2.0a=========", 0
	.finish_txt defb "BIOS FINISH POINT!", 0
	.load_status defb "Loading IVT entries...", 0
	.load_finish defb "Done!", 0
	.halt defb "CPU Halted", 0

	.stack_base_addr defdw 0x400
	.stack_len defdw 0x400

	
	.kb_head defdw KB_BUFF_ADDR
	.kb_tail defdw kb_head
	.kb_buff_len defw 512 

	.text_buff_ptr defdw TEXT_BUFF_ADDR
	.text_buff_len defdw 500 * 500 * 2

	section BIOS_CHAR_LOOKUP
		
		.FONT_SPACE defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000

		.FONT_EXCLAMATION defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00111100,
			0b00111100,
			0b00111100,
			0b00111100,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00000000
		.FONT_QUOTE defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00110110,
			0b00110110,
			0b00110110,
			0b00110110,
			0b00010100,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_HASH defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01101100,
			0b01101100,
			0b01101100,
			0b11111110,
			0b01101100,
			0b01101100,
			0b11111110,
			0b01101100,
			0b01101100,
			0b01101100,
			0b00000000,
			0b00000000
		.FONT_DOLLAR defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b01111100,
			0b11000110,
			0b11000000,
			0b01111000,
			0b00111100,
			0b00000110,
			0b11000110,
			0b01111100,
			0b00011000,
			0b00011000,
			0b00000000
		.FONT_PERCENT defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01100010,
			0b01100110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_AMPERSAND defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111000,
			0b01101100,
			0b00111000,
			0b00110000,
			0b01110110,
			0b01111110,
			0b11001100,
			0b11001100,
			0b11001100,
			0b01110110,
			0b00000000,
			0b00000000
		.FONT_APOSTROPHE defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00011000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_LPAREN defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00011000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00011000,
			0b00001100,
			0b00000000,
			0b00000000
		.FONT_RPAREN defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00110000,
			0b00011000,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00011000,
			0b00110000,
			0b00000000,
			0b00000000
		.FONT_ASTERISK defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01101100,
			0b00111000,
			0b11111110,
			0b00111000,
			0b01101100,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_PLUS defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b01111110,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_COMMA defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00011000,
			0b00000000,
			0b00000000
		.FONT_MINUS defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111110,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_DOT defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00000000
		.FONT_SLASH defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000010,
			0b00000110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100000,
			0b11000000,
			0b10000000,
			0b00000000,
			0b00000000
		.FONT_DIGIT_0 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11001110,
			0b11011110,
			0b11110110,
			0b11100110,
			0b11000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_DIGIT_1 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b01111000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b01111110,
			0b00000000,
			0b00000000
		.FONT_DIGIT_2 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b00000110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100000,
			0b11000110,
			0b11111110,
			0b00000000,
			0b00000000
		.FONT_DIGIT_3 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b00000110,
			0b00000110,
			0b00111100,
			0b00000110,
			0b00000110,
			0b00000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_DIGIT_4 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00011100,
			0b00111100,
			0b01101100,
			0b11001100,
			0b11001100,
			0b11111110,
			0b00001100,
			0b00001100,
			0b00011110,
			0b00000000,
			0b00000000
		.FONT_DIGIT_5 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111110,
			0b11000000,
			0b11000000,
			0b11000000,
			0b11111100,
			0b00000110,
			0b00000110,
			0b00000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_DIGIT_6 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000000,
			0b11000000,
			0b11111100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_DIGIT_7 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111110,
			0b11000110,
			0b00000110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00000000,
			0b00000000
		.FONT_DIGIT_8 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_DIGIT_9 defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01111110,
			0b00000110,
			0b00000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_COLON defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00001100,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00001100,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_SEMICOLON defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00001100,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00011000,
			0b00000000,
			0b00000000
		.FONT_LT defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100000,
			0b11000000,
			0b01100000,
			0b00110000,
			0b00011000,
			0b00001100,
			0b00000000,
			0b00000000
		.FONT_EQUAL defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111110,
			0b00000000,
			0b11111110,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_GT defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01100000,
			0b00110000,
			0b00011000,
			0b00001100,
			0b00000110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100000,
			0b00000000,
			0b00000000
		.FONT_QUESTION defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b00001100,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00000000
		.FONT_AT defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11011110,
			0b11011110,
			0b11011110,
			0b11011100,
			0b11000000,
			0b01111110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_A defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111000,
			0b01101100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11111110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_B defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01111100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b11111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_C defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111100,
			0b01100110,
			0b11000010,
			0b11000000,
			0b11000000,
			0b11000000,
			0b11000000,
			0b11000010,
			0b01100110,
			0b00111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_D defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111000,
			0b01101100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01101100,
			0b11111000,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_E defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111110,
			0b01100110,
			0b01100000,
			0b01100100,
			0b01111100,
			0b01100100,
			0b01100000,
			0b01100000,
			0b01100110,
			0b11111110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_F defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111110,
			0b01100110,
			0b01100000,
			0b01100100,
			0b01111100,
			0b01100100,
			0b01100000,
			0b01100000,
			0b01100000,
			0b11110000,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_G defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11000000,
			0b11000000,
			0b11000000,
			0b11001110,
			0b11000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_H defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11111110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_I defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111100,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_J defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111100,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b11011000,
			0b11011000,
			0b01110000,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_K defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11001100,
			0b11011000,
			0b11110000,
			0b11110000,
			0b11011000,
			0b11001100,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_L defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11110000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100010,
			0b01100110,
			0b11111110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_M defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11101110,
			0b11101110,
			0b11111110,
			0b11010110,
			0b11010110,
			0b11010110,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_N defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11100110,
			0b11100110,
			0b11110110,
			0b11011110,
			0b11001110,
			0b11001110,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_O defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_P defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01111100,
			0b01100000,
			0b01100000,
			0b01100000,
			0b11110000,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_Q defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11010110,
			0b11010110,
			0b01111100,
			0b00000110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_R defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01111100,
			0b01111000,
			0b01101100,
			0b01100110,
			0b01100110,
			0b11100110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_S defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000000,
			0b11000000,
			0b01110000,
			0b00011100,
			0b00000110,
			0b00000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_T defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111110,
			0b01011010,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_U defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_V defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01101100,
			0b00111000,
			0b00010000,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_W defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11010110,
			0b11010110,
			0b11010110,
			0b11111110,
			0b11101110,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_X defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01101100,
			0b00111000,
			0b00111000,
			0b01101100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_Y defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b00111100,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00111100,
			0b00000000,
			0b00000000
		.FONT_UPPERCASE_Z defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111110,
			0b11000110,
			0b10000110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100000,
			0b11000010,
			0b11000110,
			0b11111110,
			0b00000000,
			0b00000000
		.FONT_LBRACKET defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_BACKSLASH defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b10000000,
			0b11000000,
			0b01100000,
			0b00110000,
			0b00011000,
			0b00001100,
			0b00000110,
			0b00000010,
			0b00000000,
			0b00000000
		.FONT_RBRACKET defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_CARET defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00010000,
			0b00111000,
			0b01101100,
			0b11000110,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_UNDERSCORE defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111111,
			0b00000000
		.FONT_BACKTICK defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00001100,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_A defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111000,
			0b00001100,
			0b01111100,
			0b11001100,
			0b11001100,
			0b11011100,
			0b01110110,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_B defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11100000,
			0b01100000,
			0b01100000,
			0b01111100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b11111100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_C defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000000,
			0b11000000,
			0b11000000,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_D defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011100,
			0b00001100,
			0b00001100,
			0b01111100,
			0b11001100,
			0b11001100,
			0b11001100,
			0b11001100,
			0b11001100,
			0b01111110,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_E defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11111110,
			0b11000000,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_F defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011100,
			0b00110110,
			0b00110000,
			0b00110000,
			0b11111100,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b01111000,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_G defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01110110,
			0b11001110,
			0b11000110,
			0b11000110,
			0b11001110,
			0b01110110,
			0b00000110,
			0b11000110,
			0b01111100,
			0b00000000
		.FONT_LOWERCASE_H defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11100000,
			0b01100000,
			0b01100000,
			0b01111100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b11100110,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_I defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00111000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00111100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_J defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00001100,
			0b00000000,
			0b00011100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b00001100,
			0b11001100,
			0b11001100,
			0b01111000,
			0b00000000
		.FONT_LOWERCASE_K defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11100000,
			0b01100000,
			0b01100000,
			0b01100110,
			0b01100110,
			0b01101100,
			0b01111000,
			0b01101100,
			0b01100110,
			0b11100110,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_L defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_M defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01101100,
			0b11111110,
			0b11010110,
			0b11010110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_N defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11011100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_O defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_P defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11011100,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01100110,
			0b01111100,
			0b01100000,
			0b01100000,
			0b11110000,
			0b00000000
		.FONT_LOWERCASE_Q defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01110110,
			0b11001100,
			0b11001100,
			0b11001100,
			0b11001100,
			0b01111100,
			0b00001100,
			0b00001100,
			0b00011110,
			0b00000000
		.FONT_LOWERCASE_R defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11011100,
			0b01100110,
			0b01100000,
			0b01100000,
			0b01100000,
			0b01100000,
			0b11110000,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_S defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111100,
			0b11000110,
			0b11000000,
			0b01111100,
			0b00000110,
			0b11000110,
			0b01111100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_T defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b11111100,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00110000,
			0b00110110,
			0b00011100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_U defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11001100,
			0b11001100,
			0b11001100,
			0b11001100,
			0b11001100,
			0b11001100,
			0b01110110,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_V defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b01101100,
			0b00111000,
			0b00010000,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_W defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11010110,
			0b11010110,
			0b11010110,
			0b11111110,
			0b01101100,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_X defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b01101100,
			0b00111000,
			0b01101100,
			0b11000110,
			0b11000110,
			0b00000000,
			0b00000000
		.FONT_LOWERCASE_Y defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11000110,
			0b11001110,
			0b01110110,
			0b00000110,
			0b11000110,
			0b01111100,
			0b00000000
		.FONT_LOWERCASE_Z defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111110,
			0b10000110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100010,
			0b11111110,
			0b00000000,
			0b00000000
		.FONT_LBRACE defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001110,
			0b00011000,
			0b00011000,
			0b00011000,
			0b01110000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00001110,
			0b00000000,
			0b00000000
		.FONT_PIPE defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00000000
		.FONT_RBRACE defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01110000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00001110,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b01110000,
			0b00000000,
			0b00000000
		.FONT_TILDE defb 
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01110110,
			0b11011100,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		.FONT_CURSOR defb 
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111,
			0b11111111


		.font_ascii_map defdw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		    FONT_SPACE,
		    FONT_EXCLAMATION,
		    FONT_QUOTE,
		    FONT_HASH,
		    FONT_DOLLAR,
		    FONT_PERCENT,
		    FONT_AMPERSAND,
		    FONT_APOSTROPHE,
		    FONT_LPAREN,
		    FONT_RPAREN,
		    FONT_ASTERISK,
		    FONT_PLUS,
		    FONT_COMMA,
		    FONT_MINUS,
		    FONT_DOT,
		    FONT_SLASH,
		    FONT_DIGIT_0,
		    FONT_DIGIT_1,
		    FONT_DIGIT_2,
		    FONT_DIGIT_3,
		    FONT_DIGIT_4,
		    FONT_DIGIT_5,
		    FONT_DIGIT_6,
		    FONT_DIGIT_7,
		    FONT_DIGIT_8,
		    FONT_DIGIT_9,
		    FONT_COLON,
		    FONT_SEMICOLON,
		    FONT_LT,
		    FONT_EQUAL,
		    FONT_GT,
		    FONT_QUESTION,
		    FONT_AT,
		    FONT_UPPERCASE_A,
		    FONT_UPPERCASE_B,
		    FONT_UPPERCASE_C,
		    FONT_UPPERCASE_D,
		    FONT_UPPERCASE_E,
		    FONT_UPPERCASE_F,
		    FONT_UPPERCASE_G,
		    FONT_UPPERCASE_H,
		    FONT_UPPERCASE_I,
		    FONT_UPPERCASE_J,
		    FONT_UPPERCASE_K,
		    FONT_UPPERCASE_L,
		    FONT_UPPERCASE_M,
		    FONT_UPPERCASE_N,
		    FONT_UPPERCASE_O,
		    FONT_UPPERCASE_P,
		    FONT_UPPERCASE_Q,
		    FONT_UPPERCASE_R,
		    FONT_UPPERCASE_S,
		    FONT_UPPERCASE_T,
		    FONT_UPPERCASE_U,
		    FONT_UPPERCASE_V,
		    FONT_UPPERCASE_W,
		    FONT_UPPERCASE_X,
		    FONT_UPPERCASE_Y,
		    FONT_UPPERCASE_Z,
		    FONT_LBRACKET,
		    FONT_BACKSLASH,
		    FONT_RBRACKET,
		    FONT_CARET,
		    FONT_UNDERSCORE,
		    FONT_BACKTICK,
		    FONT_LOWERCASE_A,
		    FONT_LOWERCASE_B,
		    FONT_LOWERCASE_C,
		    FONT_LOWERCASE_D,
		    FONT_LOWERCASE_E,
		    FONT_LOWERCASE_F,
		    FONT_LOWERCASE_G,
		    FONT_LOWERCASE_H,
		    FONT_LOWERCASE_I,
		    FONT_LOWERCASE_J,
		    FONT_LOWERCASE_K,
		    FONT_LOWERCASE_L,
		    FONT_LOWERCASE_M,
		    FONT_LOWERCASE_N,
		    FONT_LOWERCASE_O,
		    FONT_LOWERCASE_P,
		    FONT_LOWERCASE_Q,
		    FONT_LOWERCASE_R,
		    FONT_LOWERCASE_S,
		    FONT_LOWERCASE_T,
		    FONT_LOWERCASE_U,
		    FONT_LOWERCASE_V,
		    FONT_LOWERCASE_W,
		    FONT_LOWERCASE_X,
		    FONT_LOWERCASE_Y,
		    FONT_LOWERCASE_Z,
		    FONT_LBRACE,
		    FONT_PIPE,
		    FONT_RBRACE,
		    FONT_TILDE,
		    FONT_CURSOR
