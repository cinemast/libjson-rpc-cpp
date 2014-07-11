/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    xbmcremote.cpp
 * @date    03.06.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "XbmcRemoteClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <iostream>

using namespace jsonrpc;
using namespace std;

#ifdef __APPLE__
//Taken from: http://stackoverflow.com/questions/312185/kbhit-in-mac
int kbhit_mac() {
    char ch;
    int error;
    static struct termios Otty, Ntty;
    fflush(stdout);
    tcgetattr(0, &Otty);
    Ntty = Otty;
    Ntty.c_iflag  =  0;		/* input mode		*/
    Ntty.c_oflag  =  0;		/* output mode		*/
    Ntty.c_lflag &= ~ICANON;	/* line settings 	*/
#if 1
    /* disable echoing the char as it is typed */
    Ntty.c_lflag &= ~ECHO;	/* disable echo 	*/
#else
    /* enable echoing the char as it is typed */
    Ntty.c_lflag |=  ECHO;	/* enable echo	 	*/
#endif
    Ntty.c_cc[VMIN]  = CMIN;	/* minimum chars to wait for */
    Ntty.c_cc[VTIME] = CTIME;	/* minimum wait time	*/
#if 1
    /*
    * use this to flush the input buffer before blocking for new input
    */
    #define FLAG TCSAFLUSH
#else
    /*
    * use this to return a char from the current input buffer, or block if
    * no input is waiting.
    */
    #define FLAG TCSANOW
#endif
    if ((error = tcsetattr(0, FLAG, &Ntty)) == 0) {
        error  = read(0, &ch, 1 );	      /* get char from stdin */
        error += tcsetattr(0, FLAG, &Otty);   /* restore old settings */
    }
    return (error == 1 ? (int) ch : -1 );
}
#else
//Taken from: http://stackoverflow.com/questions/2984307/c-key-pressed-in-linux-console
int kbhit_linux() {
    static int ch = -1, fd = 0;
    struct termios neu, alt;
    fd = fileno(stdin);
    tcgetattr(fd, &alt);
    neu = alt;
    neu.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(fd, TCSANOW, &neu);
    ch = getchar();
    tcsetattr(fd, TCSANOW, &alt);
    return ch;
}
#endif
int main(int argc, char** argv) {

    if(argc < 2)
    {
        cerr << "Provide XBMC API URL as argument! e.g.: " << argv[0] << " 127.0.0.1:8080/jsonrpc" << endl;
        return -1;
    }
    else
    {
        cout << "XBMC Remote: a,s,d,w for navigation, enter for select, escape or backspace for back button" << endl;
        try {
            HttpClient* httpClient = new HttpClient(argv[1]);
            XbmcRemoteClient stub(httpClient);
            int key = 0;
            for (;;) {
#ifdef __APPLE__
                key = kbhit_mac();
#else
                key = kbhit_linux();
#endif
                switch(key) {
                    case 97: stub.Input_Left();
                        break;
                    case 115: stub.Input_Down();
                        break;
                    case 100: stub.Input_Right();
                        break;
                    case 119: stub.Input_Up();
                        break;
                    case 10:
                    case 13: stub.Input_Select();
                        break;
                    case 127:
                    case 27: stub.Input_Back();
                        break;
                }
                cout << "Pressed: " << key << endl;
            }
            delete httpClient;
        } catch(jsonrpc::JsonRpcException e) {
            cerr << e.what() << endl;
        }
    }
    return 0;
}
