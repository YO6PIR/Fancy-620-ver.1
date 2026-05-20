/*----------  EEPROM Data initialization  ---------------*/        
void band2eep(){
    Data = eprom.read(EEP_INIT); 
  if(Data != 73){                       // Iinitialization check Memory Blanc
    eprom.write(EEP_BAND, 2);  //Default Band = 2 [14MHz]

    xtalFreq = XTAL_FREQ;               //Default Xtall Frequency
    eprom.put(EEP_XTAL, xtalFreq);  //eprom.put(xtalFreq,EEP_XTAL);       //Write default Xtall Freq
    
    eep_romadd=0x010;                   // BAND:0 ROMadd:0x010
    eep_freq[0]=3710000;
    eep_freq[1]=3580000;   
    eep_scala=0;
    eep_agc=1;
    eep_fmode=0;
    eep_fstep=1;
    band2write();
  
    eep_romadd=0x020;                   // BAND:1 ROMadd:0x020
    eep_freq[0]=7100000;
    eep_freq[1]=7050000;   
    eep_scala=0;
    eep_agc=1;
    eep_fmode=0;
    eep_fstep=1;
    band2write();
  
    eep_romadd=0x030;                   // BAND:2 ROMadd:0x030
    eep_freq[0]=14200000;
    eep_freq[1]=14074000; 
    eep_scala=0;
    eep_agc=0;
    eep_fmode=1;
    eep_fstep=1;
    band2write();

    eep_romadd=0x040;                   // BAND:3 ROMadd:0x040
    eep_freq[0]=18100000;
    eep_freq[1]=18050000;    
    eep_scala=0;
    eep_agc=1;
    eep_fmode=1;
    eep_fstep=1;
    band2write();

    eep_romadd=0x050;                   // BAND:4 ROMadd:0x050
    eep_freq[0]=21200000;
    eep_freq[1]=21074000;   
    eep_scala=0;
    eep_agc=0;
    eep_fmode=1;
    eep_fstep=1;
    band2write();
  
    eep_romadd=0x060;                   // BAND:5 ROMadd:0x060
    eep_freq[0]=28400000;
    eep_freq[1]=28074000;    
    eep_scala=0;
    eep_agc=0;
    eep_fmode=1;
    eep_fstep=1;
    band2write();

    eep_rombadd=0x090;                  // BFO ROMadd:0x090
    eep_bfo[0]=7798000;                 //     LSB
    eep_bfo[1]=7802000;                 //     USB
    eep_bfo[2]=7800000;                 //     CW
    eep_bfo[3]=7800000;                 //     AM

    for (int i=0;i<4;i++){
      eprom.put((eep_rombadd+4*i),eep_bfo[i]);  //eprom.put(eep_bfo[i],(eep_rombadd+4*i));
    }
    eprom.write(EEP_INIT, 73);  //eprom.write(EEP_INIT,73); // Initialyzed End code
  }
}
/*----------  Function Band Write to EEPROM  --------------*/        
void band2write(){
  for (int i=0;i<2;i++){
    eprom.put((eep_romadd+4*i),eep_freq[i]);
  }
  eprom.write(eep_romadd+8,eep_scala);
  eprom.write(eep_romadd+10,eep_agc);
  eprom.write(eep_romadd+12,eep_fmode);
  eprom.write(eep_romadd+14,eep_fstep);
}
/*---------- Band data write to eeprom ----------*/
void bandwrite(){
  romadd=0x010+(band*0x010);
  eprom.put(romadd,freq[0]); 
  eprom.put(romadd+4, freq[1]); 
  eprom.write(EEP_BAND,band);
  eprom.write(romadd+8,flag_scala);
  eprom.write(romadd+10,flag_agc);
  eprom.write(romadd+12,fmode);
  if (fstep==1000){steprom=1;}     
  if (fstep==10){steprom=2;}
  if (fstep==100){steprom=3;}
  eprom.write(romadd+14,steprom);
}
