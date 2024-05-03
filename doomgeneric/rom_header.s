.align  1
.section ".rom_header", "ax"
  .align  1

  .ascii  "DOOM                "   # Title
  .byte   0x00,0x00,0x00,0x00,0x00 # Must be zero
  .ascii  "PR"                     # Maker ID
  .ascii  "TMPL"                   # Game ID
  .byte   0x00                     # Version
