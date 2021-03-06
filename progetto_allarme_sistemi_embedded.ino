/*
  The circuit:
   LCD RS pin to digital pin 41
   LCD Enable pin to digital pin 39
   LCD D4 pin to digital pin 37
   LCD D5 pin to digital pin 35
   LCD D6 pin to digital pin 33
   LCD D7 pin to digital pin 31
   LCD R/W pin to ground
   LCD VSS pin to ground
   LCD VCC pin to 5V
   ends to +5V and ground
   wiper to LCD VO pin (pin 3)
*/

// importazione librerie:
#include <LiquidCrystal.h>
#include <SR04.h>
#include <pitches.h>
#include <Keypad.h>

// inizializzazione display lcd
LiquidCrystal lcd(41, 39, 37, 35, 33, 31);

//definizione pin per i sensori di prossimità

//sensore 1
#define echo_pin 53
#define trig_pin 51
//sensore 2
#define echo_pin2 52
#define trig_pin2 50
//sensore 3
#define echo_pin3 48
#define trig_pin3 46

//istanza sensori di prossimità
SR04 sensore  = SR04(echo_pin, trig_pin);
SR04 sensore2 = SR04(echo_pin2, trig_pin2);
SR04 sensore3 = SR04(echo_pin3, trig_pin3);

//definizione pin per i 3 led
#define GREEN 42
#define RED 44
#define YELLOW 40

//definizione variabili per la misurazione delle distanze e la durata del suono del buzzer
int distanza;
int distanza2;
int distanza3;
int durata_suono = 100;

//definizione keypad, impostazioni righe/colonne
const byte rows = 4;
const byte cols = 4;

char Keys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

//definizione pin keypad
byte colPins[cols] = {6, 7, 8, 9};
byte rowPins[rows] = {2, 3, 4, 5};

//inizializzazione del keypad
Keypad key = Keypad(makeKeymap(Keys), rowPins, colPins, rows, cols);
String s = ""; // stringa per il codice di sblocco

//definizione variabili per i valori di normalità dei 3 sensori
int safety1 = 0;
int safety2 = 0;
int safety3 = 0;

//stampa la stringa ogni volta che è richiesto il codice di sblocco
void print_stringa_sblocco() {
  lcd.print("codice sblocco:");
  lcd.setCursor(0, 1);
}

//permette di cancellare quando si digita sul tastierino
String cancella(String str) {
  lcd.clear();
  str = "";
  print_stringa_sblocco();
  return str;
}

//definisce il valore della distanza normale del sensore passato come argomento
int monitoraggio(SR04 sensor) {
  int result = 0;
  int distanza = 0;
  for (int i = 0; i <= 150; i++) {
    distanza = sensor.Distance();
    if (distanza - result >= 10) {
      result = distanza;
    }
    else {
      continue;
    }
  }
  return result;
}

//stampa su schermo la misurazione del sensore passato come argomento
void stampa_misurazione(String s, int n) {
  lcd.print("Sensore " + s + ": ");
  lcd.print(n);
  lcd.print("cm");
  delay(2000);
}

//effettua il controllo confrontando la distanza normale con la misurazione attuale del sensore passato come argomento
boolean controllo(int sensor, int misurazione) {
  int safe = 0;
  switch (sensor) {
    case 1: safe = safety1; break;
    case 2: safe = safety2; break;
    case 3: safe = safety3; break;
  }
  if (misurazione < (safe - 5)) {
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
    lcd.print("Area violata su");
    lcd.setCursor(0, 1);
    stampa_misurazione(String(sensor), misurazione);
    delay(1500);
    lcd.clear();
    print_stringa_sblocco();
    return false;
  }
  else {
    if (misurazione < 400) {
      lcd.print("Area sicura");
      digitalWrite(GREEN, HIGH);
      delay(1000);
      lcd.clear();
    }
    //i sensori hanno un raggio di 4 metri
    else {
      lcd.print("Fuori scala");
      lcd.setCursor(0, 1);
      lcd.print(">= 400cm");
      digitalWrite(YELLOW, HIGH);
      delay(2000);
      lcd.clear();
    }
  }
  return true;
}

void setup() {

  // set up schermo LCD con numero di righe e colonne
  lcd.begin(16, 2);

  // prima stampa su schermo LCD una volta acceso il sistema.
  lcd.print("Sistema");
  lcd.setCursor(0, 1);
  lcd.print("inizializzato");
  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);
  digitalWrite(YELLOW, HIGH);
  delay(2000);
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  lcd.clear();

  //monitoraggio per la definizione della distanza "normale" per tutti i sensori
  lcd.print("Monitorando...");
  safety1 = monitoraggio(sensore);
  lcd.clear();
  stampa_misurazione("1", safety1);
  lcd.clear();
  lcd.print("Monitorando...");
  safety2 = monitoraggio(sensore2);
  lcd.clear();
  stampa_misurazione("2", safety2);
  lcd.clear();
  lcd.print("Monitorando...");
  safety3 = monitoraggio(sensore3);
  lcd.clear();
  stampa_misurazione("3", safety3);
  lcd.clear();
}

void loop() {
  //assegnamento variabili che contengono la misurazione dei 3 sensori
  distanza  = sensore.Distance();
  distanza2 = sensore2.Distance();
  distanza3 = sensore3.Distance();

  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(YELLOW, LOW);

  //condizione che verifica che non ci sia un'anomalia dalla situazione monitorata inizialmente
  if (not(controllo(1, distanza)) || not(controllo(2, distanza2)) || not(controllo(3, distanza3))) {
    while (true) {
      tone(49, NOTE_C3, durata_suono);
      char input = key.getKey();
      if (input == 'C') {
        s = cancella(s);
      }
      else {
        s.concat(String(input));
      }
      if (input && !(input == 'C')) {
        lcd.print(input);
        delay(500);
        if (s.startsWith("117") && s.endsWith("*")) {
          lcd.clear();
          lcd.print("codice corretto");
          s = "";
          delay(1000);
          lcd.clear();
          break;
        } else if (s.endsWith("*")) {
          lcd.clear();
          lcd.print("codice errato");
          s = "";
          delay(1000);
          lcd.clear();
          print_stringa_sblocco();
          continue;
        }
      }
    }
  }
  lcd.clear();
}
