#include "ssmconnection.h"
//-----------------------------------------------------------------------
//             USER INTERFACE
//-----------------------------------------------------------------------
// Programs are grouped according to functions
// (previos version in ssmconui.c.modes was organised according to modes):
// Connections
// Past/Future
// Monitor
// Generate
// Logic
/*FGROUP Connections
 Algorithm(1):
  - write to generating board simple pattern: e.g.1000100010001...
  - start receiving board
  - start generating board
  - wait
  - stop all boards
  This fails often due to the wrong bits either at the begining of 
  recording in ssmssm or start of generation. 
  SO BEFORE CLAIMING WRONG CONNECTION CHECK IT. 
  You will see inapropriate bit at the beginning of ssm or at the begining
  of recording.
  To avoid this problem a new algoritm was introduced.
  Algoritm (2):
  - write to the generating board any pattern (even more complcated) BUT
    its length should be divider if the ssm size, i.e multiple of 2.
  - start generating ssm in continuous mode
  - start receiving board 
  - stop boards
  By this you avoid problem with the start of generation errors
*/
void README(){
}
/*FGROUP Connections
 * ------------------------------------------------------------------
 * Connections between FO -> LTUs.
 * FO2LTU() -- ltu1 = top connector -> first in parameters !!!
 * FO mode
 ltu = 11 : 0x811000 tested
     = 12 : 0x812000 testes
*/
int FOLTUConnection(int ltu){	
 if(ltu == 11)return A2BCD(3,"fo1_outgen","ltu1_i1",0);
 else if(ltu == 12) return A2BCD(3,"fo1_outgen","ltu2_i1",0);
 else return 0xffff; 
 //return A2BCD(4,"fo1_outgen","ltu1_i1","ltu2_i1",0); 
}	
/*FGROUP Logic
 * ------------------------------------------------------------------
 * FO L0/L1 logic
 * FO: generates PP,L0,L1,L1data
 * LTU: receives PP,L0,L1,L1data
 * FO2LTU() -- ltu1 = top connector -> first in parameters !!!
 * FO mode
 * RULE for FO->LTU CABLE CONNECTIONS:
 * the lowest LTU VME dial goes to top FO connector(=ltu1)
 * RULE for BUSY CABLE CONNECTIONS:
 * the ltu1 = 1st busy input
*/
int FOL0L1mode(){	
 return A2BCD(3,"fo1_igl0l1","ltu1_ipp",0); 
 //return A2BCD(4,"fo4_igl0l1","ltu1_ipp","ltu3_ipp",0); 
}	
/*FGROUP Logic
 * ------------------------------------------------------------------
 * FO L2 logic
 * FO: Generates L2strobe,L2Data1,L2Data2
 * LTU: receives L2strobe, L2data/
 * FO2LTU() -- ltu1 = top connector -> first in parameters !!!
 * FO mode
*/
int FOL2mode(){	
 return A2BCD(3,"fo1_igl2","ltu1_ipp",0); 
 //return A2BCD(4,"fo4_igl2","ltu1_ipp","ltu3_ipp",0); 
}
//
//-------------------------------L0 modes----------------------------
//
/*FGROUP Connections
 * ------------------------------------------------------------------
 * L0->FO connection
 * L0 generates: l0clstt,l0clst[6..1],l0strobe,l0data,pp,int[2..1]
 * F0 receives: l0clstt,l0clst[6..1],pp at input
 *              l0data,int are on other board
 *              pp[4..1],l0[4..1] at output
 *              
 * L0 mode             
*/
int L0FOconnection(){
 return A2BCD(3,"l0_outgen","fo1_inmonl0",0);	
}
/*FGROUP Connections
 * Different way of connection test:
  - generating board go to continuous
 * L0 mode
*/
int L0FOconnection2(){
 return A2BCD(3,"l0_outgen","fo1_inmonl0",4);
}
/*GROUP Connections
 * ------------------------------------------------------------------
 * L0->F01,FO2,FO3,FO4 connections
 * Check L0 - > mode FO boards connection
 * L0 mode
*/
int L0FOnconnection(){
 return A2BCD(4,"l0_outgen","fo1_inmonl0","fo3_inmonl0",4);	
}
/*GROUP Connection
 * ------------------------------------------------------------------
 * L0->F01,FO2,FO3,FO4 connections
 * Check L0 - > mode FO boards connection
 * L0 mode
*/
int L0FO2connectionTest(){
 return A2BCD(4,"l0_outgen","fo1_inmonl0","fo3_inmonl0",4);	
}
/*FGROUP Connections
 * ------------------------------------------------------------------
 * L0->L1 connection
 * L0 mode
 */
