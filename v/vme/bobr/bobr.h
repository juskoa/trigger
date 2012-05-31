/* bobr.h */
/*REGSTART32 */
#define MessageInput 0x0800
#define addyTime0 0x0800
#define addyTime1 0x0804
#define addyTurnCount 0x0808
#define AuxMessageInput 0x0c00

  //block status addresses
#define BlockIdentificator 0x00
#define addyVMEIRQVector 0x08
#define BlockStatus 0x10
#define addyCoarseDelay 0x14
#define addyHWByteSelect 0x18
#define addyHWByteOutp 0x1c
#define addyBunchSelectOutpEnable 0x20
#define addyBunchSelectOutpEnableSubAddr 0x24
#define addyIRQByteAddrSelect 0x30
#define addyIRQByteAddrMask 0x34
#define addyTTCErrorSingle 0x40
#define addyTTCErrorDouble 0x44
#define addyTTCErrorReady 0x48
#define BlockTurnCount 0x50
/*REGEND */
#define Message2shift 0x10000
#define EnableDPRam 0x4
/*
u_int addyTime2 0x0808;
u_int addyTime3 0x080c;
u_int addyTime4 0x0810;
u_int addyTime5 0x0814;
u_int addyTime6 0x0818;
u_int addyTime7 0x081c;
u_int addySender 0x0844;
u_int addyTurnCount0 0x0848;
u_int addyTurnCount1 0x084c;
u_int addyTurnCount2 0x0850;
u_int addyTurnCount3 0x0854;
u_int addyFillNumber0 0x0858;
u_int addyFillNumber1 0x085c;
u_int addyFillNumber2 0x0860;
u_int addyFillNumber3 0x0864;
u_int addyBeamMode0 0x0868;
u_int addyBeamMode1 0x086c;
u_int addyParticleType1 0x0870;
u_int addyParticleType2 0x0874;
u_int addyMomentum0 0x0878;
u_int addyMomentum1 0x087c;
u_int addyIntensity10 0x0880;
u_int addyIntensity11 0x0884;
u_int addyIntensity12 0x0888;
u_int addyIntensity13 0x088c;
u_int addyIntensity20 0x0890;
u_int addyIntensity21 0x0894;
u_int addyIntensity22 0x0898;
u_int addyIntensity23 0x089c;

  //global status addresses
u_int addyGlobalIdentificator 0x01;
u_int addyGlobalStatus 0x05;
*/

