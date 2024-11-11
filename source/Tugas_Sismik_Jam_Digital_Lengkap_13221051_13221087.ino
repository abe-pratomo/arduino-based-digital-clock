/* ############################################################################################################################################################################## */
/* ############################################################################# START OF PROGRAM ############################################################################### */
/* ############################################################################################################################################################################## */

/*                                       Deklarasi libraries, definisi pin-pin dan alamat yang digunakan, dan inisiasi variabel-variabel penting                                  */

// Deklarasi libraries -------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Wire.h> // Deklarasi library wire.h
#include <LiquidCrystal_PCF8574.h> // Deklarasi library untuk LCD I2C
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Definisi pin-pin ----------------------------------------------------------------------------------------------------------------------------------------------------------------
#define MODE 5 // Definisi pin D5 untuk tombol 1 / MODE
#define SET 18 // Definisi pin D4 untuk tombol 2 / SET
#define RESET 19 // Definisi pin D3 untuk tombol 3 / RESET
#define BUZZER 2 // Definisi pin D2 untuk BUZZER

LiquidCrystal_PCF8574 lcd(0x27); // Definisi alamat I2C modul LCD
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi variabel state dari jam digital ----------------------------------------------------------------------------------------------------------------------------------------
int state = -1; // State -1 menandakan state startup
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi variabel untuk startup (state -1) --------------------------------------------------------------------------------------------------------------------------------------
int dots = 0; // Inisiasi variabel penentu jumlah titik setelah pesan "starting"
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi variabel-variabel untuk mode Jam (state 0 dan 1 untuk setting) ---------------------------------------------------------------------------------------------------------
int jamClock = 0;
int menitClock = 0;
int detikClock = 0;
int milidetikClock = 0;
bool tick = false; // Boolean untuk penghitung kedip elemen waktu
bool showSeconds = false; // Boolean untuk memperlihatkan detik di mode jam

int stateClockSetting = 1; // Inisiasi state untuk elemen waktu Jam yang diedit, state 1 untuk jam
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi variabel-variabel untuk mode Stopwatch (state 2) -----------------------------------------------------------------------------------------------------------------------
int jamSW = 0;
int menitSW = 0;
int detikSW = 0;
int milidetikSW = 0;
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi variabel-variabel untuk mode Timer (state 3 dan 4 untuk setting) -------------------------------------------------------------------------------------------------------
int jamTimer = 0;
int menitTimer = 0;
int detikTimer = 0;
int milidetikTimer = 0;

int stateTimerSetting = 1; // Inisiasi state untuk elemen waktu Timer yang diedit, state 1 untuk detik
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi variabel-variabel untuk mode Buzzer (state 5) --------------------------------------------------------------------------------------------------------------------------
int buzzCount = 0; // Inisiasi variabel penentu jumlah buzz
bool beep = true; // Boolean penentu apakah waktunya buzz atau tidak
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi variabel untuk interrupt mode Jam --------------------------------------------------------------------------------------------------------------------------------------
bool interrupt1Aktif = true; // Variabel untuk menentukan apakah interrupt Timer1 (Jam) aktif atau tidak
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi variabel untuk interrupt mode Stopwatch --------------------------------------------------------------------------------------------------------------------------------
bool interrupt2Aktif = false; // Variabel untuk menentukan apakah interrupt Timer2 (Stopwatch) aktif atau tidak
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Inisiasi variabel-variabel untuk interrupt mode Timer // -------------------------------------------------------------------------------------------------------------------------
bool interrupt3Aktif = false; // Variabel untuk menentukan apakah interrupt Timer0 (Timer) aktif atau tidak
int prebuzz = 0; // Variabel untuk menyimpan state terakhir sebelum buzz() dijalankan
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Inisiasi boolean-boolean untuk debouncing tombol --------------------------------------------------------------------------------------------------------------------------------
bool modePressed = false; // Variabel untuk debounce tombol MODE
bool setPressed = false; // Variabel untuk debounce tombol SET
bool resetPressed = false; // Variabel untuk debounce tombol RESET
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*                                END OF Deklarasi libraries, definisi pin-pin dan alamat yang digunakan, dan inisiasi variabel-variabel penting                                  */
/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */
/*                                                                    Deklarasi fungsi-fungsi pada jam digital                                                                    */