int L0L1connection(){
 return A2BCD(3,"l0_outgen","l1_inmon",0);	
}
/*FGROUP Connections
 * ------------------------------------------------------------------
 * L0->L2 connection
 * L0 mode
 */
int L0L2connection(){
 return A2BCD(3,"l0_outgen","l2_inmon",0);	
}
/*FGROUP Connections
 *------------------------------------------------------------------
 * L0->INT connections
 * L0 mode
 */
int L0INTconnection(){
 return A2BCD(3,"l0_outgen","int_inmon",0);	
}
/*FGROUP Logic
 * ------------------------------------------------------------------
 * L0 class logic NOT FINISHED
 * L0 generates: l0clstt,l0clst[6..1],l0strobe,l0data,pp,int[2..1]
 * F0 receives: l0clstt,l0clst[6..1],pp at input
 *              l0data,int are on other board
 *              pp[4..1],l0[4..1] at output
 * L0 mode     
*/
int L0ingener(){
 //return A2BCD(3,"l0_ingen","fo1_inmonl0",0);	
 return A2BCD(3,"l0_ingen","l1_inmon",0);	
}
/*FGROUP Monitor
 * ------------------------------------------------------------------
 *  L0 Output Monitor mode
 *  Start L0 in recording mode and read SSM to memory.
 *  L0 mode
 */  
int L0outmon(){
 return A2BCD(2,"l0_outmon",0);
}
/*FGROUP Monitor
 * ------------------------------------------------------------------
 *  L0 Input Monitor mode
 *  Start L0 in recording mode and read SSM to memory.
 *  L0 mode
 */  
int L0inmon(){
 return A2BCD(2,"l0_inmon",0);
}

/*FGROUP PastFuture
 * ------------------------------------------------------------------
 *  Output Monitor mode: testing L0 PF
 *  Input: almost all p/f settings
 *  Set HW according to input
 *  Examples with SB: 
 *            th=1,deltaT=4,sbc1=3,res=0
 *            th=1,deltaT=3,sbc1=6,res=1
 *            th=1,deltaT=2,sbc1=7,res=2
 *            th=1,deltaT=5,sbc1=10,res=1
 *  L0 mode
*/ 
int L0testPF(w32 tha1,w32 tha2,w32 thb1,w32 thb2,w32 deltaTa,w32 deltaTb,w32 sbc1,w32 resa,w32 resb,w32 lut12D){
 HardWare.ipf=1;
 HardWare.THa1=tha1;
 HardWare.THa2=tha2;
 HardWare.THb1=thb1;
 HardWare.THb2=thb2;
 HardWare.deltaTa=deltaTa;
 HardWare.deltaTb=deltaTb;
 HardWare.sbc1=sbc1;
 HardWare.scaleA=resa;
 HardWare.scaleB=resb;
 HardWare.delayA=0;
 HardWare.nodelayAf=1;
 HardWare.lut12D=lut12D;
 HardWare.luta=0xe;
 HardWare.lutb=0xe;
 HardWare.delayedINTlut=0x0;
 A2BCD(2,"l0_outmon",2);
 return 0;
}
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 *  L0 Output Monitor mode: testing L0 PF
 *  Input: ipf : circuit
 *  Read HW settings from HW !
 *  Examples with SB: 
 *            th=1,deltaT=4,sbc1=3,res=0
 *            th=1,deltaT=3,sbc1=6,res=1
 *            th=1,deltaT=2,sbc1=7,res=2
 *            th=1,deltaT=5,sbc1=10,res=1
 *  L0 mode
