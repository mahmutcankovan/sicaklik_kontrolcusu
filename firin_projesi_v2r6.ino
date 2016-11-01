//v2r6 revlist
//Menü ekranı sürekli yenilenme kaldırıldı
//pişirim anı sürekli yenileme kaldırıldı. 
//
//

#include <LiquidCrystal.h>
#include <Bounce2.h>

//LCD Ekran
LiquidCrystal lcd(9, 8, 5, 4, 3, 2);

//Debounce
Bounce debounce_ileri = Bounce();
Bounce debounce_geri = Bounce();


//Çıkış Pin Tanımlama
#define role_1 10
#define role_2 11
#define role_3 12
#define test_led 13

//Buton Pin Tanımlama  // Başına A gelebilir. Test etmek Gerek //
#define set_pot A0
#define ileri A1
#define geri A2



// Değişkenler
int t_max_pot;                                        // Maksimum sıcaklık Pot okunan değer
int t_max_pot_old;
int t_ramp_pot;                                       // Sıcaklık rampası Pot okunan değer
int t_ramp_pot_old;
int t_time_pot;                                       // Pişirim süresi Pot okunan değer
int t_time_pot_old;
int t_max;                                            // Maksimum sıcaklık
int t_ramp;                                           // Sıcaklık rampası
int t_ramp_basamak;                                   // Rampa basamak değeri
int t_ramp_anlik= 0;                                  // Rampanın o anki sıcaklığı
int t_olcum_periyot = 120;                            // Rampanın ölçüm periyodu (60 => dakikadada bir; 120 => 30sn'de bir)

//Zaman Değişkenleri
unsigned long t_time;                                 // Pişirim süresi dk
unsigned long t_pisirim_baslangic = 0;                //Pişirim başlangıç anı ms
unsigned long t_pisirim_bitis;                        // pisirim bitis anı
unsigned long t_kontrol_zaman= 30000;                 // Sıcaklık kontrol zamanları ms
unsigned long t_kontrol_zaman_periyot = 30000;        // 30sn'de bir için 30000 ms
unsigned long t_kalan;                                // Pişirim kalan süre dk
unsigned long t_anlik_zaman;                          // Pişirim gerçek zaman
unsigned long lcd_clear_time;
unsigned long lcd_clear_gap = 1000;                   //LCD Temizleme periyodu

int t_instant_analog;                                 // Termokupl okunan değer
int t_instant;                                        // Termokupl sıcaklık
int menu_sayac = 1;                                   //Menu No

//Programı iptal etmek için
int buttonCase = 0;                                   //Butonun başlangıç değerini LOW olarak atadık.
long lastTime = 0;                                    //En son butona basma zamanını bu değişkende tutacağız.

boolean lcd_clear_flag;
char relay_status = 80;
void setup() {

  //Pin ayarları
  pinMode(A0, INPUT);  //Pot
  pinMode(A1, INPUT);  //İleri Butonu
  pinMode(A2, INPUT);  //Geri Butonu
  pinMode(A3, INPUT);  //Pot?
  
  pinMode(10, OUTPUT); //Röle 1
  pinMode(11, OUTPUT); //Röle 2
  pinMode(12, OUTPUT); //Röle 3

   //Röle Pin Başlangıç Ayarları
  digitalWrite(role_1, LOW);
  digitalWrite(role_2, LOW);
  digitalWrite(role_3, LOW);

  //Buton Debounce Süreleri
  debounce_ileri.attach(ileri);
  debounce_ileri.interval(10);
  
  debounce_geri.attach(geri);
  debounce_geri.interval(10);

  //LCD İlk Ayarlar
  lcd.begin(16, 2);                                   // LCD sütun satır sayısı
  //Başlangıç Menusu
  lcd.print("42 v2r6");
  delay (1000);
  lcd.clear();



  //Seriport Veri Okuma
  //Serial.begin(9600);
  //Serial.write("2");

}

