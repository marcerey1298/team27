#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library

void setup() {
  Serial.begin(9600); // use the serial port
}
int l = 0;
void loop() {
  while(1) {
    cli();
    for (int i = 0 ; i < 512 ; i += 2) {
      fft_input[i] = analogRead(A0); // <-- NOTE THIS LINE
      fft_input[i+1] = 0;
    }
    fft_window();
    fft_reorder();
    fft_run();
    fft_mag_log();
    sei();

    for (int i = 0 ; i < FFT_N/2 ; i++) {
      Serial.print(fft_log_out[i]);
      Serial.print(' ');
    }
    Serial.println();
    l = l + 1;
    if (l == 20) {
      delay(100000000);
    }
  }
}
