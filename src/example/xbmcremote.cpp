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

#include <iostream>

using namespace jsonrpc;
using namespace std;

//Taken from: http://stackoverflow.com/questions/2984307/c-key-pressed-in-linux-console
int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}

int main(int argc, char** argv) {

    if(argc < 2)
    {
        cerr << "Provide XBMC API URL as argument! e.g.: " << argv[0] << " 127.0.0.1:8080/jsonrpc" << endl;
        return -1;
    }
    else
    {
        try {
            XbmcRemoteClient stub(new HttpClient(argv[1]));
            int key = 0;
            for (;;) {
                key = getkey();
                /* terminate loop on ESC (0x1B) or Ctrl-D (0x04) on STDIN */
                if (key == 0x1B || key == 0x04) {
                    break;
                }
                else {
                    cout << "presseD: " << key << endl;
                }
                sleep(0.2);
            }
        } catch(jsonrpc::JsonRpcException e) {
            cerr << e.what() << endl;
        }
    }



    return 0;
}
