/*#######################################################################    
#       ___    ___ _______    _______   _______   ___ _______           #
#       \   \ /  //   __   \ /   ____) |    __  \|   |    __  \         #
#        \   /  /|   |  |   |   |____  |   |__)  |   |   |__)  |        #
#         \    / |   |  |   |    ___  \|    ____/|   |       _/         # 
#         |   |  |   |__|   |   (___)  |   |     |   |   |\   \         #
#         |___|   \_______ / \________/|___|     |___|___| \___\        # 
#                                                                       #   
#########################################################################    
           
 *             Transceiver controller FANCY-620 Ver6 06.06.2025
 *                  https://www.qsl.net/yo6pir/fancy620
 *    Processor:    STM32F103CBT6, 64KB Flash, 20KB RAM, EEprom 24C02
 *    VFO-DDS:      Si5351, VFO, BFO, IF-SHIFT, RIT
 *    Display:      ILI9341 SPI
 *    LCD Library:  Ucglib.h -HWSPI
 *    MEMORY:       EEProm 24C02 - 2kB RAM; 89% Flash fill
 *    Based on 'JAN2KD 2016.10.19 Multi Band DDS VFO Ver3.1 and JA2GQP-2020
 *    Functionality included:
 *          - 6 benzi de frecventa comutabile...............[Key1] scurt
 *          - Comutare VFO A<->B ...........................[Key1] lung
 *          - Ajustare BFO pe fiecare domeniu ........[Key2] apasata la pornire
 *          - Moduri de lucru LSB, USB si CW ...............[Key2] scurt
 *          - AGC ON/OFF....................................[Key2] lung
 *          - ATT ON/OFF................................... [Key3] lung
 *          - AMPLI +20dB ON/OFF........................... [Key3] scurt
 *          - RIT ON/OFF....................................[Key4]scurt 
 *          - Scala liniara pe centru ecranului ON/OFF..... [Key4]lung 
 *          - LOCK KNOB ....................................[ENC_BUT] lung   
 *          - STEP x10, x100, x1K...........................[ENC_BUT] scurt
 *          - Ajustare XTALL oscilator ............[ENC_BUT] apasat la pornire
 *          - iesire de comutare in cod BCD pe CD4028 decoder
 *          - reglaj RIT din encoder cu scala mica liniara pe colt ecran 
 *          - La schimbare pas, rotunjeste VFO cu valoarea STEP
 *          - Salveaza automat dupa 2s de la schimbare frecventa, mode sau banda
 *          - Bargraf analogic 
 *          - Mesaje de interfa in partea de jos a ecranului pe ultima linie 
 *          - Tensiune de alimentare monitorizata pe ecran Ualim < 19V
 *          - Masurare de temperatura Celsius cu Thermistor de 10K
 *          - Pe emisie CW se genereaza ton de 700Hz + FREQ_VFO          
 *          - VFO = CLK0, BFO = CLK1
 ****************************************************************************/
#include <Wire.h>                 
#include <SPI.h>
#include "src/Rotary.h"                 
#include "src/Ucglib.h"
#include <at24c02.h>
/* Create a eprom object configured at addrfess 0
 Sketch assumes that there is an eprom present at this address*/
AT24C02 eprom(AT24C_ADDRESS_0);
/* Create another eprom object configured att address 2
Sketch assumes that there is NO eprom present at this address*/
AT24C02 badEprom(AT24C_ADDRESS_2);

//#define TEST            0              //uncomment for background tests
/*----------   Encorder setting  ---------------*/
#define ENC_A     PB12                    // Rotary encoder A
#define ENC_B     PB13                    // Rotary encoder B
Rotary Rot = Rotary(ENC_A,ENC_B);
/*----------   TFT setting  -------------------*/ 
#define   __CS    PB10                    // CS    
#define   __DC    PB0                     // D/C
#define   __RST   PB1                     // RESET   
Ucglib_ILI9341_18x240x320_HWSPI ucg(__DC, __CS, __RST);
/*----------   CW Tone  -------------------*/ 
#define   CW_TONE     700                // 700Hz
/*----------   I/O Assign  ------------------*/ 
#define   MODE_OUT1    PB15    //Iesire1 in cod BCD pt MODE                            
#define   MODE_OUT2    PA8     //Iesire2 in cod BCD pt MODE                        
#define   BAND_OUT1    PB3     //
#define   BAND_OUT2    PB4     //--Iesiri in cod BCD pt BANDS(CD4028 Decoder) 
#define   BAND_OUT3    PB5     //
#define   SW_BAND      PA0     //Buton de schimbare benzi            
#define   SW_MODE      PC14    //Buton de schimbare MODE[LSB,USB,CW,AM]             
#define   SW_STEP      PB14    //Buton Encoder             
#define   SW_RIT       PC15    //Buton RIT             
#define   SW_TX        PC13    //INPUT PTT sense             
#define   SMETER       PA1     //INPUT S-meter analog 
#define   VOLT         PA3      //INPUT Voltage Battery sense
#define   TEMP         PA2      //INPUT Temperature Thermistor 10k 
#define   SW_AMP       PA4      //Buton AMP/ATT
#define   AMP_OUT      PA10     //OUTPUT AMP signal[ON/OFF]
#define   ATT_OUT      PA9     //OUTPUT ATT signal [ON/OFF]
#define   AGC_OUT      PA15     //OUTPUT AGC signal[ON/OFF]                
#define   EEP_BAND     0x00         // EEProm BAND Adress
#define   EEP_XTAL     0x08         // EEProm Xtall Adress
#define   EEP_INIT     0x0e         // INIT end Adress
#define   XTAL_FREQ    27000000     // Default Crystal frequency 27MHz
/********************Definitii culori ****************************/
#define WHITE           255,255,255
#define BLUE            0,0,255
#define CYAN            0,255,255
#define YELLOW          255,255,0
#define GRAY            190,190,190
#define RED             255,0,0
#define GREEN           0,255,0
#define ORANGE          255,192,0

