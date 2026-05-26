#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

// Kita bikin fungsi custom namanya "ngetikWA"
void ngetikWA(String teks) {
  for (int i = 0; i < teks.length(); i++) {
    if (teks[i] == '\n') { 
      // Kalau kodenya nemu karakter enter (\n), paksa jadi Shift+Enter
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press(KEY_RETURN);
      Keyboard.releaseAll();
      delay(50); // Jeda dikit biar WA nggak error nerima input
    } else {
      // Kalau huruf biasa, ketik aja langsung
      Keyboard.print(teks[i]);
    }
  }
  
  delay(500); // Tarik napas bentar setelah selesai ngetik
  Keyboard.write(KEY_RETURN); // Eksekusi Enter terakhir buat NGIRIM pesan
}

void setup() {
  Keyboard.begin();
  USB.begin();
  
  // Kasih jeda 3 detik awal pas dicolok
  delay(3000);

  // --- LU TINGGAL CUSTOM TEKSNYA DI SINI ---
  // Gunakan \n untuk pindah baris
  String pesanGua = "Halo! Kenalan dong, ini kontak saya:\n\nWA: 089505060252\nTikTok: https://www.tiktok.com/@something127.0.0.1\nIG: https://www.instagram.com/apocalypse_127.0.0.1/\nYouTube: https://www.youtube.com/@bagusdharma4979\n\nSave ya! wkwk";

  // Panggil fungsi saktinya
  ngetikWA(pesanGua);
}

void loop() {
  // Biarin kosong
}