#include "display.h"
#include "mykeyboard.h"

#if defined(CARDPUTER) || defined(STICK_C_PLUS2)  //Battery Calculation
  #include <driver/adc.h>
  #include <esp_adc_cal.h>
  #include <soc/soc_caps.h>
  #include <soc/adc_channel.h>
#endif

/***************************************************************************************
** Function name: resetTftDisplay
** Description:   set cursor to 0,0, screen and text to default color
***************************************************************************************/
void resetTftDisplay(int x, int y, uint16_t fc, int size, uint16_t bg, uint16_t screen) {
    tft.setCursor(x,y);
    tft.fillScreen(screen);
    tft.setTextSize(size);
    tft.setTextColor(fc,bg);
}

/***************************************************************************************
** Function name: resetSpriteDisplay -> Sprite
** Description:   set cursor to 0,0, screen and text to default color
***************************************************************************************/
void resetSpriteDisplay(int x, int y, uint16_t fc, int size, uint16_t bg, uint16_t screen) {
    if(x<0) x=0;
    if(y<0) y=0;
    sprite.setCursor(x,y);
    sprite.fillScreen(screen);
    sprite.setTextSize(size);
    sprite.setTextColor(fc,bg);
}
/***************************************************************************************
** Function name: setTftDisplay
** Description:   set cursor, font color, size and bg font color
***************************************************************************************/
void setTftDisplay(int x, int y, uint16_t fc, int size, uint16_t bg) {
    if (x>=0 && y<0)        tft.setCursor(x,tft.getCursorY());          // if -1 on x, sets only y
    else if (x<0 && y>=0)   tft.setCursor(tft.getCursorX(),y);          // if -1 on y, sets only x
    else if (x>=0 && y>=0)  tft.setCursor(x,y);                         // if x and y > 0, sets both
    tft.setTextSize(size);
    tft.setTextColor(fc,bg);
}

/***************************************************************************************
** Function name: setSpriteDisplay -> SPRITE
** Description:   set cursor, font color, size and bg font color of Sprite
***************************************************************************************/
void setSpriteDisplay(int x, int y, uint16_t fc, int size, uint16_t bg) {
    if (x>=0 && y<0)        sprite.setCursor(x,sprite.getCursorY());    // if -1 on x, sets only y
    else if (x<0 && y>=0)   sprite.setCursor(sprite.getCursorX(),y);    // if -1 on y, sets only x
    else if (x>=0 && y>=0)  sprite.setCursor(x,y);                      // if x and y > 0, sets both
    sprite.setTextSize(size);
    sprite.setTextColor(fc,bg);
}

/***************************************************************************************
** Function name: BootScreen
** Description:   Start Display functions and display bootscreen
***************************************************************************************/
void initDisplay() {

    sprite.fillRect(0,0,WIDTH,HEIGHT,BGCOLOR);

    srand(time(0));
    sprite.setTextSize(FP);
    sprite.setCursor(0,0);
    while(sprite.getCursorY()<sprite.height()) {
        int cor = rand() % 3;
        if (cor==0) sprite.setTextColor(TFT_PURPLE-0x4000);
        else if (cor==1) sprite.setTextColor(TFT_PURPLE-0x2000);
        else sprite.setTextColor(TFT_PURPLE);
        sprite.print(random(0,9));
    }
    sprite.setTextSize(FG);
    sprite.setTextColor(TFT_PURPLE+0x4000);

    #ifndef STICK_C
    sprite.drawCentreString("BRUCE",sprite.width()/2,sprite.height()/2-10,SMOOTH_FONT); //SMOOTH_FONT
    #else
    sprite.drawCentreString("Launcher",sprite.width()/2,sprite.height()/2-10,SMOOTH_FONT); //SMOOTH_FONT
    #endif

    sprite.pushSprite(10,10);
    delay(50);

    tft.drawSmoothRoundRect(5,5,5,5,WIDTH-10,HEIGHT-10,FGCOLOR,BGCOLOR);
    
}