#ifdef TEST
#define backcolor       200,200,200
#else
#define backcolor       0,0,0
#endif

#define SHORT_PRESS_THRESHOLD   300                 //aprox~1sec
#define MAXBANDS                6                   //Nr maxim de benzi 
/*************  Definitii Senzor Temperatura    */
#define BETA    3950        //coeficientul Beta al termistorului
#define R_NOMINAL 10020      //rezistenta in Ohmi la 25C
#define ROOM_TEMP 298.15    //Temperatura la 25K
#define R_BALANCE 9940      //rezistenta divizorului serie de tensiune
#define ADC_MAX   4096.0    //Val Max ADC

/*---------- Variable setting ----------*/
long Flim[MAXBANDS][2] = { {3500000, 3800000}, //Limitele de frecventa ale benzilor
                            {7000000, 7200000}, 
                            {14000000, 14350000}, 
                            {18050000, 18200000}, 
                            {21000000, 21500000},
                            {28000000, 29000000}};
int       cur_vfo = 0;                      //Default 0 = VFO-A; 1 = VFO-B
long      freq[2];                          //cele doua frecvente afisate 
long      romf[4];                          // Buffer de citire Freq din EEprom
long      freqold = 1;
int       freq_ifshift = 0;
int       freq_ifshift_old = 0;
int       freq_rit, freq_rit_old;
String    freq_str = String(freq[cur_vfo]);   //Frequency text
long      vfofreq = 0;
long      vfofreqb;                 
long      cio = 0;                            //variabila care tine BFO
long      old_cio;                            //variabila temporara BFO
long      romb[5];                            // EEPROM bfo copy buffer
char      f100m,f10m,fmega,f100k,f10k,f1k,f100,f10,f1;
int       fstep = 100;
uint16_t  steprom;                            //Pasul citit din EEprom [int]
uint16_t  fmode;                              //MODE citit din EEprom
uint16_t  scalarom;                           //Flag SCALA citit din EEprom
uint16_t  agcrom;                             //Flag AGC citit din EEprom
bool      init_flag;                          //Flag de pornire PLL
bool      flag_rit=0;
bool      flag_lock;
bool      flag_att = 0;
bool      flag_amp = 0;
bool      flag_call=0;
bool      flag_bfocall;
int       flag_agc = 0; 
bool      flag_frqwt = 0;                    // Frequency data Wite Flag(EEPROM)
bool      flag_bfoadj = 0;                   // BFO Wite Flag(EEPROM)
bool      flag_xtalladj = false;                 // Frequency ADJ Flag
bool      flag_step = 0;                    //Flag STEP ACT
bool      message = 0;
int       flag_scala;
int       meter_value  = 0;
int       romadd     = 0;                   //adresa mem care tine VFO curent
int       key = 0;
int       adc_v;
uint16_t          band;                   
uint16_t          Status;
uint16_t          Stare;
uint16_t          Data;          
uint32_t          xtalFreq;
unsigned long     eep_freq[4];
int               eep_romadd;
int               eep_fstep;
int               eep_fmode;
int               eep_scala;
int               eep_agc;
unsigned long     eep_bfo[6];
int               eep_rombadd;
long              freqb = 0;

/*Volts measurement variables*/
double    v1;                               //varriabila care tine tensiunea afisata VOLT

int prev_freq = freq_ifshift;  // Frecvența anterioară
bool start_with_small = false; // Flag pentru alternarea începutului scării
 
