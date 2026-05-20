/*Versiunea 4 cu scala mare pe mijlocul ecranului

13.05.2025
- Butoane: Band[1], IF-SHIFT[2], Mode[3], AMP/ATT[4], Encoder[STEP/BFO-Adj-XTALL-Adj]
- selectare frecventa procesor 48MHz, HSI-USB, optimizare Fast(-01)
- afisare AMP/ATT fgara comanda externa
- selectie BFO Adjust pe Encoder apasat lung, fara IF-SHIFT activat
- selectie XTALL Adj pe encoder apasat lung, Cu IF-SHIFT activat
- Afisare 25C-12,8V fara actionare externa
- Memorie []
- Meniu simplu din tasta Encoder 
	1. keep ENC fara SHIFT activat = BFO Adjust
	2. keep ENC cu SHIFT activat   = XTALL Adjust
	*********************************************************************************/
  /*-----------Display IF-SHIFT/RIT scala on screen --------------*/
/*----------Rutina generata cu ajutorul AI-----------------------*/
void show_scara_mica(int act){
  int xpos=200;
  int ypos=3;  

 
  ucg.setColor(GRAY);
  if (act)ucg.setColor(YELLOW); 
  ucg.drawRFrame(xpos,ypos+8,120,38,3);                   //Round Frame    
   ucg.setColor(backcolor);
  ucg.drawBox(xpos+28,ypos,66,15);                        //fundal text "IF SHIFT"
  ucg.setFont(ucg_font_fub11_hr); 
  
  ucg.setColor(GRAY);
  if (act)ucg.setColor(YELLOW);
  ucg.setPrintPos(xpos+30,ypos+13);               ucg.print("RIT x10");

  int x_start = 209;
  int baseline_y = 47;
  int spacing; 
  int thickness = 2;
  int line_count;                                   
  int number;
  int divizor;
  number=-2; spacing=10; line_count=11;
 
  ucg.setColor(backcolor); 
  ucg.drawBox(xpos+58+freq_ifshift_old/5, ypos+18, thickness+2, 26);  //sterge acul vechi
  
  freq_ifshift_old = freq_ifshift;  

  ucg.setColor(RED); 
  bool current_is_small;
  ucg.drawBox(xpos+58+freq_ifshift/5, ypos+18, thickness+2, 26);    //deseneaza acul rosu
  
  current_is_small = !start_with_small;              // Setăm să începem cu linie mică
  for (int i = 0; i < line_count; i++) {
    int len = current_is_small ? 8 : 12;                  // Linia mică = 8px, linia mare = 12px
    int x_position = x_start + (i * spacing);
    int y_position = baseline_y - len;

    ucg.setColor(GRAY);
    if (act)ucg.setColor(WHITE);
    ucg.drawBox(x_position, y_position, thickness, len);

    if (!current_is_small) {                              // Doar liniile mari primesc cifre
      int text_x = (number < 0) ? x_position - 8 : x_position-5;
      ucg.setPrintPos(text_x, y_position - 3);
      ucg.print(number);
      number++;
    }
    current_is_small = !current_is_small;                 // Alternăm linia următoare
  }
}

/************ Deseneaza scala mare pe mijlocul ecranului******************************/
void show_scara_mare(){
  int xpos=47;
  int ypos=90;  
   ucg.setFont(ucg_font_fub11_hr); 
     
  int x_start = 50;
  int baseline_y = 126;
  int spacing; 
  int thickness = 3;
  int line_count;                                   
  int number;
  int divizor;
    
    switch (band){
       case 0://Banda 80m   
          number=35;spacing=14; line_count=16; divizor=1400;break;
       case 1://Banda 40m   
          number=700;spacing=10; line_count=21; divizor=1000;break;
       case 2://Banda 20m   
          number=140; spacing=12; line_count=19;divizor=1650;break;
       case 3://Banda 17m   
          number=1805; spacing=13; line_count=16;divizor=750;break;
       case 4://Banda 15m   
          number=210; spacing=9; line_count=24;divizor=2400;break;
       case 5://Banda 10m   
          number=2800; spacing=10; line_count=21;divizor=5000;break;
    }
    ucg.setColor(backcolor); 
    ucg.drawBox(xpos+(freqold- Flim[band][0])/divizor, ypos, thickness+2, 36);      //sterge acul_ind old

    if(!init_flag)freqold=freq[cur_vfo];           //actualizeaza frecventa noua
    if(init_flag)init_flag=0;
    
    ucg.setColor(RED); 
    bool current_is_small;

    ucg.drawBox(xpos+(freq[cur_vfo]-Flim[band][0])/divizor, ypos, thickness+2, 36);  //deseneaza acul rosu
    current_is_small = false; //start_with_small;              // Setăm sa începem cu linie mare
  
  for (int i = 0; i < line_count; i++){
    if((i==0)||(i%5==0))  current_is_small = false;                 // Alternăm linia următoare
    
    int len = current_is_small ? 8 : 18;                  // Linia mică = 8px, linia mare = 12px
    int x_position = x_start + (i * spacing);
    int y_position = baseline_y - len;

    ucg.setColor(WHITE);
    ucg.drawBox(x_position, y_position, thickness, len);

    if (!current_is_small){                              // Doar liniile mari primesc cifre
      int text_x=x_position-20; //= (number < 0) ? x_position -  : x_position-5;
      ucg.setPrintPos(text_x, y_position - 7);
            
      if(band==1 || band==3 || band==5){
        ucg.print(number/100);ucg.print(".");
        if((band==1 && i==5)||(band==3 && i==0))ucg.print(0);
        ucg.print(number%100);
        if(band==1 && i==0)ucg.print(0);
        if(band==5)number+=25; else number+=5; 
      }
      else{
        ucg.print(number/10);ucg.print(".");ucg.print(number%10);        
        if(band==0)ucg.print(0);
        number +=1;  
      }    
    }
    current_is_small=true;    
 }
}
     
