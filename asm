          ConfigPath:
0800242c:   push    {r4, lr}
0800242e:   mov     r4, r0
14        	memcpy(p[0].call, "AKLPRZ", 6), p[0].ssid = 0;
08002430:   movs    r2, #6
08002432:   ldr     r1, [pc, #44]   ; (0x8002460 <ConfigPath+52>)
08002434:   bl      0x8005d90 <memcpy>
08002438:   movs    r3, #0
0800243a:   strb    r3, [r4, #6]
20        	memcpy(p[1].call, _CALL, 6), p[1].ssid = _SSID;
0800243c:   movs    r2, #6
0800243e:   ldr     r1, [pc, #36]   ; (0x8002464 <ConfigPath+56>)
08002440:   adds    r0, r4, #7
08002442:   bl      0x8005d90 <memcpy>
08002446:   movs    r3, #12
08002448:   strb    r3, [r4, #13]
21        	memcpy(p[2].call, "WIDE2", 6), p[2].ssid = 1;
0800244a:   movs    r2, #6
0800244c:   ldr     r1, [pc, #24]   ; (0x8002468 <ConfigPath+60>)
0800244e:   add.w   r0, r4, #14
08002452:   bl      0x8005d90 <memcpy>
08002456:   movs    r3, #1
08002458:   strb    r3, [r4, #20]
30        }
0800245a:   movs    r0, #3
0800245c:   pop     {r4, pc}
0800245e:   nop     
08002460:   strh    r4, [r5, #26]
08002462:   lsrs    r0, r0, #32
08002464:   strh    r0, [r3, #22]
08002466:   lsrs    r0, r0, #32
08002468:   strh    r4, [r6, #26]
0800246a:   lsrs    r0, r0, #32


path_len = ConfigPath(path);
0800278e:   ldr.w   r8, [pc, #428]  ; 0x800293c <main(int, char**)+468>
08002792:   mov     r0, r8
08002794:   bl      0x800242c <ConfigPath>
08002798:   ldr     r3, [pc, #332]  ; (0x80028e8 <main(int, char**)+384>)
0800279a:   strb    r0, [r3, #0]
 98         LedConfig();
0800279c:   bl      0x80023c8 <LedConfig>