int_fast32_t timepassed;                    // int to hold the arduino miilis since startup
int_fast32_t runseconds10msg = 0;           //cronometru pentru afisare mesaj
int_fast32_t runseconds10volts = -50;       //cronometru afisare Temp/Volt
unsigned long buttonPressStartTime = 0;     // Momentul in care a fost apasata o tasta
bool key1pressed = false;                   // Variabila pentru confirmare daca Tasta1 a fost apasata 
bool key2pressed = false;                   // Variabila pentru confirmare daca Tasta2 a fost apasata 
bool key3pressed = false;                   // Variabila pentru confirmare daca Tasta3 a fost apasata 
bool key4pressed = false;                   // Variabila pentru confirmare daca Tasta4 a fost apasata 
bool key5pressed = false;                   // Variabila pentru confirmare daca Tasta5 a fost apasata  
/******************************************************************************************************
----------------------------------  Initialization  Program  ------------------------------------------
*******************************************************************************************************/ 
void setup() {
  adc_calibrate(ADC1);                            //ADC Module calibration
  timepassed = millis();
  afio_cfg_debug_ports(AFIO_DEBUG_NONE);          // ST-LINK(PB3,PB4,PA15,PA12,PA11) Can be used      
  Wire.begin();                   
  pinMode( ENC_A,INPUT_PULLUP);                   // PC13 pull up
  pinMode( ENC_B,INPUT_PULLUP);                   // PC14
  /*Activeaza intreruperile pentru pini encoder*/
  attachInterrupt( ENC_A, Rotary_encoder_isr, CHANGE);    // Encorder A
  attachInterrupt( ENC_B, Rotary_encoder_isr, CHANGE);    //          B

  delay(100);
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.clearScreen();
  ucg.setRotate90();

  pinMode(SW_BAND,INPUT_PULLUP);
  pinMode(SW_MODE,INPUT_PULLUP);
  pinMode(SW_STEP,INPUT_PULLUP);
  pinMode(SW_RIT,INPUT_PULLUP);
  pinMode(SW_TX,INPUT_PULLUP);
  pinMode(ENC_A,INPUT_PULLUP);                   
  pinMode(ENC_B,INPUT_PULLUP);                    
  pinMode(SW_AMP,INPUT_PULLUP); 
  pinMode (BAND_OUT1,OUTPUT);
  pinMode (BAND_OUT2,OUTPUT);
  pinMode (BAND_OUT3,OUTPUT);
  pinMode(MODE_OUT1,OUTPUT);
  pinMode(MODE_OUT2,OUTPUT);
  pinMode(AMP_OUT,OUTPUT);
  pinMode(ATT_OUT,OUTPUT);
  pinMode(AGC_OUT,OUTPUT);
  
  band2eep();
  delay(100);

  eprom.get(EEP_XTAL, xtalFreq);    //read Frecv Xtall from memory
  band = eprom.read(EEP_BAND);  

  romadd = 0x010+(band*0x10);
  for (int i=0; i<3;i++){
  eprom.get((romadd+4*i),romf[i]);  
  }
  for( int i=0; i<2; i++){
   freq[i]=romf[i];
  }
  scalarom = eprom.read(romadd+8);
  agcrom = eprom.read(romadd+10);
  fmode = eprom.read(romadd+12);
  steprom = eprom.read(romadd+14);
  /**********************************/
  eep_rombadd = 0x090;                             // EEPROM read(BFO)
  for (int i=0; i<4;i++){
    eprom.get((eep_rombadd+(4*i)),romb[i]);  
    eep_bfo[i] = romb[i];    
  }
 
  if (steprom==1){fstep=1000;}                      // STEP set
  if (steprom==2){fstep=10;}
  if (steprom==3){fstep=100;}
  flag_scala = scalarom;
  flag_agc = agcrom;  
  if(digitalRead(SW_STEP) == LOW)flag_call=1;      //la pornire se poate activa CalibrationXtall cu STEP apasat
  if(digitalRead(SW_RIT) == LOW)flag_bfocall=1;    //la pornire se poate activa CalibrationBFO cu SHIFT apasat  
  init_screen();
}
/******************************************************************************************************
-----------------------------------------  Main program  ----------------------------------------------
*******************************************************************************************************/ 
void loop() {
  bargraf();
  key = get_keys();     //citeste tastele + encoder_button  
    key_bands();        //verifica tasta-1 BANDS/VFO
    key_mode();         //verifica tasta-2 MODE/AGC
    key_amp_att();      //verifica tasta-3 AMP/ATT
    key_shift();        //verifica tasta-4 SHIFT/SPLITT
    key_enc_button();   //verifica tasta-5 ENCODER/SETTINGS  
      
  if (digitalRead(SW_TX)==LOW)                  // TX sw check
    txset();

  if(flag_rit){                     /*daca este activat IF-Shift*/
    if (freq_ifshift != freq_ifshift_old){      /*Anti Flicker IF-SHIFT*/
      PLL_write();
      show_scara_mica(flag_rit);
    }
  }
  
    if(!flag_xtalladj){
      if(freq[cur_vfo] != freqold){        /*Anti-Flicker freq[cur_vfo]*/
        PLL_write();
            if(flag_scala) 
              show_scara_mare();
            else 
              show_frequency1(freq[cur_vfo]); 
       flag_frqwt = 1;                                // EEP Wite Flag
        timepassed = millis();    
      }
    }
    else{/*Daca este activat Xtall-Adjust*/
        xtalFreq = freq[cur_vfo];
        si5351aSetFrequency(10000000);        
        show_frequency1(freq[cur_vfo]); 
        freqb = freq[cur_vfo];
    }
/*----------- EEprom Auto-Save in 10 sec -------------------- */
  if((flag_frqwt) && (!flag_bfoadj)&&(!flag_xtalladj)){                     
    if(timepassed+1000 < millis()){
     bandwrite();
      flag_frqwt = 0;
     //  show_msg("          Save to memory...", 0);
    } 
  }
/*  Afiseaza mesaj permanent cu un refresh de 2sec in functie de message */
if( millis() > (runseconds10msg + 1000) && message)
    {
     show_msg("  Fancy620 HF TRANSCEIVER by YO6PIR", 0);
      runseconds10msg = millis();
     message = 0;
    } 
   
    /*Refresh la 3sec VOLTS and TEMPERATURE measurement*/
    if(millis() > runseconds10volts + 3000)
    {
      show_voltage();
      show_temperature();
      runseconds10volts = millis();
    }
}
/*---------- Encoder Interrupt -----------------------*/
void Rotary_encoder_isr(){
  if(!flag_lock){
  if (flag_rit){  /* Daca este activat reglaj IF-SHIFT...*/
    unsigned char result = Rot.process();
    if(result) {
      if(result == DIR_CW){  
        freq_ifshift += 10;
        if (freq_ifshift >= 300) freq_ifshift = 300;
      }
      else {
      freq_ifshift -= 10; 
      if (freq_ifshift <= -300) freq_ifshift = -300;
      }
    }
   }
  else{ /* daca NU este activat IF-Shift...*/ 
    unsigned char result = Rot.process();
    if(result) {
      if(result == DIR_CW){  /* Rotire DREAPTA*/
        freq[cur_vfo] += fstep;
        if((!flag_bfoadj) && (!flag_xtalladj) && (freq[cur_vfo] >= Flim[band][1])) 
        freq[cur_vfo]=Flim[band][1];
      }
      else{ /* Rotire STANGA*/
        freq[cur_vfo] -= fstep; 
        if((!flag_bfoadj) && (!flag_xtalladj) && (freq[cur_vfo]<=Flim[band][0]))          //freqmin 
        freq[cur_vfo]=Flim[band][0];    
      }  
    freq[cur_vfo] = rounding(freq[cur_vfo]);    //rotunjeste valoarea in functie de step
    }     
  }
 }
}
/* Routine for rounding the adjusted value to the fraction of the selected step */
long rounding(long f){
  double fractia = f / fstep;
  double fractia2 = f % fstep;
      if(fractia2 != 0){ 
        return f = fractia * fstep;  
      }
  return f;
}            
/*************************  Citeste tastele **************************************************/
int get_keys(void){
  if(digitalRead(SW_BAND) == LOW)       return 1;
  else if(digitalRead(SW_MODE) == LOW)  return 2;
  else if(digitalRead(SW_AMP) == LOW)   return 3;         
  else if(digitalRead(SW_RIT) == LOW)   return 4;
  else if(digitalRead(SW_STEP) == LOW)  return 5;
  return 0;
}
/************************************Tasta-1 BANDS/VFO[A-B] ***************************************/
void key_bands(){
 
  if((!flag_call)&&(!flag_bfocall)&&(key==1)&&(!key1pressed)){ //Tasta 1 este apasata dar nu a fost confirmata inainte...    
        delay(100);                           //Debounce button     
        if(key==1){                           //Tasta este inca apasata?
        buttonPressStartTime = millis();    // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
        key1pressed = true;                 //confirma ca tasta 1 a fost apasata  
      }
    }
        if((!flag_call)&&(!flag_bfocall)&&(key==0) && key1pressed){//daca nu este apasata nicio tasta dar se confirma Tasta1...    
    
       unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
                
        if(pressDuration < SHORT_PRESS_THRESHOLD) 
        {/* Daca timpul apasarii este mai mic de 1 secunda, consideram ca a fost o apasare scurta*/
            band++;
            if(band > (MAXBANDS-1))band=0;  
            if(flag_rit){
              flag_rit=0;
              freq_ifshift=0;
              show_scara_mica(0);                      
            }
            if(flag_scala) {
               ucg.setColor(backcolor);
               ucg.drawBox(32,88,241,38);                      //fundal scara mare
               ucg.drawBox(8,88,39,16);                        //sterge fundal  "VFO" de pe pozitie 
               show_scara_mare();                              //Afiseaza Scala Mare                 
             }
        }     
        else {/* Daca timpul apasarii este mai mare de 1 secunda, consideram ca a fost o apasare lunga*/          
            cur_vfo ^= 1;                                                
        }
        /* La orice apasare de tasta scurt/lung, executa.....*/
            romadd = 0x010+(band*0x010);            
            for (int i=0; i<3;i++){
              eprom.get((romadd+4*i),romf[i]);  
            }            
            freq[cur_vfo]=romf[cur_vfo];
            freq[!cur_vfo]=romf[!cur_vfo];
            agcrom=eprom.read(romadd+10);            
            fmode=eprom.read(romadd+12);
            steprom=eprom.read(romadd+14);  
            flag_agc=agcrom; show_agc(flag_agc);       
            if(steprom==1)fstep=1000;          
            if(steprom==2)fstep=10;
            if(steprom==3)fstep=100;
            
        if(flag_rit)flag_rit = 0;           //Anuleaza RIT la orice schimbare de banda, daca era activat inainte,                  
        show_mode();
        show_step();    
        show_band_index();
        bands_sw_out();                
        show_frequency2(cur_vfo);

        key1pressed = false;                  // Resetam starea butonului
        buttonPressStartTime=0;               //reset contor durata
    } 
}
/*********************Tasta-2 MODE[LSB,USB,CW,AM]/AGC[LOW-FAST] ***************************************/
void key_mode(){
 
  if((key==2) && (!key2pressed))              //Tasta 1 este apasata dar nu a fost confirmata inainte...
    {
      delay(100);                             //Debounce button     
      if(key==2){                             //Tasta este inca apasata?
        buttonPressStartTime = millis();      // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
        key2pressed = true;                   //confirma ca tasta 2 a fost apasata  
        }
    }
        if(key==0 && key2pressed)             //daca nu este apasata nicio tasta dar se confirma Tasta1...    
    {
       unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
      
        if(pressDuration < SHORT_PRESS_THRESHOLD) 
        {/* Daca timpul apasarii este mai mic de 1 secunda, consideram ca a fost o apasare scurta*/
         fmode++;
         show_mode();
         PLL_write();
        }     
        else {/* Daca timpul apasarii este mai mare de 1 secunda, consideram ca a fost o apasare lunga*/
          flag_agc ^= 1;                           //toggle agc mode
          show_agc(flag_agc);   
           flag_frqwt = 1;                                // EEP Wite Flag
           timepassed = millis();                
        }
        key2pressed = false;                  // Resetam starea butonului
        buttonPressStartTime=0;               //reset contor durata
    } 
}
/******************************** Tasta-3 AMP[ON-OFF]/ATT[ON-OFF] **************************************/
void key_amp_att(){
 
  if((key==3) && (!key3pressed))              //Tasta 3 este apasata dar nu a fost confirmata inainte...
    {
      delay(100);                             //Debounce button     
      if(key==3){                             //Tasta este inca apasata?
      buttonPressStartTime = millis();      // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
      key3pressed = true;                   //confirma ca tasta 3 a fost apasata  
      }
    }
        if(key==0 && key3pressed)             //daca nu este apasata nicio tasta dar se confirma Tasta1...    
    {
       unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
      
        if(pressDuration < SHORT_PRESS_THRESHOLD) 
        {/* Daca timpul apasarii este mai mic de 1 secunda, consideram ca a fost o apasare scurta*/
          flag_amp^=1;    //toggle Amp
          switch_amp(flag_amp);
        }     
        else {/* Daca timpul apasarii este mai mare de 1 secunda, consideram ca a fost o apasare lunga*/
         flag_att ^= 1;                      //toggle att  
         switch_att(flag_att);     
        }
        key3pressed = false;                  // Resetam starea butonului
        buttonPressStartTime=0;               //reset contor durata
    } 
}
/****************************** Tasta-4 SHIFT[ON-OFF] **********************************/
void key_shift(){
 
  if((!flag_call)&&(key==4) && (!key4pressed))              //Tasta 4 este apasata dar nu a fost confirmata inainte...
    {
      delay(100);                             //Debounce button     
      if(key==4){                             //Tasta 4 este inca apasata?
        buttonPressStartTime = millis();      // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
        key4pressed = true;                   //confirma ca tasta 4 a fost apasata  
      }
    }
       if((!flag_call)&&(key==0) && key4pressed)             //daca nu este apasata nicio tasta dar se confirma Tasta1...    
    {
       unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
      
        if(pressDuration < SHORT_PRESS_THRESHOLD) 
        {/* Daca timpul apasarii este mai mic de 1 secunda, consideram ca a fost o apasare scurta*/
              if(freq_ifshift==0){
                flag_rit ^=1;    
              }
              else {
                freq_ifshift=0;PLL_write();
              }
              show_scara_mica(flag_rit);
        }     
        else {/* Daca timpul apasarii este mai mare de 1 secunda, consideram ca a fost o apasare lunga*/             
        /*************************** Start BFO adjust ***************/
            if((flag_bfocall)&&(flag_scala))flag_bfocall=0;
              
            if((!flag_bfoadj)&&(flag_bfocall)&&(!flag_call)){
                romadd=0x010+(band*0x10);
                eprom.get(romadd, romf[0]);  
                freqold=freq[cur_vfo];
                eprom.get(0x090+(fmode * 4),freq[cur_vfo]);  
                     
                show_frequency1(freq[cur_vfo]);
                flag_bfoadj = 1;
                show_msg(" BFO [<->]Adjust.[Keep STEP]to Save ",1);
            }
            else if ((flag_bfoadj)&&(flag_bfocall)&&(!flag_call)){
                cio = freq[cur_vfo];
                eprom.put(0x090+(fmode * 4), cio);  
                eep_bfo[fmode] = cio;
                freq[cur_vfo] = romf[0];
            
                show_frequency1(freq[cur_vfo]);
                flag_bfoadj = 0;
                flag_bfocall=0;
                show_msg("          Save to memory...", 0);
            }
            /******************************************************/ 
            if(!flag_bfocall){ 
              flag_scala ^=1;
              
             if(flag_scala) {
               ucg.setColor(backcolor);
               ucg.drawBox(32,88,241,38);                      //fundal scara mare
               ucg.drawBox(8,88,39,16);                        //sterge "VFO" de pe pozitie 
               show_scara_mare();                              //Afiseaza Scala Mare  
             }
             else{
              show_frequency1(0);                               //Dump-buffer   
              delay(10);
              show_frequency1(freq[cur_vfo]);
               ucg.setColor(backcolor);
              ucg.drawBox(8,88,39,16);                        //sterge "VFO" de pe pozitie 
              ucg.setFont(ucg_font_fub11_tr); 
              ucg.setColor(WHITE);
              ucg.setPrintPos(8,100);  
              ucg.print("VFO");
            }
          } 
            flag_frqwt = 1;                                // EEP Wite Flag
            timepassed = millis();      
        }
        /**  Executa daca oricare mod de apasare este activ SHORT/LONG*************/
        key4pressed = false;                        // Resetam stare tasta 4
        buttonPressStartTime=0;                     //reset contor durata
    } 
}
/*********************************** Tasta-5 ENCODER[STEP]/SETTINGS ***********************************/
void key_enc_button(){
   if((key==5) && (!key5pressed))            //Tasta 5 este apasata dar nu a fost confirmata inainte...
    {
      delay(100);                            //Debounce button     
      if(key==5){                            //Tasta 5 este inca apasata?
        buttonPressStartTime = millis();     // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
        key5pressed = true;                  //confirma ca tasta 5 a fost apasata           
        }
    }
    if(key==0 && key5pressed)               //la ridicarea degetului de pe tasta se confirma Tasta-5 activata...    
    {
    unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
      
        if(pressDuration < SHORT_PRESS_THRESHOLD){
        /* Daca timpul apasarii este mai mic de 1 sec -> apasare scurta*/   
          setstep();                             //ajusteaza pasul encoder 
        }     
        else {// Daca timpul apasarii este mai mare de 1 sec -> apasare lunga
           if((flag_call)&&(flag_scala))flag_call=0;
          if(!flag_call)  flag_lock ^=1;                             //toggle Flag lOCK
           show_lock(flag_lock);
             
         /*************** Start Xtall Adjust ***********************/            
            if((!flag_xtalladj)&&(flag_call)&&(!flag_bfoadj)){
                romadd = 0x010 + (band*0x10);    //salveaza band in Memoria temporara 
                eprom.get(romadd, romf[0]);  
                freq[cur_vfo] = xtalFreq;                //copiaza xtallFreq pe encoder
                si5351aSetFrequency(10000000);  //seteaza iesirea de VFO pe 10MHz             
                show_frequency1(freq[cur_vfo]);
                flag_xtalladj = 1;               //Activate xtallAdjust
                vfofreqb = 0;                   //activeaza iesirea de VFO la schimbarea valorii
                show_msg(" XTALL [<->]Adjust.[Keep STEP] Save",1);        
            }
           else if((flag_xtalladj)&&(flag_call)&&(!flag_bfoadj)){
                xtalFreq = freq[cur_vfo];                  //citeste valoarea ajustata
                eprom.put(EEP_XTAL,xtalFreq);  // write to EEprom new XtallFreq
                freq[cur_vfo] = romf[0];                   //recupereaza frecventa salvata temporar
                PLL_write();
                
                 show_frequency1(freq[cur_vfo]);
                flag_xtalladj = 0;                 //Disable XtallAdj Flag 
                flag_call=0;
                show_msg("          Save to memory...", 0);
            }         
        }
         flag_frqwt = 1;                                // EEP Wite Flag
         timepassed = millis();      
        key5pressed = false;                        // Resetam stare tasta 5
        buttonPressStartTime = 0;                     //reset contor durata
    } 
}
/*---------- PLL write ---------------------------*/
void PLL_write(){
  if(!flag_bfoadj){  
    if(flag_rit) vfofreq = freq[cur_vfo] + cio+freq_ifshift; else vfofreq = freq[cur_vfo] + cio;  
    Vfo_out(vfofreq);                        
    Bfo_out(cio);
  }
  else{
    cio = freq[cur_vfo];
    Bfo_out(cio);                     
    freq[cur_vfo] = cio;
  }
    if(!flag_rit){
    if(!flag_scala)freqold=freq[cur_vfo]; 
    flag_frqwt = 1;                            
    timepassed = millis();    
    }
}
/*---------- VFO  out  ---------------*/ 
void Vfo_out(long frequency){
    si5351aSetFrequency(frequency);
}
/*----------  BFO out  ---------------*/        
void Bfo_out(long freqcio){
  if(freqcio != old_cio){
    si5351aSetFrequency2(freqcio);
    old_cio = freqcio;  
  }
}
/*------------- Show Temperature --------------------*/
void show_temperature(){
  int tmp = get_pa_temp();
    ucg.setFont(ucg_font_fub14_tf);
    ucg.setColor(backcolor);
    ucg.drawBox(200,56,35,18);
    ucg.setColor(CYAN);
    ucg.setPrintPos(200,72);        ucg.print(tmp);
    ucg.setColor(WHITE);
    ucg.setPrintPos(228,72);        ucg.print((char)176);ucg.print("C");
}
/* Citeste Temperatura de la thermistor 10K */
double get_pa_temp(void){
    int adc = analogRead(TEMP);
    double rThermistor = R_BALANCE * ((ADC_MAX / adc) - 1);
    /*Ecuatia Steinhart-Hart*/
    double temperatura_K = (BETA * ROOM_TEMP) / 
    (BETA + (ROOM_TEMP * log(rThermistor / R_NOMINAL)));
    return (int) temperatura_K - 273.15;                //transforma din K in Celsius
} 
/**** Rutina de afisare Voltaj Baterie ********************************/
void show_voltage(){
    double v1 = (double) analogRead(VOLT)* 3.3 / 4092 * 6 * 10;
    adc_v = (int) v1;
    ucg.setFont(ucg_font_fub14_tr);
    ucg.setColor(backcolor);
    ucg.drawBox(255,56,44,18);
    ucg.setPrintPos(240,72);     
    ucg.setColor(CYAN);
    ucg.setPrintPos(255,72);    ucg.print(adc_v / 10); ucg.print("."); ucg.print(adc_v % 10);
    ucg.setColor(WHITE);
    ucg.setPrintPos(295,72);    ucg.print(" V");   
}
/*---------- Bargraf meter -------------------------*/
void bargraf(){
    meter_value = analogRead(SMETER);
    meter_value = meter_value/200;                  
    if (meter_value > 15){meter_value = 15;}
    int sx1 = sx1+(meter_value*17);
    sx1 = sx1+41;
    int sx2 = 0;
    sx2 = sx2+(40+((15-meter_value)*17));
    ucg.setFont(ucg_font_fub35_tr);
    ucg.setColor(backcolor);
    ucg.drawBox(sx1,180,sx2,16);
    ucg.setPrintPos(40,200);
    /*Bargraf desenat cu liniute din caracterele fontului */
    for(int i=1;i<=meter_value;i++){
      if (i<=9){
        ucg.setColor(CYAN);           ucg.print("-");  
      }
      else{
        ucg.setColor(255,0,0);        ucg.print("-");
      }
    }
}
/*------------ On Air -----------------------------*/
void txset(){
  if(fmode == 2){                              // CW?
  Vfo_out(vfofreq + CW_TONE);               // Vfofreq+700Hz
  show_msg("                CW TONE OUT 700Hz",1); 
  }
  ucg.setColor(backcolor);
  ucg.drawBox(220,140,90,24);    
  ucg.setColor(YELLOW);  
  ucg.drawRFrame(220,140,90,24,3); 
  ucg.setFont(ucg_font_fub17_tr);
  ucg.setColor(RED); 
  ucg.setPrintPos(223,160);             ucg.print("ON AIR");

  while(digitalRead(SW_TX) == LOW){
    bargraf();
    /* Afiseaza Temp/Volt cu un refresh de 1sec */
    if(millis() > runseconds10volts + 1000){
    show_voltage();
    show_temperature();
    runseconds10volts=millis();
    }
  }
  ucg.setColor(backcolor);
  ucg.drawBox(220,140,90,24);  
}
/*------------- Mode AMP +20dB ---------------*/
void switch_amp(int state){
    ucg.setFont(ucg_font_fub17_tr);
    ucg.setColor(GRAY);
    if(state) ucg.setColor(GREEN);// else ucg.setColor(GRAY);
    ucg.setPrintPos(138,24);            ucg.print("AMP"); 
    if(state) ucg.setColor(WHITE);
    ucg.drawRFrame(133,3,60,25,3); 
    digitalWrite(AMP_OUT, flag_amp);
}
/*------------- Mode ATT --------------------*/
void switch_att(int state){
    ucg.setFont(ucg_font_fub17_tr);
    ucg.setColor(GRAY);
    if(state) ucg.setColor(CYAN);// else ucg.setColor(GRAY);
    ucg.setPrintPos(140,54);            ucg.print("ATT"); 
    if(state) ucg.setColor(WHITE);
    ucg.drawRFrame(133,33,60,25,3); 
    digitalWrite(ATT_OUT, flag_att);    
}
/*------------- show_mode (LSB-USB-CW-AM) ------------*/
void show_mode(){
    ucg.setFont(ucg_font_fub17_tr);
    ucg.setColor(GRAY);
    ucg.setPrintPos(10,24);           ucg.print("LSB"); 
    ucg.setPrintPos(75,24);           ucg.print("USB");
    ucg.setPrintPos(10,54);           ucg.print("C W");     
    ucg.setColor(GRAY);
    ucg.drawRFrame(3,3,60,25,3);
    ucg.drawRFrame(68,3,60,25,3);
    ucg.drawRFrame(3,33,60,25,3);
 
  switch(fmode){  
    case 0:                                       // LSB
      cio = eep_bfo[0];
      ucg.setColor(YELLOW);
      ucg.setPrintPos(10,24);          ucg.print("LSB");
      ucg.setColor(WHITE);
      ucg.drawRFrame(3,3,60,25,3);
      digitalWrite(MODE_OUT1,HIGH);
      digitalWrite(MODE_OUT2,LOW);
      break;    
    case 1:                                       // USB                                       
      cio = eep_bfo[1];
      ucg.setColor(CYAN);
      ucg.setPrintPos(75,24);          ucg.print("USB");
      ucg.setColor(WHITE);
      ucg.drawRFrame(68,3,60,25,3);
      digitalWrite(MODE_OUT1,LOW);
      digitalWrite(MODE_OUT2,HIGH);    
      break;
    case 2:                                       // CW
      cio = eep_bfo[2];
      ucg.setColor(GREEN);
      ucg.setPrintPos(10,54);          ucg.print("C W");
      ucg.setColor(WHITE);
      ucg.drawRFrame(3,33,60,25,3);
      digitalWrite(MODE_OUT1,LOW);
      digitalWrite(MODE_OUT2,LOW);
      break;
    default:
      cio = eep_bfo[0];
      ucg.setColor(YELLOW);
      ucg.setPrintPos(10,24);          ucg.print("LSB");
      ucg.setColor(WHITE);
      ucg.drawRFrame(3,3,60,25,3);
      digitalWrite(MODE_OUT1,HIGH);
      digitalWrite(MODE_OUT2,LOW);
      fmode = 0;
      break;
    }
}
/************* AGC-MODE ***********************/
void show_agc(int agc){
      ucg.setFont(ucg_font_fub17_tr);
      ucg.setColor(GRAY);
      if(flag_agc)  ucg.setColor(ORANGE);
      ucg.setPrintPos(75,54);                 ucg.print("AGC");
      if(flag_agc)  ucg.setColor(WHITE); 
      ucg.drawRFrame(68,33,60,25,3);      
      digitalWrite(AGC_OUT,flag_agc);
}
/*   Rutina de setare a pasului encoder*/
void setstep(){  
  if(fstep==1000) fstep = 10; else fstep = fstep * 10;
  flag_step = 1;
  show_step(); 
  delay(300);
  flag_step = 0;
  show_step(); 
}
/*-------------Display Step on LCD ---------------------*/
void show_step(){ 
  ucg.setColor(backcolor);
  ucg.drawRBox(278,88,32,38,3);
  ucg.setFont(ucg_font_fub11_tr);  
  if(flag_step) ucg.setColor(CYAN); else ucg.setColor(GRAY);
  ucg.setPrintPos(277,105);               ucg.print("STP");
  ucg.setPrintPos(278,123);
  if (fstep==1000)                        ucg.print("x1K");
  if (fstep==10)                          ucg.print("x10");
  if (fstep==100)                         ucg.print("100");
}
/*----------- Main frequency screen -------------------*/              
void show_frequency1(long f){
  freq_str = String(f);
  ucg.setFont(ucg_font_fub35_tn);   
  int mojisuu = (freq_str.length());
        
  if(f < 100){
    ucg.setColor(backcolor);
    ucg.drawBox(217,90,28,36);    
  }
  if(f10 !=(freq_str.charAt(mojisuu-2))){
    ucg.setColor(backcolor);
    ucg.drawBox(245,90,28,36); 
    ucg.setPrintPos(245,126);  
    ucg.setColor(GREEN);
    ucg.print(freq_str.charAt(mojisuu-2)); 
    f10 = (freq_str.charAt(mojisuu-2));
  }  
  if(f<1000){
    ucg.setColor(backcolor);
    ucg.drawBox(202,90,15,36);        
  }
  if(f100 != (freq_str.charAt(mojisuu-3))){
    ucg.setColor(backcolor);
    ucg.drawBox(217,90,28,36);
    ucg.setPrintPos(217,126);
    ucg.setColor(GREEN);
    ucg.print(freq_str.charAt(mojisuu-3)); 
    f100 = (freq_str.charAt(mojisuu-3));
  }
  if(f>=1000){
    ucg.setPrintPos(202,126);
    ucg.setColor(GREEN);
    ucg.print(".");
  }
  if(f<10000){
    ucg.setColor(backcolor);
    ucg.drawBox(146,90,28,36);     
  }
  if(f1k !=(freq_str.charAt(mojisuu-4))){
    ucg.setColor(backcolor);
    ucg.drawBox(174,90,28,36);
    ucg.setPrintPos(174,126);
    ucg.setColor(GREEN);
    ucg.print(freq_str.charAt(mojisuu-4));       
    f1k  = (freq_str.charAt(mojisuu-4));
  }
  if(f<100000){
    ucg.setColor(backcolor);
    ucg.drawBox(118,90,28,36); 
  }
  if(f10k !=(freq_str.charAt(mojisuu-5))){
    ucg.setColor(backcolor);
    ucg.drawBox(146,90,28,36);
    ucg.setPrintPos(146,126);
    ucg.setColor(GREEN);
    ucg.print(freq_str.charAt(mojisuu-5));   
    f10k = (freq_str.charAt(mojisuu-5));
  }
   if(f<1000000){
    ucg.setColor(backcolor);
    ucg.drawBox(103,90,15,36); 
    }
  if(f100k !=(freq_str.charAt(mojisuu-6))){
    ucg.setColor(backcolor);
    ucg.drawBox(118,90,28,36);
    ucg.setPrintPos(118,126);
    ucg.setColor(GREEN);
    ucg.print(freq_str.charAt(mojisuu-6));   
    f100k = (freq_str.charAt(mojisuu-6));
  }
   if(f>=1000000){
    ucg.setPrintPos(103,126);
    ucg.setColor(GREEN);
    ucg.print(".");
  }
   if(f<10000000){
    ucg.setColor(backcolor);
    ucg.drawBox(47,90,28,36);
  }
   if(fmega !=(freq_str.charAt(mojisuu-7))){
    ucg.setColor(backcolor);
    ucg.drawBox(75,90,28,36);
    ucg .setPrintPos(75,126);
    ucg.setColor(GREEN);
    ucg.print(freq_str.charAt(mojisuu-7));   
    fmega  = (freq_str.charAt(mojisuu-7));
  }
   if (f10m !=(freq_str.charAt(mojisuu-8))){
    ucg.setColor(backcolor);
    ucg.drawBox(47,90,28,36);
    ucg .setPrintPos(47,126);
    ucg.setColor(GREEN);
    ucg.print(freq_str.charAt(mojisuu-8));
    f10m = (freq_str.charAt(mojisuu-8));
  } 
}
  /************** Arata VFO si schimba *******************/