// Deklarasi fungsi setup ----------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  // Inisiasi LCD
  lcd.begin(16, 2); // Inisialisasi LCD dengan 16 kolom dan 2 baris
  lcd.setBacklight(1); // Nyalakan backlight LCD

  // Set mode setiap tombol sebagai input dan BUZZER sebagai output 
  pinMode(MODE, INPUT);
  pinMode(SET, INPUT);
  pinMode(RESET, INPUT);
  pinMode(BUZZER, OUTPUT);

  // Setup Timer (0, 1, 2) dengan frekuensi 1000 Hz dan mode CTC
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;
  OCR0A = 249;
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (1 << CS01) | (1 << CS00);
  TIMSK0 |= (1 << OCIE0A);
  // ->
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 249;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  // ->
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  OCR2A = 124;
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS22) | (1 << CS20);
  TIMSK2 |= (1 << OCIE2A);
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi fungsi startup --------------------------------------------------------------------------------------------------------------------------------------------------------
void startup()
{
  // Set kursor dan cetak pesan "HELLO!"
  lcd.setCursor(5, 0);
  lcd.print("HELLO!");
  
  // Cetak pesan "starting" setelah 3 detik pesan "HELLO!" dicetak diikuti titik-titik
  while (dots < 4) // Selama jumlah titik < 4..
  {
    if (milidetikClock <= 750 && detikClock == 0) // Jika belum 750 ms startup..
    {
      digitalWrite(BUZZER, HIGH); // Bunyikan dahulu BUZZER, sebagai pengetesan apakah BUZZER bekerja
    }
    else // Jika sudah..
    {
      digitalWrite(BUZZER, LOW); // Matikan BUZZER
    }

    lcd.setCursor(0, 1); // Set kursor di 0, 1

    if (detikClock - 3 >= dots) // Jika "HELLO!" sudah ditampilkan selama 3 detik..
    {
      lcd.print("starting"); // Cetak pesan "starting"
      for(int i = 0; i < dots; i ++) // Cetak "." sebanyak dots
      {
        lcd.print(".");
      }
      dots++; // Tambah jumlah dots
    }
  }

  // Pindah ke state setting Jam setelah "starting..." ditampilkan sedetik
  if (detikClock >= 7)
  {
      state = 1;
      lcd.clear(); // Clear LCD setiap pindah state
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi fungsi mode Jam -------------------------------------------------------------------------------------------------------------------------------------------------------
void tampilkanWaktu()
{
  // Set kursor dan cetak header
  lcd.setCursor(1, 0);
  lcd.print("The time is...");

  // Atur tampilan waktu
  if (!showSeconds) // Jika detik ditampilkan..
  {
    lcd.setCursor(5, 1); // Set kursor di 5, 1
  }
  else // Jika tidak..
  {
    lcd.setCursor(4, 1); // Set kursor di 4, 1
  }
  // ->
  if (jamClock < 10) // Jika jam masih satuan..
  {
    lcd.print("0"); // Cetak "0" terlebih dahulu
  }
  lcd.print(jamClock); // Cetak nilai jam
  // ->
  if (tick || showSeconds) // Jika sedang tick atau detik ditampilkan..
  {
    lcd.print(":"); // Cetak pemisah ":"
  }
  else // Jika tidak..
  {
    lcd.print(" "); // Cetak " " sebagai kedip
  }
  // ->
  if (menitClock < 10) // Jika menit masih satuan..
  {
    lcd.print("0"); // Cetak "0" terlebih dahulu
  }
  lcd.print(menitClock); // Cetak nilai menit
  // ->
  if (showSeconds) // Jika detik ditampilkan....
  {
    lcd.print(":"); // Cetak pemisah ":"...
    if (detikClock < 10) // Apabila detik masih satuan..
    {
      lcd.print("0"); // Cetak "0" terlebih dahulu
    }
    lcd.print(detikClock); // Cetak nilai detik
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi fungsi setting Jam ----------------------------------------------------------------------------------------------------------------------------------------------------
void clockSetting()
{
  detikClock = 0; // Buat detik 0 terus-menerus sampai selesai atur Jam agar menghindari kasus menit ataupun jam terinkremen saat pengaturan

  // Setting waktu
  if (stateClockSetting == 1) // Jika sedang setting jam..
  {
    // Set kursor dan cetak header
    lcd.setCursor(2, 0);
    lcd.print("Set the hour");

    // Atur tampilan setting Jam
    lcd.setCursor(5, 1); // Set kursor di 5, 1
    // ->
    if (tick) // Jika sedang tick...
    {
      if (jamClock < 10) // Dan jika jam masih satuan..
      {
        lcd.print("0"); // Cetak "0" dahulu
      }
      lcd.print(jamClock); // Cetak nilai jam
    }
    else // Jika tidak..
    {
      lcd.print("  "); // Cetak "  " sebagai kedip
    }
    // ->
    lcd.print(":"); // Cetak pemisah ":"
    // ->
    if (menitClock < 10) // Jika menit masih satuan..
    {
      lcd.print("0"); // Cetak "0" dahulu
    }
    lcd.print(menitClock); // Cetak nilai menit
  }
  else if (stateClockSetting == 2) // Jika sedang setting menit..
  {
    // Set kursor dan cetak header
    lcd.setCursor(1, 0);
    lcd.print("Set the minute");

    // Atur tampilan setting Jam
    lcd.setCursor(5, 1); // Set kursor di 5, 1
    if (jamClock < 10) // Jika jam masih satuan..
    {
      lcd.print("0"); // Cetak "0" dahulu
    }
    lcd.print(jamClock); // Cetak nilai jam
    // ->
    lcd.print(":"); // Cetak pemisah ":"
    // ->
    if (tick) // Jika sedang tick...
    {
      if (menitClock < 10) // Dan jika menit masih satuan..
      {
        lcd.print("0"); // Cetak "0" dahulu
      }
      lcd.print(menitClock); // Cetak nilai menit
    }
    else // Jika tidak..
    {
      lcd.print("  "); // Cetak "  " sebagai kedip
    }
  }

  // Setting penekanan tombol
  if (digitalRead(RESET) == HIGH && stateClockSetting == 1 && !resetPressed) // Jika RESET baru ditekan dan ada di setting jam..
  {
    resetPressed = true; // Nyatakan RESET sudah ditekan
    jamClock++; // Inkremen jam
    if (jamClock >= 24) // Jika jam sudah mencapai 24..
    {
      jamClock = 0; // Reset jam ke 0
    }
  }
  else if (digitalRead(RESET) == HIGH && stateClockSetting == 2 && !resetPressed) // Jika RESET baru ditekan dan ada di setting menit.. 
  {
    
    resetPressed = true; // Nyatakan RESET sudah ditekan
    menitClock++; // Inkremen menit
    if (menitClock >= 60) // Jika menit sudah mencapai 60..
    {
      menitClock = 0; // Reset menit ke 0
    }
  }
  // ->
  if (digitalRead(MODE) == HIGH && stateClockSetting == 1 && !modePressed) // Jika MODE baru ditekan dan ada di setting jam..
  {
    modePressed = true; // Nyatakan MODE sudah ditekan
    stateClockSetting = 2; // Pindah ke setting menit
    lcd.clear(); // Clear LCD setiap pindah stateClockSetting
  }
  else if (digitalRead(MODE) == HIGH && stateClockSetting == 2 && !modePressed) // Jika MODE baru ditekan dan ada di setting menit..
  {
    modePressed = true; // Nyatakan MODE sudah ditekan 
    stateClockSetting = 1; // Pindah ke setting jam
    lcd.clear();
  }
  // ->
  if (digitalRead(MODE) == LOW) // Jika MODE tidak sedang ditekan..
  {
    modePressed = false; // Nyatakan MODE tidak ditekan
  }
  // ->
  if (digitalRead(RESET) == LOW) // Jika RESET tidak sedang ditekan..
  {
    resetPressed = false; // Nyatakan RESET tidak ditekan
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi fungsi mode Stopwatch -------------------------------------------------------------------------------------------------------------------------------------------------
void stopwatch()
{
  // Set kursor dan cetak header
  lcd.setCursor(1,0);
  lcd.print("Stopwatch mode");

  // Atur tampilan Stopwatch
  lcd.setCursor(2, 1); // Set kursor di 2, 1
  // ->
  if (jamSW < 10) // Jika jam masih satuan..
  {
    lcd.print("0"); // Cetak "0" dahulu
  }
  lcd.print(jamSW); // Cetak nilai jam
  // ->
  lcd.print(":"); // Cetak pemisah ":"
  // ->
  if (menitSW < 10) // Jika menit masih satuan..
  {
    lcd.print("0"); // Cetak "0" dahulu
  }
  lcd.print(menitSW); // Cetak nilai menit
  // ->
  lcd.print(":"); // Cetak pemisah ":"
  // ->
  if (detikSW < 10) // Jika detik masih satuan..
  {
    lcd.print("0"); // Cetak pemisah ":"
  }
  lcd.print(detikSW); // Cetak nilai detik
  // ->
  lcd.print("."); // Cetak pemisah "."
  // ->
  if (milidetikSW < 100) // Jika milidetik masih puluhan..
  {
    lcd.print("0"); // Cetak "0" dahulu
  }
  // ->
  if (milidetikSW < 10) // Jika milidetik masih satuan..
  {
    lcd.print("0"); // Cetak lagi "0" dahulu
  }
  lcd.print(milidetikSW); // Cetak nilai milidetik
  // ->
  lcd.setCursor(14, 1);
  lcd.print(" "); // Mengatasi bug kadang muncul angka '0' keempat di bagian milidetik

  // Setting penekanan tombol
  if (digitalRead(SET) == HIGH && !setPressed) // Jika SET baru ditekan..
  {
    setPressed = true; // Nyatakan SET sudah ditekan
    interrupt2Aktif = !interrupt2Aktif; // Negasikan kondisi Stopwatch (berjalan menjadi berhenti, berhenti menjadi berjalan)
  }
  // ->
  if (!interrupt2Aktif && digitalRead(RESET) == HIGH && !resetPressed) // Jika RESET baru ditekan dan Stopwatch berhenti..
  {
    resetPressed = true; // Nyatakan RESET sudah ditekan
    // Reset Stopwatch ke 0
    jamSW = 0;
    menitSW = 0;
    detikSW = 0;
    milidetikSW = 0;
  }
  // ->
  if (digitalRead(SET) == LOW) // Jika SET tidak sedang ditekan..
  {
    setPressed = false; // Nyatakan SET tidak ditekan
  }
  // ->
  if (digitalRead(RESET) == LOW) // Jika RESET tidak sedang ditekan..
  {
    resetPressed = false; // Nyatakan RESET tidak ditekan
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi fungsi mode Timer -----------------------------------------------------------------------------------------------------------------------------------------------------
void tampilkanTimer()
{
  // Set kursor dan cetak header
  if (interrupt3Aktif) // Jika Timer sedang berjalan..
  {
    lcd.setCursor(1, 0);
    lcd.print("Timer running"); // Cetak header Timer berjalan
  }
  else // Jika tidak..
  {
    lcd.setCursor(3, 0);
    lcd.print("Timer mode"); // Cetak header Timer berhenti
  }

  // Atur tampilan Timer
  lcd.setCursor(4, 1); // Set kursor di 4, 1
  // ->
  if (jamTimer < 10) // Jika jam masih satuan..
  {
    lcd.print("0"); // Cetak "0" dahulu
  }
  lcd.print(jamTimer); // Cetak nilai jam
  // ->
  lcd.print(":"); // Cetak pemisah ":"
  // ->
  if (menitTimer < 10) // Jika jam masih satuan..
  {
    lcd.print("0"); // Cetak "0" dahulu
  }
  lcd.print(menitTimer); // Cetak nilai menit
  // ->
  lcd.print(":"); // Cetak pemisah ":"
  // ->
  if (detikTimer < 10) // Jika detik masih satuan..
  {
    lcd.print("0"); // Cetak "0" dahulu
  }
  lcd.print(detikTimer); // Cetak nilai detik
  
  // Cetak " " untuk mengatasi bug terkadang ada tiga angka di bagian detik
  lcd.setCursor(12, 1);
  lcd.print(" ");

  // Setting penekanan tombol
  if (digitalRead(SET) == HIGH && !setPressed) // Jika SET baru ditekan..
  {
    if (!interrupt3Aktif && (milidetikTimer > 0 || detikTimer > 0 || menitTimer > 0 || jamTimer > 0)) // Jika Timer sedang berhenti dan masih belum 0..
    {
      setPressed = true; // Nyatakan SET sudah ditekan
      interrupt3Aktif = true; // Jalankan Timer
    }
    else if (interrupt3Aktif) // Jika Timer sedang berjalan..
    {
      setPressed = true; // Nyatakan SET sudah ditekan
      interrupt3Aktif = false; // Hentikan Timer
      lcd.clear();
    }
  }
  // ->
  if (digitalRead(RESET) == HIGH && resetPressed == false && !interrupt3Aktif) // Jika RESET baru ditekan dan Timer sedang berhenti..
  {
    resetPressed = true; // Nyatakan RESET sudah ditekan

    // Reset Timer ke 0
    milidetikTimer = 0;
    detikTimer = 0;
    menitTimer = 0;
    jamTimer = 0;
  }
  // ->
  if (digitalRead(SET) == LOW) // Jika SET tidak sedang ditekan..
  {
    setPressed = false; // Nyatakan SET tidak ditekan
  }
  // ->
  if(digitalRead(RESET) == LOW) // Jika RESET tidak sedang ditekan..
  {
    resetPressed = false; // Nyatakan RESET tidak ditekan
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi fungsi setting Timer --------------------------------------------------------------------------------------------------------------------------------------------------
void timerSetting()
{
  // Set kursor dan cetak header
  lcd.setCursor(1, 0);
  lcd.print("Set the timer");

  lcd.setCursor(4, 1); // Set kursor di 4, 1

  // Atur tampilan setting Timer
  if (stateTimerSetting == 1) // Jika sedang setting detik..
  {
    if (jamTimer < 10) // Jika jam masih satuan..
    {
      lcd.print("0"); // Cetak "0" dahulu
    }
    lcd.print(jamTimer); // Cetak nilai jam
    // ->
    lcd.print(":"); // Cetak pemisah ":"
    // ->
    if (menitTimer < 10) // Jika menit masih satuan..
    {
      lcd.print("0"); // Cetak "0" dahulu
    }
    lcd.print(menitTimer); // Cetak nilai menit
    // ->
    lcd.print(":"); // Cetak pemisah ":"
    // ->
    if (tick) // Jika sedang tick...
    {
      if (detikTimer < 10) // Dan jika detik masih satuan..
      {
        lcd.print("0"); // Cetak "0" dahulu
      }
      lcd.print(detikTimer); // Cetak nilai detik
    }
    else // Jika tidak..
    {
      lcd.print("  "); // Cetak "  " sebagai kedip
    }
  }
  else if (stateTimerSetting == 2) // Jika sedang setting menit
  {
    if (jamTimer < 10) // Jika jam masih satuan..
    {
      lcd.print("0"); // Cetak "0" dahulu
    }
    lcd.print(jamTimer); // Cetak nilai jam
    // ->
    lcd.print(":"); // Cetak pemisah ":"
    // ->
    if (tick) // Jika sedang tick...
    {
      if (menitTimer < 10) // Dan jika menit masih satuan..
      {
        lcd.print("0"); // Cetak "0" dahulu
      }
      lcd.print(menitTimer); // Cetak nilai menit
    }
    else // Jika tidak..
    {
      lcd.print("  "); // Cetak "  " sebagai kedip
    }
    // ->
    lcd.print(":"); // Cetak pemisah ":"
    // ->
    if (detikTimer < 10) // Jika detik masih satuan..
    {
      lcd.print("0"); // Cetak "0" dahulu
    }
    lcd.print(detikTimer); // Cetak nilai detik
  }
  else if (stateTimerSetting == 3) // Jika sedang setting jam
  {
    if (tick) // Jika sedang tick...
    {
      if (jamTimer < 10) // Dan jika jam masih satuan..
      {
        lcd.print("0"); // Cetak "0" dahulu
      }
      lcd.print(jamTimer); // Cetak nilai jam
    }
    else // Jika tidak..
    {
      lcd.print("  "); // Cetak "  " sebagai kedip
    }
    // ->
    lcd.print(":"); // Cetak pemisah ":"
    // ->
    if (menitTimer < 10) // Jika menit masih satuan..
    {
      lcd.print("0"); // Cetak "0" dahulu
    }
    lcd.print(menitTimer); // Cetak nilai menit
    // ->
    lcd.print(":"); // Cetak pemisah ":"
    // ->
    if (detikTimer < 10) // Jika detik masih satuan..
    {
      lcd.print("0"); // Cetak "0" dahulu
    }
    lcd.print(detikTimer); // Cetak nilai detik
  }

  // Setting penekanan tombol
  if (digitalRead(RESET) == HIGH && stateTimerSetting == 1 && !resetPressed) // Jika RESET baru ditekan dan ada di setting detik..
  {
    resetPressed = true; // Nyatakan RESET sudah ditekan
    detikTimer++; // Inkremen detik
    if (detikTimer >= 60) // Jika detik sudah mencapai 60..
    {
      detikTimer = 0; // Reset detik ke 0
    }
  }
  else if (digitalRead(RESET) == HIGH && stateTimerSetting == 2 && !resetPressed) // Jika RESET baru ditekan dan ada di setting menit..
  {
    resetPressed = true; // Nyatakan RESET sudah ditekan
    menitTimer++; // Inkremen menit
    if (menitTimer >= 60) // Jika menit sudah mencapai 60..
    {
      menitTimer = 0; // Reset menit ke 0
    }
  }
  else if (digitalRead(RESET) == HIGH && stateTimerSetting == 3 && !resetPressed) // Jika RESET baru ditekan dan ada di setting jam..
  {
    resetPressed = true; // Nyatakan RESET sudah ditekan
    jamTimer++; // Inkremen jam
    if (jamTimer >= 24) // Jika jam sudah mencapai 24..
    {
      jamTimer = 0; // Reset jam ke 0
    }
  }
  // ->
  if (digitalRead(MODE) == HIGH && stateTimerSetting == 1 && !modePressed) // Jika MODE baru ditekan dan ada di setting detik..
  {
    modePressed = true; // Nyatakan MODE sudah ditekan
    stateTimerSetting = 2; // Pindah ke setting menit
    lcd.clear(); // Clear LCD setiap pindah stateTimerSetting
  }
  else if (digitalRead(MODE) == HIGH && stateTimerSetting == 2 && !modePressed) // Jika MODE baru ditekan dan ada di setting menit..
  {
    modePressed = true; // Nyatakan MODE sudah ditekan
    stateTimerSetting = 3; // Pindah ke setting jam
    lcd.clear();
  }
  else if (digitalRead(MODE) == HIGH && stateTimerSetting == 3 && !modePressed) // Jika MODE baru ditekan dan ada di setting jam..
  {
    modePressed = true; // Nyatakan MODE sudah ditekan
    stateTimerSetting = 1; // Pindah ke setting detik
    lcd.clear();
  }
  // ->
  if (digitalRead(RESET) == LOW) // Jika RESET tidak sedang ditekan..
  {
    resetPressed = false; // Nyatakan RESET tidak ditekan
  }
  // ->
  if (digitalRead(MODE) == LOW) // Jika MODE tidak sedang ditekan..
  {
    modePressed = false; // Nyatakan MODE tidak ditekan
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi fungsi mode Buzzer ---------------------------------------------------------------------------------------------------------------------------------------------------------
void buzz()
{
  // Clear LCD setelah Timer habis dan reset Timer ke 0
  lcd.clear();
  milidetikTimer = 0;
  detikTimer = 0;
  menitTimer = 0;
  jamTimer = 0;

  // Atur tampilan Buzzer
  while (buzzCount < 12) // Selama jumlah buzz belum 11..
  {
    // Set kursor dan cetak header
    lcd.setCursor(4, 0);
    lcd.print("Time up!");

    // Atur tampilan Timer
    lcd.setCursor(4, 1);
    if (beep) // Jika sedang beep..
    {
      // Cetak Timer yang sudah 0 dan bunyikan BUZZER
      lcd.print("00:00:00");
      digitalWrite(BUZZER, HIGH);
    }
    else // Jika tidak..
    {
      // Cetak Timer dengan nilai kosong sebagai kedip dan matikan BUZZER
      lcd.print("  :  :  ");
      digitalWrite(BUZZER, LOW);
    }
  }
  
  interrupt3Aktif = false; // Hentikan Timer
  buzzCount = 0; // Reset jumlah buzz ke 0
  digitalWrite(BUZZER, LOW); // Matikan BUZZER

  // Reset Timer ke 0
  milidetikTimer = 0;
  detikTimer = 0;
  menitTimer = 0;
  jamTimer = 0;

  // Pindah ke state sebelum mode Buzzer yang disimpan di variabel prebuzz
  state = prebuzz;
  lcd.clear();
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*                                                                 END OF Deklarasi fungsi-fungsi pada jam digital                                                                */
/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */
/*                                                                       Deklarasi ISR-ISR timer interrupt                                                                        */

// Deklarasi ISR untuk mode Jam ----------------------------------------------------------------------------------------------------------------------------------------------------
ISR(TIMER0_COMPA_vect) 
{
  if (interrupt1Aktif) // Jika Jam berjalan...
  {
    if (milidetikClock % 500 == 0) // Dan jika milidetik kelipatan 500..
    {
      tick = !tick; // Negasikan tick
    }
    
    milidetikClock++; // Inkremen milidetik
    if (milidetikClock >= 1000) // Jika milidetik mencapai 1000..
    {
      milidetikClock = 0; // Reset milidetik ke 0
      detikClock++; // Inkremen detik
      if (detikClock >= 60) // Jika detik mencapai 60..
      {
        detikClock = 0; // Reset detik ke 0
        menitClock++; // Inkremen menit
        if (menitClock >= 60) // Jika menit mencapai 60..
        {
          menitClock = 0; // Reset menit ke 0
          jamClock++; // Inkremen jam
          if (jamClock >= 24) // Jika jam mencapai 24..
          {
            jamClock = 0; // Reset jam ke 0
          }
        }
      }
    }
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi ISR untuk mode Stopwatch ----------------------------------------------------------------------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect) 
{
  if (interrupt2Aktif) // Jika Stopwatch berjalan..
  {
    milidetikSW++;  // Inkremen milidetik
    if (milidetikSW >= 1000) // Jika milidetik mencapai 1000..
    {
      milidetikSW = 0; // Reset milidetik ke 0
      detikSW++; // Inkremen menit
      if (detikSW >= 60) // Jika detik mencapai 60..
      {
        detikSW = 0; // Reset detik ke 0
        menitSW++; // Inkremen menit
        if (menitSW >= 60) // Jika menit mencapai 60..
        {
          menitSW = 0; // Reset menit ke 0
          jamSW++; // Inkremen jam
          if (jamSW >= 24) // Jika jam mencapai 24..
          {
            jamSW = 0; // Reset jam ke 0
          }
        }
      }
    }
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Deklarasi ISR untuk mode Timer
ISR(TIMER2_COMPA_vect) 
{
  if (interrupt3Aktif && detikTimer <= 0 && menitTimer <= 0 && jamTimer <= 0) // Jika Timer sedang berjalan dan Timer sudah mencapai 0...
  {
    if (state != 5) // Dan jika state saat ini bukan mode Buzzer..
    {
      prebuzz = state; // Simpan state terakhir sebelum masuk ke mode Buzzer
    }

    if (milidetikTimer - 500 >= 0) // Jika milidetik sudah mencapai 500..
    {
      beep = !beep; // Negasikan beep
      buzzCount++; // Inkremen jumlah buzz
      milidetikTimer = 0; // Reset milidetik ke 0
    }
    state = 5; // Ubah ke state mode Buzzer
    milidetikTimer++; // Inkremen milidetik
  }
  else if (interrupt3Aktif) // Jika Timer sedang berjalan..
  {
    milidetikTimer--; // Dekremen milidetik
    if (milidetikTimer < 0 && (detikTimer >= 1 || menitTimer >= 1 || jamTimer >= 1)) // Jika milidetik kurang dari 0 tetapi Timer belum 0..
    {
      milidetikTimer = 999; // Set milidetik ke 999
      detikTimer--; // Dekremen detik
      if (detikTimer < 0 && (menitTimer >= 1 || jamTimer >= 1)) // Jika detik kurang dari 0 tetapi Timer belum 0..
      {
        detikTimer = 59; // Set detik ke 59
        menitTimer--; // Dekremen menit
        if (menitTimer < 0 && jamTimer >= 1) // Jika menit kurang dari 0 tetapi Timer belum 0..
        {
          menitTimer = 59; // Set menit ke 59
          jamTimer--; // Dekremen jam
          if (jamTimer <= 0) // Jika jam kurang dari 0..
          {
            jamTimer = 0; // Reset jam ke 0
          }
        }
        else if (menitTimer < 0) // Jika tidak..
        {
          menitTimer = 0; // Reset menit ke 0
        }
      }
      else if (detikTimer < 0) // Jika tidak..
      {
        detikTimer = 0; // Reset detik ke 0
      }
    }
    else if (milidetikTimer < 0) // Jika tidak..
    {
      milidetikTimer = 0; // Reset milidetik ke 0
    }
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*                                                                  END OF Deklarasi ISR-ISR timer interrupt                                                                      */
/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */
/*                                                                  Deklarasi fungsi utama loop() jam digital                                                                     */

// Deklarasi fungsi loop() ---------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  // Jika state startup..
  if (state == -1)
  {
    startup(); // Jalankan fungsi startup()
  }
  
  // Jika state mode Jam..
  if (state == 0)
  {
    tampilkanWaktu(); // Jalankan fungsi tampilkanWaktu()

    // Setting penekanan tombol
    if (digitalRead(SET) == HIGH && !setPressed) // Jika SET baru ditekan..
    {
      Serial.print("Pushed");
      showSeconds = false; // Buat tampilan Jam tanpa detik, agar tidak terjadi kasus penimpaan tampilan antara tampilan dengan dan tanpa detik saat pindah ke mode Jam lagi
      setPressed = true; // Nyatakan SET sudah ditekan
      stateClockSetting = 1; // Reset stateClockSetting ke 1
      state = 1; // Pindah ke setting Jam
      lcd.clear();
    }
    // ->
    if (digitalRead(MODE) == HIGH && !modePressed) // Jika MODE baru ditekan..
    {
      Serial.print("Pushed");
      showSeconds = false; // Buat tampilan Jam tanpa detik, agar tidak terjadi kasus penimpaan tampilan antara tampilan dengan dan tanpa detik saat pindah ke mode Jam lagi
      modePressed = true; // Nyatakan MODE sudah ditekan
      state = 2; // Pindah ke mode Stopwatch
      lcd.clear();
    }
    // ->
    if (digitalRead(RESET) == HIGH && !resetPressed) // Jika RESET baru ditekan..
    {
      Serial.print("Pushed");
      resetPressed = true; // Nyatakan RESET sudah ditekan
      showSeconds = true; // Buat agar Jam menampilkan detik
    }
    // ->
    if (digitalRead(MODE) == LOW) // Jika MODE tidak sedang ditekan..
    {
      modePressed = false; // Nyatakan MODE tidak ditekan
    }
    // ->
    if (digitalRead(SET) == LOW) // Jika SET tidak sedang ditekan..
    {
      setPressed = false; // Nyatakan SET tidak ditekan
    }
    // ->
    if (digitalRead(RESET) == LOW) // Jika RESET tidak sedang ditekan..
    {
      if (resetPressed)
      {
        lcd.clear(); // Clear LCD sebelum detik disembunyikan
      }
      resetPressed = false; // Nyatakan RESET tidak ditekan
      showSeconds = false; // Matikan penampilan detik pada Jam
    }
  }

  // Jika state setting Jam..
  if (state == 1)
  {
    clockSetting(); // Jalankan fungsi clockSetting()

    // Setting penekanan tombol
    if (digitalRead(SET) == HIGH && !setPressed) // Jika SET baru ditekan..
    {
      // Reset detik dan milidetik ke 0
      milidetikClock = 0;
      detikClock = 0;
      Serial.print("Pushed");
      
      setPressed = true; // Nyatakan SET sudah ditekan
      state = 0; // Kembali ke state mode Jam
      lcd.clear();
    }
    // ->
    if (digitalRead(SET) == LOW) // Jika SET tidak sedang ditekan..
    {
      setPressed = false; // Nyatakan SET tidak ditekan
    }
  }

  // Jika state mode Stopwatch..
  if (state == 2)
  {
    stopwatch(); // Jalankan fungsi stopwatch()
    
    // Setting penekanan tombol
    if (digitalRead(MODE) == HIGH && !modePressed) // Jika MODE baru ditekan..
    {
      modePressed = true; // Nyatakan MODE sudah ditekan
      state = 3; // Pindah ke state mode Timer
      Serial.print("Pushed");
      lcd.clear();
    }
    // ->
    if (digitalRead(MODE) == LOW) // Jika MODE tidak sedang ditekan..
    {
      modePressed = false; // Nyatakan MODE tidak ditekan
    }
  }

  // Jika state mode Timer..
  if (state == 3)
  {
    tampilkanTimer(); // Jalankan fungsi tampilkanTimer()

    // Setting penekanan tombol
    if (digitalRead(MODE) == HIGH && !modePressed) // Jika MODE baru ditekan..
    {
      modePressed = true; // Nyatakan MODE sudah ditekan
      state = 0; // Pindah ke state mode Jam
      Serial.print("Pushed");
      lcd.clear();
    }
    // ->
    if (digitalRead(SET) == HIGH && !interrupt3Aktif && !setPressed && milidetikTimer <= 0 && detikTimer <= 0 && menitTimer <= 0 && jamTimer <= 0) // Jika SET baru
    // ditekan, Timer sedang berhenti, dan Timer sudah habis (atau masih 0)..
    {
      setPressed = true; // Nyatakan SET sudah ditekan
      Serial.print("Pushed");

      // Pindah ke state setting Timer
      state = 4;
      stateTimerSetting = 1;
      lcd.clear();
    }
    // ->
    if (digitalRead(MODE) == LOW) // Jika MODE tidak sedang ditekan..
    {
      modePressed = false; // Nyatakan MODE tidak ditekan
    }
    // ->
    if (digitalRead(SET) == LOW) // Jika SET tidak sedang ditekan..
    {
      setPressed = false; // Nyatakan SET tidak ditekan
    }
  }

  // Jika state setting Timer..
  if (state == 4)
  {
    timerSetting(); // Jalankan fungsi timerSetting()

    // Setting penekanan tombol
    if (digitalRead(SET) == HIGH && !setPressed) // Jika SET baru ditekan..
    {
      setPressed = true; // Nyatakan SET sudah ditekan
      if (detikTimer > 0 || menitTimer > 0 || jamTimer > 0) // Jika Timer yang diatur bukan 0..
      {
        milidetikTimer = 1000; // Tambahkan milidetikTimer dengan nilai 1000. Hal ini agar, saat Timer berjalan, Timer tidak terlihat mulai dengan dikurangi satu detik dan agar
        // langsung berhenti saat detik bernilai 0
        interrupt3Aktif = true; // Jalankan Timer
        state = 3; // Pindah ke state mode Timer
        lcd.clear();
      }
      else // Jika tidak..
      {
        state = 3; // Pindah ke state mode Timer tanpa menjalankan Timer
        lcd.clear();
      }
    }
    // ->
    if (digitalRead(SET) == LOW) // Jika SET tidak sedang ditekan..
    {
      setPressed = false; // Nyatakan SET tidak ditekan
    }
  }

  // Jika state mode Buzzer..
  if (state == 5)
  {
    buzz(); // Jalankan fungsi buzz()
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*                                                              END OF Deklarasi fungsi utama loop() jam digital                                                                  */

/* ############################################################################################################################################################################## */
/* ############################################################################ END OF PROGRAM ################################################################################## */
/* ############################################################################################################################################################################## */