/***************************************************************************************
** Function name: displayRedStripe
** Description:   Display Red Stripe with information
***************************************************************************************/
void displayRedStripe(String text) {
#ifndef STICK_C
    int size;
    if(text.length()<19) size = FM;
    else size = FP;
    tft.fillRect(10, 55, WIDTH - 20, 26, ALCOLOR);
    if(size==2) setTftDisplay(WIDTH/2 - FM*3*text.length(), 60, TFT_WHITE, size, ALCOLOR);
    else setTftDisplay(WIDTH/2 - FP*3*text.length(), 65, TFT_WHITE, size, ALCOLOR);
    tft.println(text);
#else
    int size;
    if(text.length()<20) size = FM;
    else size = FP;
    tft.fillRect(10, 5, WIDTH - 20, 20, ALCOLOR);
    if(size==2) setTftDisplay(WIDTH/2 - FM*3*text.length(), 7, TFT_WHITE, size, ALCOLOR);
    else setTftDisplay(WIDTH/2 - FP*3*text.length(), 7, TFT_WHITE, size, ALCOLOR);
    tft.println(text);
#endif

}

/*********************************************************************
**  Function: loopOptions                             
**  Where you choose among the options in menu
**********************************************************************/
void loopOptions(const std::vector<std::pair<std::string, std::function<void()>>>& options, bool bright, bool submenu, String subText){
  bool redraw = true;
  int index = 0;
  while(1){
    if (redraw) { 
      if(submenu) drawSubmenu(index, options, subText);
      else drawOptions(index, options, FGCOLOR, BGCOLOR);
      if(bright){
        #if !defined(STICK_C_PLUS)
        int bl = MINBRIGHT + round(((255 - MINBRIGHT) * (4 - index) * 0.25)); // 4 is the number of options
        analogWrite(BACKLIGHT, bl);
        #else
        axp192.ScreenBreath(100*(4 - index) * 0.25);  // 4 is the number of options
        #endif
      }
      redraw=false;
      delay(200); 
    }

    if(checkPrevPress()) {
    #ifdef CARDPUTER  
      if(index==0) index = options.size() - 1;
      else if(index>0) index--;
      redraw = true;
    #else
      break;
    #endif
    }
    /* DW Btn to next item */
    if(checkNextPress()) { 
      index++;
      if((index+1)>options.size()) index = 0;
      redraw = true;
    }

    /* Select and run function */
    if(checkSelPress()) { 
      options[index].second();
      break;
    }

    #ifdef CARDPUTER
    Keyboard.update();
    if(Keyboard.isKeyPressed('`')) break;
    #endif
  }
  delay(200);
}

/***************************************************************************************
** Function name: progressHandler
** Description:   Função para manipular o progresso da atualização
** Dependencia: prog_handler =>>    0 - Flash, 1 - SPIFFS
***************************************************************************************/
void progressHandler(int progress, size_t total) {
#ifndef STICK_C
  int barWidth = map(progress, 0, total, 0, 200);
  if(barWidth <7) {
    tft.fillRect(6, 6, WIDTH-12, HEIGHT-12, BGCOLOR);
    if (prog_handler == 1) tft.drawRect(18, HEIGHT - 28, 204, 17, ALCOLOR);
    else tft.drawRect(18, HEIGHT - 47, 204, 17, FGCOLOR);
    
    String txt;
    switch(prog_handler) {
      case 0: 
          txt = "Installing FW";
          break;
      case 1: 
          txt = "Installing SPIFFS";
          break;
      case 2: 
          txt = "Downloading";
          break;          
    }
    displayRedStripe(txt);
  }
  if (prog_handler == 1) tft.fillRect(20, HEIGHT - 26, barWidth, 13, ALCOLOR);
  else tft.fillRect(20, HEIGHT - 45, barWidth, 13, FGCOLOR);
#else
  
  int barWidth = map(progress, 0, total, 0, 100);
  if(barWidth <5) {
    tft.fillRect(6, 6, WIDTH-12, HEIGHT-12, BGCOLOR);
    tft.drawRect(6, 6, WIDTH-12, HEIGHT-12, BGCOLOR);
    if (prog_handler == 1) tft.drawRect(28, HEIGHT - 28, 104, 17, ALCOLOR);
    else tft.drawRect(28, HEIGHT - 47, 104, 17, FGCOLOR);
    
    String txt;
    switch(prog_handler) {
      case 0: 
          txt = "Installing FW";
          break;
      case 1: 
          txt = "Installing SPIFFS";
          break;
      case 2: 
          txt = "Downloading";
          break;              
    }
    displayRedStripe(txt);
  }
  if (prog_handler == 1) tft.fillRect(30, HEIGHT - 26, barWidth, 13, ALCOLOR);
  else tft.fillRect(30, HEIGHT - 45, barWidth, 13, FGCOLOR);

#endif

}