void show_frequency2(int vfo){
    int xpos = 10, ypos = 125;
    const char *vfostr[] = {"A", "B"};
    ucg.setColor(backcolor);
    ucg.drawBox(xpos-2,ypos-19,20,20);
    ucg.drawBox(xpos-2,ypos+35-19,20,20); 
    ucg.setFont(ucg_font_fub17_tr); 
    ucg.setColor(GREEN);
    ucg.setPrintPos(xpos,ypos);        ucg.print(vfostr[vfo]); 
    ucg.setColor(YELLOW);
    ucg.setPrintPos(xpos,ypos+35);     ucg.print(vfostr[!vfo]);
    ucg.setColor(backcolor);
    ucg.drawBox(50-2,160-18,120, 20);
    ucg.setColor(YELLOW);
    ucg.setPrintPos(50, 160);
/*-----------rutina de formatare String Frecventa--------------*/
/*** Rutina de formatare String generata de AI *********/    
    freq_str = String(freq[!cur_vfo]); 
    String formatted = "";                                    // Inițializare rezultat
    int length = freq_str.length();                           // Lungimea șirului
    
    for (int i = 0; i < length; i++) {
    formatted += freq_str.charAt(i);                          // Adaugă caracterul curent
      if ((length - i - 1) % 3 == 0 && i != length - 1) {
        formatted += ".";                                     // Adaugă punct dacă e nevoie
      }
    }
    ucg.print(formatted);
}
/*----------- Basic Screen -------------------------*/
void init_screen(){
  init_flag = 1;
  show_frequency2(cur_vfo);

  if(flag_scala){
    show_scara_mare();    
  }
  else {
    show_frequency1(freq[cur_vfo]);
    ucg.setColor(WHITE);
    ucg.setFont(ucg_font_fub11_tr); 
    ucg.setPrintPos(8,100);  
    ucg.print("VFO"); 
  }
  
  /*Deseneaza o rama dubla cu colturi rotunjite pentru Freqv.1 */
  ucg.setColor(WHITE);                  
  ucg.drawRFrame(1,80,314,55,5);
  ucg.drawRFrame(2,81,312,53,5);
  show_agc(flag_agc);
  show_mode();
  bands_sw_out();                                   //seteaza iesirile de banda   
  show_band_index();
  switch_amp(flag_amp);
  switch_att(flag_att);
  show_step();
  show_scara_mica(0);
  show_temperature();
  show_voltage();
  /*deseneaza grafic S-meter si PO-meter pentru bargraf*/
  ucg.setFont(ucg_font_fub11_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(10,210);                 ucg.print("P: 0----1----3------5--------10----------20 W");
  ucg.setPrintPos(10,177);                 ucg.print("S: -1----3----5-----7-----9");
  ucg.setColor(RED);                       ucg.print("---20---40---60dB");
  show_msg("  Please Wait! Loading program...  ", 1);
}
/*Seteaza iesirile BCD pentru schimbarea benzilor*/
void bands_sw_out(){
  digitalWrite(BAND_OUT1,LOW);
  digitalWrite(BAND_OUT2,LOW);
  digitalWrite(BAND_OUT3,LOW);
  if (band==0){}
  if (band==1){
   digitalWrite( BAND_OUT1,HIGH); 
  }
   if (band==2){
   digitalWrite(BAND_OUT2,HIGH); 
  }
  if (band==3){
   digitalWrite(BAND_OUT1,HIGH);
   digitalWrite(BAND_OUT2,HIGH); 
  }
  if (band==4){
   digitalWrite(BAND_OUT3,HIGH);
  }
  if (band==5){
   digitalWrite(BAND_OUT1,HIGH);
   digitalWrite(BAND_OUT3,HIGH); 
  }
  if (band==6){
   digitalWrite(BAND_OUT2,HIGH);
   digitalWrite(BAND_OUT3,HIGH);
  }
  if (band==7){
   digitalWrite(BAND_OUT1,HIGH);
   digitalWrite(BAND_OUT2,HIGH);
   digitalWrite(BAND_OUT3,HIGH);     
  }
}
/*---------- Show Band on LCD----------*/
void show_band_index(){
  ucg.setColor(backcolor);
  ucg.drawBox(2,60,90,20);
  ucg.setFont(ucg_font_fub11_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(5,75);                  ucg.print("Banda ");
  ucg.setColor(CYAN);
  switch(band+1){ 
    case 1:
        ucg.print("80m");
        break;
    case 2:
        ucg.print("40m");
        break;
    case 3:
        ucg.print("20m");
        break;
    case 4:
        ucg.print("17m");
        break;
    case 5:
        ucg.print("15m");
        break;
    case 6:
        ucg.print("10m");
        break;                    
    default:
        break;    
  } 
}
/************ Arata mesajul SPLIT ********************/
void show_lock(bool ind){
  ucg.setFont(ucg_font_fub11_tr);
  ucg.setPrintPos(99,75);
  if(ind){
    ucg.setColor(ORANGE); ucg.print("   LOCK  ");
  }else{
  ucg.setColor(backcolor);  ucg.drawBox(98,60,98,20);
  }
}
/********* Mesaj afisat pe row 9*****************/
void show_msg(const char *msg, int bcg){  
  ucg.setFont(ucg_font_fub11_tr);
  if(bcg) ucg.setColor(CYAN); else ucg.setColor(backcolor);
  ucg.drawBox(0,215,320,20);  
  if(bcg) ucg.setColor(backcolor); else ucg.setColor(GRAY);   
  ucg.setPrintPos(0,230);                                         ucg.print(msg);  
  runseconds10msg = millis(); 
  if(flag_xtalladj || flag_bfoadj) message = 0; else message = 1;   //Activeaza resetarea mesajului initial  
}