*/ 
int L0testPF2(int ipf){
 HardWare.ipf=ipf;
 getPFHW(1,ipf); //1=l0 board
 printPFHW();
 A2BCD(2,"l0_outmon",3);
 return 0;
}
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 *  L0 Output Monitor mode: testing L0 PF by random HW settings
 *  L0 mode
*/ 
int L0rndPF(int ipf){
 HardWare.ipf=ipf;
 //A2BCD(2,"l0_outmon",4);
 A2BCD(2,"l0_outmon",4);
 return 0;
}
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 *  Output Monitor mode: testing all 5 L0 PF by random HW settings
 *  L0 mode
*/ 
int L0rndallPF(){
 A2BCD(2,"l0_outmon",6);
 return 0;
}
/*FGROUP Logic
 * ------------------------------------------------------------------
 *  L0 class logic
 *  Classes generated by BC downscale or RND
 *  AJ version exists
 *  L0 mode
*/ 
int L0classlogic(){
 A2BCD(2,"l0_outmon",1);
 return 0;
}
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 * L1 P/F testing:
 * L0 generates int1/int2 in outgen mode by user defined sequence !
 * L1 outmon getting p/f
 * L2 inmon getting int1/int2
 * L0 mode
 */ 
int L1testPF2(int ipf){
 HardWare.ipf=ipf;
 getPFHW(2,ipf); //1=l0 board
 printPFHW();
 A2BCD(4,"l0_outgen","l2_inmon","l1_outmon",1);
 return 0;
}
/*FGROUP Logic
 *------------------------------------------------------------------
 * L1 class logic testing
 * L0 generates in L0 outgen
 * L1 inmon, L2 inmon
 * L0 mode
*/
int L1classlogic2(){
 return A2BCD(4,"l0_outgen","l1_inmon","l2_inmon",2);
}
//
//--------------------L1 modes---------------------------------------
//
/*FGROUP Logic
 * ------------------------------------------------------------------
 * check if necessary     
*/
int L1ingener(){
 //return A2BCD(3,"l0_ingen","fo1_inmonl0",0);	
 return 2;	
}

/*FGROUP Monitor
 * ------------------------------------------------------------------
 *  L1 Output Monitor mode
 *  Start L1 in recording mode and read SSM to memory.
 *  L1 mode
 */  
int L1outmon(){
 return A2BCD(2,"l1_outmon",0);
}
/*FGROUP Monitor
 * ------------------------------------------------------------------
 *  L1 Input Monitor mode
 *  Start L1 in recording mode and read SSM to memory.
 *  L1 mode
 */  
int L1inmon(){
 return A2BCD(2,"l1_inmon",0);
}
/*FGROUP Generate
 * ------------------------------------------------------------------
 * L1 Input genereting mode
 * L1 mode
*/ 
int L1ingen(){
 return A2BCD(2,"l1_ingen",0);
}
/*FGROUP Connections
 * ------------------------------------------------------------------
 * L1->L2 connection
 * L1 mode
 */ 
int L1L2Connect(){
 return A2BCD(3,"l1_outgen","l2_inmon",0);	
}
/*FGROUP Connections
 * ------------------------------------------------------------------
 * L1-> FO connection
 * L1 mode
 */ 
int L1FOconnection(){
 return A2BCD(3,"l1_outgen","fo1_inmonl1",0);	
}
/*FGROUP Connections
 * L1-> FO connection
 * L1 mode
 */
int L1INTconnection(){
 return A2BCD(3,"l1_outgen","int_inmon",0);	
}
/*FGROUP Logic
 * L2 class logic testing
 * L1 outgen generates l1 strobe and l1data
 * L2 outmon monitors l2 strobe and l2 data
 * L1 mode
 */