/***************************************************************************************
** Function name: drawOptions
** Description:   Função para desenhar e mostrar as opçoes de contexto
***************************************************************************************/
void drawOptions(int index,const std::vector<std::pair<std::string, std::function<void()>>>& options, uint16_t fgcolor, uint16_t bgcolor) {
    int menuSize = options.size();
    if(options.size()>MAX_MENU_SIZE) menuSize = MAX_MENU_SIZE;

    menu_op.createSprite(WIDTH*0.7, (FM*8+4)*menuSize + 10);
    menu_op.fillRoundRect(0,0,WIDTH*0.7,(FM*8+4)*menuSize+10,5,bgcolor);
    
    menu_op.setTextColor(fgcolor,bgcolor);
    menu_op.setTextSize(FM);
    menu_op.setCursor(5,5);

    int i=0;
    int init = 0;
    menuSize = options.size();
    if(index>=MAX_MENU_SIZE) init=index-MAX_MENU_SIZE+1;
    for(i=0;i<menuSize;i++) {
      if(i>=init) {
        String text="";
        if(i==index) text+=">";
        else text +=" ";
        text += String(options[i].first.c_str());
        menu_op.setCursor(5,menu_op.getCursorY()+4);
        menu_op.println(text.substring(0,13));
      }
    }

    if(options.size()>MAX_MENU_SIZE) menuSize = MAX_MENU_SIZE;
    menu_op.drawRoundRect(0,0,WIDTH*0.7,(FM*8+4)*menuSize+10,5,fgcolor);
    menu_op.pushSprite(WIDTH*0.15,HEIGHT/2-menuSize*(FM*8+4)/2 -5);
    menu_op.deleteSprite();
}

/***************************************************************************************
** Function name: drawOptions
** Description:   Função para desenhar e mostrar as opçoes de contexto
***************************************************************************************/
void drawSubmenu(int index,const std::vector<std::pair<std::string, std::function<void()>>>& options, String system) {
    int menuSize = options.size();

    menu_op.createSprite(WIDTH - 20, HEIGHT - 35);
    menu_op.setTextColor(FGCOLOR,BGCOLOR);
    menu_op.setTextSize(FP);
    menu_op.setCursor(2,3);
    menu_op.setTextColor(FGCOLOR);
    menu_op.println(system);

    if (index-1>=0) {
      menu_op.setTextSize(FM);
      menu_op.setTextColor(FGCOLOR-0x2000);
      menu_op.drawCentreString(options[index-1].first.c_str(),menu_op.width()/2, 15,SMOOTH_FONT);
    } else {
      menu_op.setTextSize(FM);
      menu_op.setTextColor(FGCOLOR-0x2000);
      menu_op.drawCentreString(options[menuSize-1].first.c_str(),menu_op.width()/2, 15,SMOOTH_FONT);
    }
      menu_op.setTextSize(FG);
      menu_op.setTextColor(FGCOLOR);
      menu_op.drawCentreString(options[index].first.c_str(),menu_op.width()/2, 40,SMOOTH_FONT);

    if (index+1<menuSize) {
      menu_op.setTextSize(FM);
      menu_op.setTextColor(FGCOLOR-0x2000);
      menu_op.drawCentreString(options[index+1].first.c_str(),menu_op.width()/2, 75,SMOOTH_FONT);
    } else {
      menu_op.setTextSize(FM);
      menu_op.setTextColor(FGCOLOR-0x2000);
      menu_op.drawCentreString(options[0].first.c_str(),menu_op.width()/2, 75,SMOOTH_FONT);
    }

    menu_op.drawFastHLine(menu_op.width()/2 - options[index].first.size()*FG*LW/2, 40+FG*LH,options[index].first.size()*FG*LW,FGCOLOR);
    menu_op.fillRect(menu_op.width()-5,index*menu_op.height()/menuSize,5,menu_op.height()/menuSize,FGCOLOR);

    menu_op.pushSprite(10,27);
    menu_op.deleteSprite();
}



