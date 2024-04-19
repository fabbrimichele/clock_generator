#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SI5351.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_SI5351 clockgen = Adafruit_SI5351();

struct OutFreq {
  char* freq0;
  char* freq1;
  char* freq2;
};

void setup() {
  OutFreq outFreq;
  
  Serial.begin(9600);

  init_clock();
  outFreq = set_clock_freq();
  
  init_display();
  display_freq(outFreq);
}

void loop() {
}


void init_display() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    stop();
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();  
  display.display();

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(2);              // Draw 2X-scale text  
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);                 // Use full 256 char 'Code Page 437' font

}

void display_freq(OutFreq outFreq) {
  char buffer[40];

  sprintf(buffer, "%s:%s", "0", outFreq.freq0);
  display.setCursor(0, 5);     
  display.write(buffer);

  sprintf(buffer, "%s:%s", "1", outFreq.freq1);
  display.setCursor(0, 25);    
  display.write(buffer);
  
  sprintf(buffer, "%s:%s", "2", outFreq.freq2);
  display.setCursor(0, 45);    
  display.write(buffer);

  display.display();    
}

void init_clock() {
  Serial.println("Si5351 Clockgen Test"); Serial.println("");

  /* Initialise the sensor */
  if (clockgen.begin() != ERROR_NONE)
  {
    /* There was a problem detecting the IC ... check your connections */
    Serial.print("Ooops, no Si5351 detected ... Check your wiring or I2C ADDR!");
    stop();
  }

  Serial.println("OK!");
}

OutFreq set_clock_freq() {
  OutFreq outFreq;
  
  clockgen.enableOutputs(false);  

  /*
     PLL A                                
     Input Frequency (MHz) = 25.000000000 
     VCO Frequency (MHz) =  756.000000000 
     Feedback Divider = 30  6/25          
  */
  clockgen.setupPLL(SI5351_PLL_A, 35, 7, 25);

  /*
     PLL B                               
     Input Frequency (MHz) = 25.000000000 
     VCO Frequency (MHz) =  855.950000000
     Feedback Divider = 34  119/500      
  */
  clockgen.setupPLL(SI5351_PLL_A, 35, 7, 25);


  /*
    Channel 0
    Output Frequency (MHz) = 36.000000000
    Multisynth Output Frequency (MHz) = 36.000000000
    Multisynth Divider = 21
    R Divider = 1
    PLL source = PLLA
  */
  clockgen.setupMultisynth(0, SI5351_PLL_A, 28, 0, 1);
  outFreq.freq0 = "31.5000M";

  /*  
    Channel 1
    Output Frequency (MHz) = 25.175000000
    Multisynth Output Frequency (MHz) = 25.175000000
    Multisynth Divider = 34
    R Divider = 1
    PLL source = PLLB
  */
  clockgen.setupMultisynth(0, SI5351_PLL_A, 28, 0, 1);
  outFreq.freq0 = "31.5000M";

  /*
    Channel 2
    Output Frequency (MHz) = 31.500000000
    Multisynth Output Frequency (MHz) = 31.500000000
    Multisynth Divider = 24
    R Divider = 1
    PLL source = PLLA
  */
  clockgen.setupMultisynth(0, SI5351_PLL_A, 28, 0, 1);
  outFreq.freq0 = "31.5000M";

  return outFreq;

  
  /* INTEGER ONLY MODE --> most accurate output */
  /* Setup PLLA to integer only mode @ 900MHz (must be 600..900MHz) */
  /* Set Multisynth 0 to 112.5MHz using integer only mode (div by 4/6/8) */
  /* 25MHz * 36 = 900 MHz, then 900 MHz / 8 = 112.5 MHz */
  /*
  Serial.println("Set PLLA to 900MHz");
  clockgen.setupPLLInt(SI5351_PLL_A, 36);
  Serial.println("Set Output #0 to 112.5MHz");
  clockgen.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);
  outFreq.freq1 = "112.500M";
  */
  /* FRACTIONAL MODE --> More flexible but introduce clock jitter */
  /* Setup PLLB to fractional mode @616.66667MHz (XTAL * 24 + 2/3) */
  /* Setup Multisynth 1 to 13.55311MHz (PLLB/45.5) */
  /*
  clockgen.setupPLL(SI5351_PLL_B, 24, 2, 3);
  Serial.println("Set Output #1 to 13.553115MHz");
  clockgen.setupMultisynth(1, SI5351_PLL_B, 45, 1, 2);
  outFreq.freq1 = "13.5531M";
  */
  /* Multisynth 2 is not yet used and won't be enabled, but can be */
  /* Use PLLB @ 616.66667MHz, then divide by 900 -> 685.185 KHz */
  /* then divide by 64 for 10.706 KHz */
  /* configured using either PLL in either integer or fractional mode */
  /*
  Serial.println("Set Output #2 to 10.706 KHz");
  clockgen.setupMultisynth(2, SI5351_PLL_B, 900, 0, 1);
  clockgen.setupRdiv(2, SI5351_R_DIV_64);
  outFreq.freq2 = "10.7060K";
  */
  /* Enable the clocks */
  /* clockgen.enableOutputs(true);  */

}

void stop() {
  while(1);
}