int L2classlogic(){
 return A2BCD(3,"l1_outgen","l2_outmon",1);
}
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 * L1 P/F testing:
 * L0 board generating int1,int2 by BC down or RND
 * L0 ssm outmon giving int2,int1
 * L1 ssm outmon giving pf
 * none mode
 */ 
int L1testPF3(int ipf){
 HardWare.ipf=ipf;
 getPFHW(2,ipf); //1=l0 board
 printPFHW();
 A2BCD(4,"none_none","l0_outmon","l1_outmon",1);
 return 0;
}
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 * L1 P/F testing:
 * L0 board generates int1/2 by BC down or RND
 * L1 board ssm gives inta/b/d -> pf1
 * All analysis on L1 board ssm
 * L1 mode
 */ 
int L1testPF4(int ipf){
 HardWare.ipf=ipf;
 getPFHW(2,ipf); //1=l0 board
 printPFHW();
 A2BCD(2,"l1_outmon",1);
 return 0;
}
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 * L1 P/F testing:
 * L0 board generates int1/2 by BC down or RND
 * L1 board ssm gives inta/b/d -> pf1
 * All analysis on L1 board ssm
 * L1 mode
 */ 
int L1rndallPF(){
 A2BCD(2,"l1_outmon",6);
 return 0;
}
/*FGROUP Logic
 * L1 class logic testing
 * L1 ingen generates l0strobe and l0 data
 * L2 unmon monitors l1 strobe l1 data
 */
int L1classlogic(){
 return A2BCD(3,"l1_ingen","l2_inmon",1);
}
//
//--------------------------------------L2 modes---------------------
//
/*FGROUP Connections
 * ------------------------------------------------------------------
 * L2->FO connection
 * L2 mode
 */ 
int L2FOconnection(){
 return A2BCD(3,"l2_outgen","fo1_inmonl2",0);	
}
/*FGROUP Connections
 * L2->INT connection
 * L2 mode
 */
int L2INTconnection(){
 return A2BCD(3,"l2_outgen","int_inmon",0);	
}
/*FGROUP Logic
 *------------------------------------------------------------------
 * Testing interface board
 * L2 generates l2a in outgen mode. 
 * The correct content of the l2 ssm should be recorded in advance
   It is not going to work:
                    - generation is not stopped when ctp_busy
                    - I cannot enable dll just after ssm start due to sw
   I go via macro
 * L2 mode
 */
int L2INT(){
 return A2BCD(3,"l2_outgen","int_inmon",1);
}
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 * L2 P/F testing:
 * L0 board generates int1/2 by BC down or RND
 * L2 board ssm gives inta/b/d -> pf1
 * All analysis on L2 board ssm
 * L2 mode
 */ 
int L2rndallPF(){
 A2BCD(2,"l2_pf",6);
 return 0;
}
/*FGROUP Logic
 *-------------------------------------------------------------------
 * L2 class logic testing
 * L2 ingen 
 * FO inmon
 * this can be done via script also
*/
int L2classlogic2(){
 //return A2BCD(3,"l2_ingen","fo1_inmon",1);
 return 0;
}
//
//---------------------- none modes----------------------------------
//
/*FGROUP PastFuture
 * ------------------------------------------------------------------
 * L2 P/F testing:
 * L0 board generating int1,int2 by BC down or RND
 * L0 ssm outmon giving int2,int1
 * L2 ssm outmon giving pf
 * none mode
 */ 
int L2testPF3(int ipf){
 HardWare.ipf=ipf;
 getPFHW(3,ipf); //1=l0 board
 printPFHW();
 A2BCD(4,"none_none","l0_outmon","l2_pf",1);
 return 0;
}
/*FGROUP Monitor
 * ------------------------------------------------------------------
 * Monitor : L0 outmon, L1 inmon,L2 inmon
 * none mode
 */ 
int L0outL1inL2in(){
 A2BCD(5,"none_none","l0_outmon","l1_inmon","l2_inmon",2);
 return 0;
}
/*FGROUP Monitor
 * ------------------------------------------------------------------
 * Monitor : L0 outmon, L1 outmon,L2 outmon
 * none mode
 */ 