/***************************************************************************************
** Function name: drawMainMenu
** Description:   Função para desenhar e mostrar o menu principal
***************************************************************************************/
void drawMainMenu(int index) {
    const int border = 10;
    const uint16_t colors[6] = {        
        static_cast<uint16_t>(FGCOLOR), 
        static_cast<uint16_t>(FGCOLOR), 
        static_cast<uint16_t>(FGCOLOR), 
        static_cast<uint16_t>(sdcardMounted ? FGCOLOR : TFT_DARKGREY), 
        static_cast<uint16_t>(FGCOLOR), 
        static_cast<uint16_t>(FGCOLOR)
    };

    const char* texts[6] = { "WiFi", "BLE", "RF", "RFID", "Others", "Config" };

    sprite.deleteSprite();
    draw.deleteSprite();
    draw.createSprite(80,80);
    sprite.createSprite(WIDTH - 20, HEIGHT - 20);
    sprite.fillRect(0, 0, WIDTH, HEIGHT, BGCOLOR);
    setSpriteDisplay(2, 2, FGCOLOR, 1, BGCOLOR);
    sprite.print("hh:mm");
    if(wifiConnected) sprite.drawCentreString("(WiFi On)",sprite.width()/2,2,SMOOTH_FONT);
    sprite.setTextSize(FG);

    switch(index) {
      case 0:
        drawWifi();
        break;
      case 1:
        drawBLE();
        break;
      case 2:
        drawRf();
        break;
      case 3:
        drawRfid();
        break;
      case 4: 
        drawOther();
        break;
      case 5:
        drawCfg();
        break;
    }
    sprite.setTextSize(FM);
    sprite.drawCentreString(texts[index],sprite.width()/2, sprite.height()-LH*FM, SMOOTH_FONT);
    sprite.setTextSize(FG);
    sprite.drawChar('<',0,sprite.height()/2);
    sprite.drawChar('>',sprite.width()-LW*FG,sprite.height()/2);
    sprite.pushSprite(10,10);
    draw.pushSprite(80,27);

    tft.drawRoundRect(5, 5, WIDTH - 10, HEIGHT - 10, 5, FGCOLOR);
    tft.drawLine(5, 25, WIDTH - 5, 25, FGCOLOR);
    drawBatteryStatus();
}


/***************************************************************************************
** Function name: getBattery()
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() {
  int percent=0;
  #if defined(STICK_C_PLUS)
  float b = axp192.GetBatVoltage();
  percent = ((b - 3.0) / 1.2) * 100;
  
  #else
  
    #if defined(CARDPUTER)
      uint8_t _batAdcCh = ADC1_GPIO10_CHANNEL;
      uint8_t _batAdcUnit = 1;
    #elif defined(STICK_C_PLUS2)
      uint8_t _batAdcCh = ADC1_GPIO38_CHANNEL;
      uint8_t _batAdcUnit = 1;
    #endif
  
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten((adc1_channel_t)_batAdcCh, ADC_ATTEN_DB_11);
    static esp_adc_cal_characteristics_t* adc_chars = nullptr;
    static constexpr int BASE_VOLATAGE = 3600;
    adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize((adc_unit_t)_batAdcUnit, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, BASE_VOLATAGE, adc_chars);
    int raw;
    raw = adc1_get_raw((adc1_channel_t)_batAdcCh);
    uint32_t volt = esp_adc_cal_raw_to_voltage(raw, adc_chars);

    float mv = volt * 2;
    percent = (mv - 3300) * 100 / (float)(4150 - 3350);

  #endif
  return  (percent < 0) ? 0
        : (percent >= 100) ? 100
        :  percent;
}

/***************************************************************************************
** Function name: drawBatteryStatus()
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
void drawBatteryStatus() {
    tft.drawRoundRect(WIDTH - 42, 7, 34, 14, 2, FGCOLOR);
    int bat = getBattery();
    tft.setTextSize(FP);
    tft.setTextColor(FGCOLOR, BGCOLOR);
    tft.drawRightString(String(bat) + "%", WIDTH - 45, 10, 1);
    tft.fillRoundRect(WIDTH - 40, 9, 30 * bat / 100, 10, 2, FGCOLOR);
    tft.drawLine(WIDTH - 30, 9, WIDTH - 30, 9 + 10, BGCOLOR);
    tft.drawLine(WIDTH - 20, 9, WIDTH - 20, 9 + 10, BGCOLOR);
}


/***************************************************************************************
** Function name: listFiles
** Description:   Função para desenhar e mostrar o menu principal
***************************************************************************************/
#define MAX_ITEMS 7
void listFiles(int index, String fileList[][3]) {
    sprite.fillRect(0,0,sprite.width(),sprite.height(),BGCOLOR);
    sprite.setCursor(0,0);
    sprite.setTextSize(FM);
    int arraySize = 0;
    while(fileList[arraySize][2]!="" && arraySize < MAXFILES) arraySize++;
    int i=0;
    int start=0;
    if(index>=MAX_ITEMS) {
        start=index-MAX_ITEMS+1;
        if(start<0) start=0;
    }
    
    while(i<arraySize) {
        if(i>=start && fileList[i][2]!="") {
            if(fileList[i][2]=="folder") sprite.setTextColor(FGCOLOR-0x1111);
            else if(fileList[i][2]=="operator") sprite.setTextColor(ALCOLOR);
            else sprite.setTextColor(FGCOLOR);

            if (index==i) sprite.print(">");
            else sprite.print(" ");
            sprite.println(fileList[i][0].substring(0,16));
            
        }
        i++;
        if (i==(start+MAX_ITEMS) || fileList[i][2]=="") break;
    }
    
    sprite.pushSprite(10,10);
}


