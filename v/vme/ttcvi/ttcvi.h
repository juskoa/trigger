/*REGSTART16 */
#define CSR1 0x80
#define CSR2 0x82
#define BOARDreset 0x84
#define L1Agen 0x86
#define EOCreset 0x8c
#define BGo0mode 0x90
#define BGo1mode 0x98
#define BGo2mode 0xa0
#define BGo3mode 0xa8
#define IDel0 0x92
#define IDur0 0x94
#define IDel1 0x9a
#define IDur1 0x9c
#define IDel2 0xa2
#define IDur2 0xa4
#define IDel3 0xaa
#define IDur3 0xac
#define EOcnt1 0x88    /*  8 bits (d23-d16)  */
#define EOcnt2 0x8a    /* 16 bits (d15-d0    */
#define BCLFACttcrxadr 0xC0
#define BCLFACdata     0xC2
#define BCSFACdata     0xC4   /* 8 bits broadcast data */
/*REGEND */
/*REGSTART32 */
#define BCDBG0 0xb0
#define BCDBG1 0xb4
#define BCDBG2 0xb8
#define BCDBG3 0xbc
/*REGEND */

#define TTCBrExRe 0x80010000
#define TTCAL1h  0x1000
#define TTCAL1d  0x2000
#define TTCAL2ah 0x3000
#define TTCAL2ad 0x4000
#define TTCAL2r  0x5000
#define TTCARoIh 0x6000
#define TTCARoId 0x7000
#define TTCDATAMask 0xfff

#define TTCppdelay 3125  // (~2us: 3422+80+54=3556)
/* 3470 (with CALIBRATION_BC=3556) will casue BrcstStr1 signal near
to L0 (depends on cables) in trg lab */