int L0outL1outL2out(){
 A2BCD(5,"none_none","l0_outmon","l1_outmon","l2_outmon",2);
 return 0;
}
/*FGROUP Monitor
 * ------------------------------------------------------------------
 * Monitor : L0 outmon, L1 outmon,L2 outmon,FO
 * L0 mode
 */ 
int L0outL1outL2outFOLTU(){
 //A2BCD(7,"none_none","l0_outmon","l1_outmon","l2_outmon","fo1_inmonl1","ltu1_ipp",3);
 A2BCD(6,"l0_outmon","l1_outmon","l2_outmon","fo1_inmonl1","ltu1_ipp",7);
 return 0;
}
/*FGROUP Monitor
 * ------------------------------------------------------------------
 * Monitor : L0 outmon, L1 outmon,L2 outmon,Busy
 * none mode
 */ 
int L0outL1outL2outBusy(){
 //A2BCD(6,"none_none","l0_outmon","l1_outmon","l2_outmon","busy_inmon",2);
 //while(!A2BCD(6,"none_none","l0_outmon","l1_outmon","l2_outmon","busy_outmon",2));
 (A2BCD(6,"none_none","l0_outmon","l1_outmon","l2_outmon","busy_outmon",2));
 return 0;
}

/*FGROUP Monitor
 * ------------------------------------------------------------------
 * Monitor : L0 inmon, busy outmon
 * none mode
 */ 
int L0inBUout(){
 A2BCD(4,"none_none","l0_inmon","busy_outmon",2);
 return 0;
}
/*FGROUP Monitor
 * ------------------------------------------------------------------
 * Monitor all PF at L0,L1,L2
 * none mode
 */ 
int MonallPF(){
 A2BCD(5,"none_none","l0_outmon","l1_outmon","l2_pf",2);
 return 0;
}
/*FGROUP Monitor
 * -----------------------------------------------------------------
 * L2 monitor mode on FO
 * none mode
 */
int FOL1mon(){
 return A2BCD(4,"none_none","fo1_inmonl1","fo3_inmonl1",2);
}
/*FGROUP Monitor
 *none mode
*/
int FOLTU(){
 return A2BCD(4,"none_none","fo1_inmonl1","ltu1_i1",2); 
}
/*FGROUP Logic
 *-------------------------------------------------------------------
 * L1 outmon for int1,int2
 * L2 outmon for L2data1,L2data2
 * INT outmon for CTP readout
 * Compares L2 serial data woth CTP readout
 * interaction record may be added
 * none mode
*/
int L2a2INT(){
 return A2BCD(5,"none_none","l1_inmon","l2_outmon","int_ddldat",4);
}
/*FGROUP Logic
 * Dump Interface SSM to file IntDump.txt
 */
int dumpINTSSM(){
 return A2BCD(3,"none_none","int_ddldat",5);
}
/*FGROUP Logic
 * Create INT list and save it to IntList.txt
 */
int checkORBIT(){
 int ret=1;
 while(ret)ret=!A2BCD(3,"none_none","int_ddldat",6);
 return 0;
}
//
//----------------------BUSY modes-------------------------------------------
//
/*GROUP Logic
 *-------------------------------------------------------------------
 * L1 logic test
 * busy generetes busy = nobusy
 * L0 generates l0 triggers by BC/RND
 * L1 outmon monitors them
 * busy mode
 * 
int L1logic(){
 return A2BCD(3,"busy_outgen","l1_outmon",1);
}
*/
/*FGROUP Connections
 * BUSY connections
 * busy mode
 */ 
int BUconnect(){
 return A2BCD(3,"busy_outgen","l0_inmon",0);
}
//
//-------------------Interface modes------------------------------------------
//
/*FGROUP Connections
 * INT->L0 connection (only ctp_busy)
 * int mode
 */
int INTL0connection(){
 printf("NOt implemented: int board under development. \n");
 //return A2BCD(3,"int_outgen","busy_outmon",0);
 return 1;
}