void displayScanning() {
    //Show Scanning display
    menu_op.deleteSprite();
    menu_op.createSprite(WIDTH*0.7, FM*16);
    menu_op.fillRoundRect(0,0,WIDTH*0.7,FM*16,5,BGCOLOR);
    
    menu_op.setTextColor(TFT_WHITE,FGCOLOR);
    menu_op.setTextSize(FM);
    menu_op.fillRoundRect(0,0,WIDTH*0.7,FM*16,5,FGCOLOR);
    menu_op.drawCentreString("Scanning..",menu_op.width()/2,5,1);
   
    menu_op.pushSprite(WIDTH*0.15,HEIGHT/2-10 -5);
    menu_op.deleteSprite();
}






// desenhos do menu principal, sprite "draw" com 80x80 pixels


void drawWifi() {
  draw.fillSprite(BGCOLOR);
  draw.fillCircle(40,60,6,FGCOLOR);
  draw.drawSmoothArc(40,60,26,20,130,230,FGCOLOR, BGCOLOR,true);
  draw.drawSmoothArc(40,60,46,40,130,230,FGCOLOR, BGCOLOR,true);
}

void drawBLE() {
  draw.fillSprite(BGCOLOR);
  draw.drawWideLine(40,53,2,26,5,FGCOLOR,BGCOLOR);
  draw.drawWideLine(40,26,2,53,5,FGCOLOR,BGCOLOR);
  draw.fillTriangle(40,26,20,40,20,12,FGCOLOR);
  draw.fillTriangle(40,53,20,40,20,68,FGCOLOR);
  draw.drawArc(40,40,10,12,210,330,FGCOLOR,BGCOLOR);
  draw.drawArc(40,40,23,25,210,330,FGCOLOR,BGCOLOR);
  draw.drawArc(40,40,36,38,210,330,FGCOLOR,BGCOLOR);
}

void drawCfg() {
  draw.fillSprite(BGCOLOR);
  int i=0;
  for(i=0;i<6;i++) {
    draw.drawArc(40,40,30,20,15+60*i,45+60*i,FGCOLOR,BGCOLOR,true);
  }
  draw.drawArc(40,40,22,8,0,360,FGCOLOR,BGCOLOR,false);
}

void drawRf() {
  draw.fillSprite(BGCOLOR);
  draw.fillCircle(40,30,7,FGCOLOR);
  draw.fillTriangle(40,40,25,70,55,70,FGCOLOR);
  draw.drawArc(40,30,18,15,40,140,FGCOLOR,BGCOLOR);
  draw.drawArc(40,30,28,25,40,140,FGCOLOR,BGCOLOR);
  draw.drawArc(40,30,38,35,40,140,FGCOLOR,BGCOLOR);
  draw.drawArc(40,30,18,15,220,320,FGCOLOR,BGCOLOR);
  draw.drawArc(40,30,28,25,220,320,FGCOLOR,BGCOLOR);
  draw.drawArc(40,30,38,35,220,320,FGCOLOR,BGCOLOR);
}

void drawRfid() {
  draw.fillSprite(BGCOLOR);
  draw.drawRoundRect(5,5,70,70,10,FGCOLOR);
  draw.fillRect(0,40,40,40,BGCOLOR);
  draw.drawCircle(15,65,7,FGCOLOR);
  draw.drawArc(15,65,18,15,180,270,FGCOLOR,BGCOLOR);
  draw.drawArc(15,65,28,25,180,270,FGCOLOR,BGCOLOR);
  draw.drawArc(15,65,38,35,180,270,FGCOLOR,BGCOLOR);
}

void drawOther() {
  draw.fillSprite(BGCOLOR);
  draw.fillCircle(40,40,7,FGCOLOR);
  draw.drawArc(40,40,18,15,0,340,FGCOLOR,BGCOLOR);
  draw.drawArc(40,40,25,22,20,360,FGCOLOR,BGCOLOR);
  draw.drawArc(40,40,32,29,0,200,FGCOLOR,BGCOLOR);
  draw.drawArc(40,40,32,29,240,360,FGCOLOR,BGCOLOR);
}

