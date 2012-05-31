int main(){
 double s,n,moc,prob,x;
 int i,nev,N;
 prob=1./400.;
 N=800;
 n=N;
 nev=8;
 moc=1.;
 for(i=0;i<N;i++)moc=moc*(1.-prob);
 s=moc;
 x=1;
 for(i=0;i<nev;i++){
  moc=moc/(1.-prob);
  x=x*((n-i)*prob)/(i+1.);
  s=s+x*moc;
 }
 printf("s=%f \n",s);
}
