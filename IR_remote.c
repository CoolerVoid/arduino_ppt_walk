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


Vocẽ vai precisar da biblioteca  libX11-dev e libxtst-dev
$ sudo apt-get install libX11-dev libxtst-dev
$ make

Obrigado ao I4K "Tiago Natel" por dar a dica da lib Xtst e X11,
Obrigado a Galera que sempre me ajuda ai,
m0nad,sigsegv,slyfunky,otacon_x32,eremitah,clandestine,f117,BSDaemon,colt7r...

 */
#include <stdio.h>    
#include <stdlib.h> 
#include <string.h>   
#include <unistd.h> 
#include <alloca.h>
#include <termios.h>  
#include <fcntl.h> 
#include <time.h>
#include <sys/resource.h>
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

int foo,revert;
Display *display;
Window winFocus;

void no_write_coredump ( void );
void type_key ( unsigned c ); 
int serialboot( const char* serialport, int baud );
int serialread( int fd, char* buf, char until, int max );

void banner() 
{
	fprintf(stdout,"\nFollow patern: ./This_Code <SerialPort>\n"
		"Presenter controller with TV remote   :-] v0.1 !!!\n"
		"Coded By Cooler_\n"
		"\n");
}


int main(int argc, char *argv[]) 
{

 int baudrate = BAUDRATE, fd = 0; 
 char buf[17];
 char *serialport = alloca( sizeof(char)*17 );

 	memset(serialport,0,17);
	no_write_coredump();

 	if( argc < 2 ) 
 	{
		banner();
		exit(EXIT_SUCCESS);
	}

   
	if( strnlen(argv[1],18) <= 16)
  	{
// start get keys of keyboard
   		if( (display=XOpenDisplay(NULL)) == NULL) 
   		{
    			DEBUG("%s: can't open %s\n", argv[0], XDisplayName(NULL));
    			exit(1);
   		}

   		if( XTestQueryExtension(display, &foo, &foo, &foo, &foo) == False ) 
   		{
    			DEBUG("XTEST extension missing\n");
    			exit(1);
   		}

   		XGetInputFocus(display, &winFocus, &revert);
 
   		strncpy(serialport,argv[1],16);
   		fd=serialboot(serialport, baudrate);
   		fprintf(stdout,"Serial:%s\n",serialport);

   		if( fd <= 0 )
   		{
    			DEBUG("Erro\nProcure saber se o dispositivo esta conectado!!\n");
    			exit(EXIT_SUCCESS);
   		}

   
   		while(1) 
   		{
    			memset(buf,0,16);
// read the input of serial device put in "buf"
    			serialread(fd, buf, '\n',16);
    			usleep(620000);
    			puts(buf);

     			if( strnlen(buf,9) > 3 )
     			{
      				fprintf(stdout,"\n string: %s \n tamanho: %d",buf,strlen(buf));

// button number 1 , if buf == E13DDA28 ...
      				if( strstr(buf,"FF30CF") || strstr(buf,"E13DDA28") )
      				{
// if hold button 1, hold the key "k" of keyboard to get next screen of presenter
       					type_key('k');
      				}

// button number 2
      				if( strstr(buf,"FF18E7") || strstr(buf,"AD586662") )
      				{
// if hold button 2, hold the key "l" of keyboard to back screen of presenter
       					type_key('l');
      				}

// button number 3
      				if( strstr(buf,"FF7A85") || strstr(buf,"273009C4") )
      				{
//if hold button 3 plays mp3 music
       					execl ("/usr/bin/mpg123"," ","/home/fulano/mp3/acdc/*",NULL);
      				}

     			}	
    
   		}
  	} else {
		DEBUG("\nErro\nentrada muito grande...\n");
		exit(1);
	}
  
	if( display )
		XCloseDisplay(display);
  
	if( fd )
		close(fd);

 exit(0);    

} 




void no_write_coredump (void)
{
	struct rlimit rlim;

	rlim.rlim_cur = 0;
	rlim.rlim_max = 0;
	setrlimit(RLIMIT_CORE, &rlim);
}


// o que vai fazer precionar a tecla
void type_key( unsigned c ) 
{
	XTestFakeKeyEvent(display, XKeysymToKeycode(display, c), True, CurrentTime);
	XTestFakeKeyEvent(display, XKeysymToKeycode(display, c), False, CurrentTime);
	XFlush(display);
}

// leitura serial
int serialread(int fd, char* buf, char until,int max)
{
 char b[1];
 int i = 0;

	do { 
		int n = read( fd, b, 1 );  

		if( n==-1 ) 
			return -1;    
		if( !n ) 
		{
			usleep(16000); 
			continue;
		}

		buf[i] = b[0]; 
		i++;

	} while ( b[0]!=until && max != i );

	buf[i] = 0;  

 return 0;
}

// open() inicial
int serialboot(const char* serialport, int baud)
{
 struct termios toptions;
 int fd;

	fd = open( serialport, O_RDWR | O_NOCTTY | O_NDELAY );

	if( fd == -1 )  
	{
		DEBUG("serialboot: não foi possivel abrir a porta ");
		close(fd);
		exit(1);
	}
    
	if( tcgetattr(fd, &toptions) < 0) 
	{
		DEBUG("serialboot: nao foi possivel pegar atributos do terminal");
		close(fd);
		exit(1);
	}

	speed_t brate = baud; 
	cfsetispeed(&toptions, brate);
	cfsetospeed(&toptions, brate);
   // para default recv com termios.h
	toptions.c_cflag &= ~PARENB;
	toptions.c_cflag &= ~CSTOPB;
	toptions.c_cflag &= ~CSIZE;
	toptions.c_cflag |= CS8;
	toptions.c_cflag &= ~CRTSCTS;
	toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
	toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
	toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
	toptions.c_oflag &= ~OPOST; 

    // olhe http://unixwiz.net/techtips/termios-vmin-vtime.html
	toptions.c_cc[VMIN]  = 0;
	toptions.c_cc[VTIME] = 20;
    
	if( tcsetattr(fd, TCSANOW, &toptions) < 0) 
	{
		DEBUG("serialboot: nao foi possivel adicionar atributos no term erro 1");
		return -1;
	}

 return fd;
}




