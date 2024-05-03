.section .text
.align  4

.global start
start:
  # Enable IC
  mov 2, r6
  ldsr r6, chcw

  # Init data
  movhi hi(__data_lma), r0, r6
  movea lo(__data_lma), r6, r6
  movhi hi(__data_start__), r0, r7
  movea lo(__data_start__), r7, r7
  movhi hi(__data_end__), r0, r8
  movea lo(__data_end__), r8, r8
  jr      .Lend_init_data

.Ltop_init_data:
  ld.b  0[r6], r9
  st.b  r9, 0[r7]
  add 1, r6
  add 1, r7
.Lend_init_data:
  cmp r8, r7
  blt .Ltop_init_data

  # Clear BSS
  movhi hi(__bss_start__), r0, r6
  movea lo(__bss_start__), r6, r6
  movhi hi(__bss_end__), r0, r7
  movea lo(__bss_end__), r7, r7
  jr  .Lend_init_bss
.Ltop_init_bss:
  st.h r0, 0[r6]
  add 1, r6
.Lend_init_bss:
  cmp r7, r6
  blt .Ltop_init_bss

  # Set SP and GP 
  movhi hi(__stack__), r0,sp
  movea lo(__stack__), sp,sp
  movhi hi(__gp__), r0, gp
  movea lo(__gp__), gp, gp

  # Clear NP flag so that interrupts work.
  ldsr r0, psw

__run_constructors:
  # Must process these in reverse order per
  # https://maskray.me/blog/2021-11-07-init-ctors-init-array
  add -4, sp # Make room for current ctor pointer
  movhi hi(__ctor_list_end__),r0,r6
  movea lo(__ctor_list_end__),r6,r6
  add -4, r6
.Lnextctor:
  movhi hi(__ctor_list__), r0, r11
  movea lo(__ctor_list__), r11, r11
  cmp r6, r11
  bgt .Ldonectors
   
  # store ptr to next ctor
  ld.w 0[r6], r11 # Load ctor ptr
  add -4, r6 
  st.w r6, 0[sp] 

  # Dynamic dispatch dance...
  jal 1f
  1:
  add 4, lp
  jmp [r11]

  ld.w 0[sp], r6
  br .Lnextctor

.Ldonectors:
  add 4, sp

  # Off we go!
  jal main
__teardown:
  # Now run destructors in forward order
  # https://maskray.me/blog/2021-11-07-init-ctors-init-array
  add -4, sp # Make room for current ctor pointer
  movhi hi(__ctor_list__),r0,r6
  movea lo(__ctor_list__),r6,r6
  add 4, r6
.Lnextdtor:
  movhi hi(__ctor_list_end__), r0, r11
  movea lo(__ctor_list_end__), r11, r11
  cmp r6, r11
  be .Ldonedtors
   
  # store ptr to next ctor
  ld.w 0[r6], r11 # Load ctor ptr
  add -4, r6 
  st.w r6, 0[sp] 

  # Dynamic dispatch dance...
  jal 1f
  1:
  add 4, lp
  jmp [r11]

  ld.w 0[sp], r6
  br .Lnextdtor
.Ldonedtors:
  add 4, sp

  # Finally, halt
.global exit
exit:
  halt

.section ".interrupt_handlers", "ax"
  # Game Pad Interrupt
  jr VbPlus_GamePadInterrupt
  .fill 0xC

  # Timer interrupt
  jr VbPlus_TimerInterrupt
  .fill 0xC

  jr VbPlus_GamePakInterrupt
  .fill 0xC

  # Communication Port Interrupt
  jr VbPlus_ComInterrupt
  .fill 0xC

  # VIP Interrupt
  jr VbPlus_VipInterrupt
  .fill 0xC

  # Unused
  .fill 0x110

  # Float Exception
  jr VbPlus_FloatException
  .fill 0xC
 
  # Unused
  .fill 0x10

  # Divide by zero
  jr VbPlus_DivideByZeroException
  .fill 0xC

  # Invalid Optcode
  jr VbPlus_InvalidOpcodeException
  .fill 0xC

  # Trap 0
  jr VbPlus_Trap0Exception
  .fill 0xC

  # Trap 1
  jr VbPlus_Trap1Exception
  .fill 0xC

  # Breakpoint
  jr VbPlus_AddrTrapException
  .fill 0xC

  # Duplexed Exception
  jr VbPlus_DuplexedException
  .fill 0xC

  # Unused
  .fill 0x10

  # Reset
  jr start
  .fill 0xC