void loop() {

  //buton okumalar
debounce_ileri.update();
debounce_geri.update();

int ileri_db = debounce_ileri.rose();
int geri_db = debounce_geri.rose();



//      t_kontrol_zaman = 30000;                       // Her dakika kontrol için ilk zaman aralığı girildi
//      lcd.clear();
//    }
//    menu_sayac++;
//  }
//  else if (geri_buton == HIGH)
//  {
//    menu_sayac--;
//  }


  //Menü seçimi
  if (ileri_db == HIGH)
  {

    menu_sayac++;
    lcd_clear_flag = false;
    
  }
  else if (geri_db == HIGH)
  {
   menu_sayac--;
   lcd_clear_flag = false;
   
  }

  //menu sayacının eksilere düşmemesi için
  if (menu_sayac <= 0 )
  {
    menu_sayac = 1;
  }
  //menu sayacının 6'ten ileri gitmemesi için
  if (menu_sayac >= 6) 
  {
    menu_sayac = 6;
  }
  //Menuler
  switch (menu_sayac)
  {
    case 1: //maksimum sıcaklık değeri giriş ekranı

      if (lcd_clear_flag == false)
      {
        lcd.clear();
        lcd_clear_flag = true;
      }
      lcd.setCursor(0, 0);
      lcd.print("pisirmeSicakligi");
      //Pisirme sıcakkığı
      lcd.setCursor(0, 1);
      t_max_pot = analogRead(set_pot);
      t_max = t_max_pot * 1;  
      
      if (t_max>1000)                                 // Sıcaklık değerinin 100 ile 1200 arasında olması için
      {
        t_max=1000;
      }
      else if (t_max<100)
      {
        t_max=100;
      }
      if (t_max_pot_old != t_max_pot)
      {
        lcd.clear();
         t_max_pot_old = t_max_pot;
      }
      lcd.print(t_max);
      Serial.write("  sicaklik:");
      Serial.print(t_max);
      break;

    case 2: //sıcaklık rampası değeri giriş ekranı santigrat derece/saat
      if (lcd_clear_flag == false)
      {
        lcd.clear();
        lcd_clear_flag = true;
      }
      lcd.setCursor(0, 0);
      lcd.print("Rampa t/saat");
      //rampa süresi
      lcd.setCursor(0, 1);
      t_ramp_pot = analogRead(set_pot);
      t_ramp = t_ramp_pot * 1;
       if (t_ramp>1000)                                 // rampa değerinin 100 ile 1200 arasında olması için
      {
        t_ramp=1000;
      }
      else if (t_ramp<100)
      {
        t_ramp=100;
      }
            if (t_ramp_pot_old != t_ramp_pot)
      {
        lcd.clear();
         t_ramp_pot_old = t_ramp_pot;
      }
      lcd.print(t_ramp);
      break;

    case 3: //pisirme süresi değeri giriş ekranı
      if (lcd_clear_flag == false)
      {
        lcd.clear();
        lcd_clear_flag = true;
      }
      lcd.setCursor(0, 0);
      lcd.print("Pisirim Suresi");
      //pisirme süresi
      lcd.setCursor(0, 1);
      t_time_pot = analogRead(set_pot);
      t_time = t_time_pot;
      if(t_time<1)                                      // Süre sınırı 1-600 dk arasında sınırlandırıldı.
      {
      t_time = 1;
      }
      else if (t_time>600)
      {
      t_time = 600;                                     // Dakika cinsınden giriş
      }
      if (t_time_pot_old != t_time_pot)
      {
        lcd.clear();
         t_time_pot_old = t_time_pot;
      }
      lcd.print(t_time);
      break;

    case 4: //onay ekranı
      if (lcd_clear_flag == false)
      {
        lcd.clear();
        lcd_clear_flag = true;
      }
      lcd.setCursor(0, 0);
      lcd.print("Ayarlari Onayla");
      lcd.setCursor(0, 1);
      lcd.print(t_max);
      lcd.setCursor(5, 1);
      lcd.print(t_ramp);
      lcd.setCursor(10, 1);
      lcd.print(t_time);
      break;
  }

if (menu_sayac==5)
{
  lcd.clear();
  menu_sayac=6;
  lcd_clear_flag = false;
  }
while (menu_sayac == 6)
{
  pisirim();
}
} //loop kapatma süslü parantezi


