#include "libctp++.h"
// Original code is in eprexa:test
int evaluate(string& eval){
 // no brackets assumed in eval
 string ori(eval);
 int ll = eval.length();
 if(ll<1){
   printf("evaluate error: zero length string\n");
   return -1;
 }  
 //       
 int i=0;
 //cout << "   evaluate start:" << eval << endl;
 while(i<ll){
  // Negation
  if(eval[i]=='!'){
   // assuming no multiple !!!!!
   if(++i<ll && ((eval[i]=='0') || (eval[i]=='1'))){
     char buf[2];
     sprintf(buf,"%d",!atoi(&eval[i]));
     eval[i]=buf[0];
     eval.erase(i-1,1);
     ll--;i--;
   }else{
     printf("evaluate error: negation without argument: %s \n",eval.c_str());
     return -2;
   }  
  }
  i++;
  }
  //cout << "evaluate After Neg " << eval << endl;
  // AND
 i=0;
 while(i<ll){
  if(eval[i]=='&'){
   if(i>0 && ++i<ll && (eval[i]=='0' || eval[i]=='1')){
     char buf[2];
     sprintf(buf,"%d",atoi(&eval[i-2])*atoi(&eval[i]));
     eval[i-2]=buf[0];
     eval.erase(i-1,2);
     ll += -2; i+= -2;
   }else{
    printf("evaluate error: AND without argument: %s \n",eval.c_str());
    return -3;
   }
  }
  i++;
 }
  //cout << "evaluate After AND " << eval << " ll=" << ll << endl;
 // OR
 i=0;
 while(i<ll){
  if(eval[i]=='|'){
   if(i>0 && ++i<ll && (eval[i]=='0' || eval[i]=='1')){
     char buf[2];
     int p=atoi(&eval[i-2])+atoi(&eval[i]);
     if(p==2)p=1;
     sprintf(buf,"%d",p);
     eval[i-2] = buf[0];
     eval.erase(i-1,2);
     ll += -2; i+= -2;
   } else {
     printf("evaluate error: OR without argument: %s \n",eval.c_str());
     return -4;
   }  
  }
  i++;
 }
 //cout << "   evaluate end:" << eval << " l=" << ll << endl;
 if (ll != 1){
  printf("evaluate error: did not converge: %s \n",eval.c_str());
  return -5;
 } 
 else {
   printf("evalute: %s ret: %c \n",ori.c_str(),eval[0]);
   return eval[0];
 }  
 //else return atoi(&eval[0]);
}
//----------------------------------------------------------------
int parse(const char* desc,int level)
{
 string descr(desc);
 //cout << "parse level:" << level << ":'"  << descr <<"'" << endl;
 if(level>5) return -1;
 int ll=descr.length();
 //cout << "parse string length:" << ll << endl;
 if(ll==0){
  cout << "parse Error: empty string" << endl;
  return -1;
 }
 // remove spaces from beginning and end
 int i=0;
 if(!level){
 while(i<ll){
   if(descr[i]==' '){
    descr.erase(i,1);
    ll--;
   }else i++;
 }
 level++;
 ll=descr.length();
 //cout << "parse After removed spaces:'" << descr <<"'"<< endl;
 //cout << "parse string length:" << ll << endl;
 }
 //
 // Unfold brackets
 // 
 int rightparpos=0,leftparpos=0;
 int rightparnum,leftparnum=0;
 char val=0;
 i=0;
 string buf("");
 while(i<ll){
        //cout << " parse start ll=" << ll << " i=" << i << endl;
        //buf.clear();
	rightparnum=0;
 	while(i<ll && descr[i] != '('){  // first left bra
   		// evaluate
		if(descr[i] == ')')rightparnum++;
		buf.append(1,descr[i]);
   		i++;
 	}
	//cout <<buf << endl;
	if(rightparnum){
	 cout << "parse syntax Error: left bracket missing ?" << endl;
	 return -1;
	}
 	if(i==ll){
  	//cout << " parse Finished level:" << level <<endl;
	return evaluate(buf);
 	}
	// Found left bracket
	i++;
 	leftparpos=i;
	rightparnum=0;
 	leftparnum=1;
 	while(i<ll && leftparnum != rightparnum){ // loop until (=)	
		//cout << i << endl;
  		if(descr[i]=='(')leftparnum++;
  		if(descr[i]==')')rightparnum++;
		//cout << "leftparnum="<<leftparnum<<" rightparnum="<< rightparnum << endl;
  		i++;
 	}
	if(leftparnum != rightparnum){
	 cout << " parse Syntax error: right bracket missing ?" << endl;
	 return -1;
	}
 	rightparpos=i;
	//cout << "leftparpos="<<leftparpos<<" rightparpos="<< rightparpos << endl;
 	val=parse(descr.substr(leftparpos,rightparpos-leftparpos-1).c_str(),level++);
	if(val<0) return -1;
        //cout << " parse after ll=" << ll << " i=" << i << endl;
	buf.append(1,val);
 }
 //cout << "parse val:"<< val << endl;
 if(buf.length()){
  return (val=evaluate(buf));
 }else return -6;
}

