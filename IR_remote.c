/*
contato: c00f3r@gmail.com ou acosta@conviso.com.br
autor: Antonio Costa aka Cooler_

Programa para passar slides usando controle remoto,
usando um arduino com seguinte código
---
#include <IRremote.h>

int RECV_PIN = 6;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); 
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(String(results.value,HEX) );
   
    irrecv.resume(); 
  } else {
    Serial.println("0");
  }   
  delay(850);
}
---

para compilar
gcc controle.c -o controle -lX11 -lXtst -Wall

voce vai precisar da libX11-dev e libxtst-dev

apt-get install libX11-dev libxtst-dev

Obrigado ao I4K "Tiago Natel" por dar a dica da lib Xtst e X11,
Obrigado a Galera que sempre me ajuda ai,
m0nad,sigsegv,slyfunky,otacon_x32,eremitah,clandestine,f117,bsdaemon,colt7r...

 */
#include <stdio.h>    
#include <stdlib.h> 
#include <string.h>   
#include <unistd.h> 
#include <alloca.h>
#include <termios.h>  
#include <fcntl.h> 
#include <time.h>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

#define ENTER() type_key(XK_Linefeed)

// sempre concatene com 'B' a  taxa de transmissão ,"9600" padrão do AVR
#define BAUDRATE B9600

// macros debug
#define BUGVIEW 1

#define DEBUG(x, s...) do { \
 if (!BUGVIEW) { break; } \
 time_t t = time(NULL); \
 char *d = ctime(&t); \
 fprintf(stderr, "%.*s %s[%d] %s(): ", \
 (int)strlen(d) - 1, d, __FILE__, \
 __LINE__, __FUNCTION__); \
 fprintf(stderr, x, ## s); \
} while (0);

// vai pegar os dados a cada 5 segundos
#define SECOND 5

Display *display;
int foo;
Window winFocus;
int revert;

void type_key(unsigned c); 
int serialboot(const char* serialport, int baud);
int serialread(int fd, char* buf, char until,int max);
int WriteFile(char *file,char *str);

void banner() 
{
 fprintf(stdout,"\nFollow patern: ./This_Code <SerialPort>\n"
  "Controlador de slides para palestras :-] v0.1 !!!\n"
  "Coded By Cooler_\n"
  "\n");
}


int main(int argc, char *argv[]) 
{
 int baudrate = BAUDRATE,fd=0; 
 char buf[64];
 char *serialport=(char *)alloca(sizeof(char)*512);


  if(argc<2) 
  {
   banner();
   exit(EXIT_SUCCESS);
  }

   
  if(strnlen(argv[1],511)<511)
  {
// iniciamos detalhes para automação das teclas
   if((display=XOpenDisplay(NULL)) == NULL) {
    fprintf(stderr, "%s: can't open %s\n", argv[0], XDisplayName(NULL));
    exit(1);
   }

   if(XTestQueryExtension(display, &foo, &foo, &foo, &foo) == False) {
    fprintf(stderr,"XTEST extension missing\n");
    exit(1);
   }
   XGetInputFocus(display, &winFocus, &revert);
 
   strncpy(serialport,argv[1],510);
   fd=serialboot(serialport, baudrate);
   fprintf(stdout,"Serial:%s\n",serialport);

   if(fd<=0)
   {
    DEBUG("veja se o dispositivo esta conectado!!");
    DEBUG("%d\n",fd);
    exit(EXIT_SUCCESS);
   }

   
   while(1) 
   {
    bzero(buf, sizeof(char)*63);
// efetuamos leitura e deixamos em "buf"
    serialread(fd, buf, '\n',62);
    usleep(620000);
    puts(buf);
     if(strlen(buf)>3)
     {
      fprintf(stdout,"\n string: %s \n tamanho: %d",buf,strlen(buf));

// button number 1 , buf == E13DDA28 então...
      if(strstr(buf,"FF30CF") || strstr(buf,"E13DDA28"))
      {
// se apertar o botão 1 no controle da samsung ou LG vai simular a tecla "k"(vai voltar slides)
       type_key('k');
      }

// button number 2
      if(strstr(buf,"FF18E7") || strstr(buf,"AD586662"))
      {
// se apertar o botão 2 no controle da samsung ou LG vai simular a tecla "l"(vai passar slides)
       type_key('l');
      }

// button number 3
      if(strstr(buf,"FF7A85") || strstr(buf,"273009C4"))
      {
// vai tocar um mp3
       system("/usr/bin/mpg123 /home/fulano/mp3/acdc/*");
      }

     }
    
   }
  }
  
 if(display)
  XCloseDisplay(display);
  
 if(fd)
  close(fd);

 exit(EXIT_SUCCESS);    
} 

// o que vai fazer precionar a tecla
void type_key(unsigned c) 
{
  XTestFakeKeyEvent(display, XKeysymToKeycode(display, c), True, CurrentTime);
  XTestFakeKeyEvent(display, XKeysymToKeycode(display, c), False, CurrentTime);
  XFlush(display);
}

// leitura serial
int serialread(int fd, char* buf, char until,int max)
{
 char b[1];
 int i=0;
 do { 
  int n=read(fd, b, 1);  
  if(n==-1) 
    return -1;    
  if(!n) 
  {
   usleep(16000); 
   continue;
  }
  buf[i] = b[0]; 
  i++;
 } while(b[0]!=until || max != i);

 buf[i]=0;  

 return 0;
}

// open() inicial
int serialboot(const char* serialport, int baud)
{
 struct termios toptions;
 int fd;

    fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);

    if(fd == -1)  
    {
     DEBUG("serialboot: não foi possivel abrir a porta ");
     return -1;
    }
    
    if(tcgetattr(fd, &toptions) < 0) 
    {
     DEBUG("serialboot: nao foi possivel pegar atributos do terminal");
     return -1;
    }
    speed_t brate = baud; 
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);
   // para default recv com termios.h
    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;
    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // olhe http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;
    
    if(tcsetattr(fd, TCSANOW, &toptions) < 0) 
    {
     DEBUG("serialboot: nao foi possivel adicionar atributos no term erro 1");
     return -1;
    }

 return fd;
}