void pisirim()   //Fırın Pişirim
{
//Saat Dakika Sayacı
  if (t_pisirim_baslangic == 0)
  {
  t_pisirim_baslangic = millis();
  t_kontrol_zaman = 30000;
  }
  
//  t_time = t_time*60000 ;                                //Seçilen pişirim süresi ms cinsine çevrildi.
  t_pisirim_bitis = t_pisirim_baslangic + t_time*60000;  
  t_kalan = (t_pisirim_bitis - millis())/ 60000;                  // Ekranda kalan süreyi göstermek için
//  t_kalan = t_kalan / 60000;

//Zaman bitimi ve bitiş fonksiyonuna geçiş
  if (millis() > t_pisirim_bitis && t_pisirim_baslangic != 0)
  {
//    lcd.clear();
//    bitis();
  }
  else
  {
//Do Nothing
  }

//Sabit Metinler
      lcd.setCursor(0,0);
      lcd.print("Sicaklik:");
      lcd.setCursor(14,0);
      lcd.print("R");
      lcd.setCursor(0,1);
      lcd.print("Kalan Sure:");

  

  //****************Buton durumu okuma 3 sn algılama için************************

  unsigned long startTime = millis() ;
  long highTime = 0;
  buttonCase = digitalRead(A2);

  boolean isFirstPress = false;
  boolean isStop = false;
  unsigned long goalTime;
  while (!isStop && (buttonCase == HIGH) )
  {
    if (isFirstPress == false)
    {
      isFirstPress = true;
      goalTime = millis() + 3000;
    }
    else {
      if (millis() >= goalTime)
      {
        isStop = 1;
      }
      else
      {
        lcd.setCursor(0, 1);
        lcd.print("            "); // ekranda bir önceli değişkeni silmek için    
        lcd.setCursor(0, 0);
        lcd.print(" iptal ediliyor");
      
      }
    }

    buttonCase = digitalRead(A2);
  }
  if (isStop)
  {
    lcd.clear();
    menu_sayac = 1;
  }
  //*****************Sıcaklık Hesap*******************
  
  t_ramp_basamak = t_ramp/t_olcum_periyot; // Basamak adımı belirle
  t_instant_analog = analogRead(A3); //Termokupl'dan sıcaklık analog oku
  t_instant = t_instant_analog*1;  // Gerçek değere çevir
  
  //t_pisirim_baslangic değişkeni referans zaman olarak alındı  
  t_anlik_zaman = millis();  // Çalışma anındaki zaman alındı
  if (t_pisirim_baslangic + t_kontrol_zaman < t_anlik_zaman)  // zamana bak, eğer süre dolduysa kontrol noktasını bir dk arttır
  {
    t_kontrol_zaman = t_kontrol_zaman + 30000;
    t_ramp_anlik = t_ramp_anlik + t_ramp_basamak; 
    
    if (t_instant >  t_ramp_anlik)                // Eğer anlık sıcaklık değeri rampadan büyükse enerjiyi kes
    {
      digitalWrite(10, LOW);                      //SSR1
      digitalWrite(11, LOW);                      //SSR2
      digitalWrite(12, LOW);                      //SSR3
      digitalWrite(13, LOW);                      //LED
      relay_status = 80;
      
//      lcd.setCursor (15,0);
//      lcd.print("P");
      
    }
    else                                                 //Diğer durumlarda sistemi enerjili bırak
    {
      digitalWrite(10, HIGH);                      //SSR1
      digitalWrite(11, HIGH);                      //SSR2
      digitalWrite(12, HIGH);                      //SSR3
      digitalWrite(13, HIGH);                      //LED

      relay_status = 65;
//      lcd.setCursor (15,0);
//      lcd.print("A");
      
    }
  }   

  //Ekrandaki değişkenler

if (lcd_clear_flag == false)
{
  lcd.clear();
  lcd_clear_flag = true;
  lcd_clear_time = millis()+lcd_clear_gap;
  lcd.setCursor(9,0);
  lcd.print(t_instant);
  lcd.setCursor(11,1);
  lcd.print(t_kalan);
  lcd.setCursor (15,0);
  lcd.print(relay_status);
}


  
if (lcd_clear_flag && (lcd_clear_time<millis()))  
{
  lcd_clear_flag = false;
}
  
}//void pisirim kapanis

//void bitis()
//{
//  //lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print("Pisirim Tamamlandi");
//}

