/*
 * Copyright (C) 2012 Kolibre
 *
 * This file is part of kolibre-clientcore.
 *
 * Kolibre-clientcore is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Kolibre-clientcore is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Input.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.sampleclient
log4cxx::LoggerPtr sampleClientInputLog(log4cxx::Logger::getLogger("kolibre.sampleclient.input"));

using namespace std;

#define MOUSE_TRIG       4 // coordinates to move before triggering
#define MOUSE_TYPE_NONE  0 // type none
#define MOUSE_TYPE_PS2   1 // type ps2
#define MOUSE_TYPE_IMPS2 2 // type imps2
void *input_thread(void *input);

Input * Input::pinstance = 0;

Input * Input::Instance()
{
    if (pinstance == 0)
    {
        pinstance = new Input;
    }

    return pinstance;
}

enum SleepTimerOption {
    SLEEP_OFF = 0,
    SLEEP_15,
    SLEEP_30,
    SLEEP_60,
    SLEEP_OPTION_COUNT,
};

/**
 * Constructor
 *
 */
Input::Input()
{
    // Setup mutexes
    pthread_mutex_init(&inputMutex, NULL);

    // disable input device reading
    inputfd = -1;

    //////////////////////////////////
    // Setup STDIN file descriptor
    LOG4CXX_DEBUG(sampleClientInputLog, "Initializing input 'KEYBOARD'");

    struct termios newsettings;

    keyboardfd = STDIN_FILENO;
    int flags;

    // save old attributes
    tcgetattr(keyboardfd, &kb_startup_settings);

    // copy old attributes to the new ones
    newsettings = kb_startup_settings;

    // invert the ICANON and ECHO attribute
    newsettings.c_lflag &= ~(ICANON | ECHO);

    // set new attributes
    if (tcsetattr(keyboardfd, TCSANOW, &newsettings) == -1)
    {
        LOG4CXX_ERROR(sampleClientInputLog, "tcsetattr error: " << strerror(errno));
    }

    // make reads non-blocking
    if ((flags = fcntl(keyboardfd, F_GETFL, 0)) == -1)
    {
        LOG4CXX_ERROR(sampleClientInputLog, "error getting flags: " << strerror(errno));
    }
    else if (fcntl(keyboardfd, F_SETFL, flags | O_RDONLY | O_NONBLOCK) == -1)
    {
        LOG4CXX_ERROR(sampleClientInputLog, "error setting flags: " << strerror(errno));
    }

    //////////////////////////////////
    // Setup mouse file descriptor

    mousefd = open("/dev/input/mice", O_RDWR | O_NONBLOCK, NULL);
    mouse_state = mb1 = mb2 = mb3 = 0;

    // Initialize mouse in imps2 mode
    static unsigned char basic_init[] = { 0xF4, 0xF3, 100 };
    static unsigned char ps2_init[]   = { 0xE6, 0xF4, 0xF3, 100, 0xE8, 3, };
    static unsigned char imps2_init[] = { 0xF3, 200, 0xF3, 100, 0xF3, 80, };

    int id = 0;

    // Set default mousetype
    mousetype = MOUSE_TYPE_NONE;
    if (mousefd != -1)
    {

        /* Do a basic init in case the mouse is confused */
        write_to_mouse(mousefd, basic_init, sizeof(basic_init));

        /* Now try again and make sure we have a PS/2 mouse */
        if (write_to_mouse(mousefd, basic_init, sizeof(basic_init)) == 0)
        {

            /* Try to switch to 3 button mode */
            if (write_to_mouse(mousefd, imps2_init, sizeof(imps2_init)) == 0)
            {

                /* Read the mouse id */
                id = read_mouse_id(mousefd);
                if (id == -1)
                    id = 0; // id error

                /* And do the real initialisation */
                if (write_to_mouse(mousefd, ps2_init, sizeof(ps2_init)) == 0)
                {

                    if (id == 3)
                    { // IMPS2 mouse
                        LOG4CXX_DEBUG(sampleClientInputLog, "Initializing input 'IMPS2 MOUSE'");
                        mousetype = MOUSE_TYPE_IMPS2;
                    }
                    else
                    {
                        if (id == 0)
                        { // PS2 mouse
                            LOG4CXX_DEBUG(sampleClientInputLog, "Initializing input 'PS2 MOUSE'");
                            mousetype = MOUSE_TYPE_PS2;
                        }
                        else
                        {
                            LOG4CXX_DEBUG(sampleClientInputLog, "No mouse found");
                            mousetype = MOUSE_TYPE_NONE;
                        }
                    }
                }
            }

        }
    } /* ps2 was not found!!! */

    LOG4CXX_DEBUG(sampleClientInputLog, "Setting up input thread");
    running = true;
    if (pthread_create(&inputThread, NULL, input_thread, this))
    {
        LOG4CXX_ERROR(sampleClientInputLog, "Failed to initialize input thread");
        usleep(500000);
    }
}

/**
 * Destructor
 *
 */
Input::~Input()
{
    LOG4CXX_DEBUG(sampleClientInputLog, "Destructor");

    pthread_mutex_lock(&inputMutex);
    running = false;
    pthread_mutex_unlock(&inputMutex);

    // wait for thread to exit
    pthread_join(inputThread, NULL);

    // restore old terminal attributes
    if (keyboardfd!= -1)
        tcsetattr(keyboardfd, TCSANOW, &kb_startup_settings);

    //close mousefd;
    if (mousefd != 0)
        close(mousefd);

    //close inputfd
    if (inputfd != -1)
        close(inputfd);
}

//--------------------------------------------------
//--------------------------------------------------
// The playback thread code
void *input_thread(void *input)
{
    Input *i = (Input *) input;

    struct timeval tv;
    fd_set readfds;
    int largest_fd = 0;

    SleepTimerOption sleepTimerOption = SLEEP_OFF;
    ClientCore::COMMAND key_pressed = (ClientCore::COMMAND) -1;

    char char_pressed = 0;

    int bytes_read;
    int bytes_to_read = 0;
    unsigned char buffer[128];
    struct input_event ev[128];
    bool isRunning = true;

    LOG4CXX_INFO(sampleClientInputLog, "Starting input thread");

    if (i->mousetype == MOUSE_TYPE_PS2)
        bytes_to_read = 3;
    else if (i->mousetype == MOUSE_TYPE_IMPS2)
        bytes_to_read = 4;

    // Loop
    do
    {
        // Reset checker parameters
        FD_ZERO(&readfds);

        if (i->inputfd != -1)
        {
            FD_SET(i->inputfd, &readfds);
            if (largest_fd < i->inputfd)
                largest_fd = i->inputfd;
        }

        if (i->keyboardfd != -1)
        {
            FD_SET(i->keyboardfd, &readfds);
            if (largest_fd < i->keyboardfd)
                largest_fd = i->keyboardfd;
        }

        if (i->mousefd != -1)
        {
            FD_SET(i->mousefd, &readfds);
            if (largest_fd < i->mousefd)
                largest_fd = i->mousefd;
        }

        // Reset timeout
        tv.tv_sec = 0;
        tv.tv_usec = 50000;

        // Check for a key on the fd's we have chosen
        select(largest_fd + 1, &readfds, NULL, NULL, &tv);

        // Check input from input device
        if (i->inputfd != -1 && FD_ISSET(i->inputfd, &readfds))
        {

            // We have data waiting
            memset(&buffer, 0, 128);
            bytes_read = read(i->inputfd, &ev, sizeof(ev));
            bytes_read = i->fill_buffer(buffer, ev, bytes_read);


            for(int c = 0; c < bytes_read; c++){
                int buffnum = buffer[c];
                LOG4CXX_TRACE(sampleClientInputLog, "buffer["<< c << "] = " << buffnum << " ('" << buffer[c] << "')");

                switch (buffer[c])
                {
                case KEY_Q:
                    key_pressed = ClientCore::EXIT;
                    break;
                case KEY_BACKSPACE:
                case KEY_KP7:
                case KEY_HOME:
                    key_pressed = ClientCore::BACK;
                    break;
                case KEY_SPACE:
                case KEY_KP5:
                    key_pressed = ClientCore::PAUSE;
                    break;
                case KEY_B:
                case KEY_END:
                case KEY_KP1:
                    key_pressed = ClientCore::BOOKMARK;
                    break;
                case KEY_F4:
                case KEY_INSERT:
                case KEY_KP3:
                    key_pressed = ClientCore::CONTEXTMENU;
                    break;
                case KEY_ESC:
                case KEY_KP9:
                case KEY_PAGEUP:
                    key_pressed = ClientCore::HOME;
                    break;
                case KEY_S:
                    // cycle between sleep timer options when key 's' is pressed
                    sleepTimerOption = (SleepTimerOption)((sleepTimerOption + 1) % SLEEP_OPTION_COUNT);
                    key_pressed = (ClientCore::COMMAND)(ClientCore::SLEEP_OFF + sleepTimerOption);
                    break;
                case KEY_UP:
                case KEY_KP8:
                    key_pressed = ClientCore::UP;
                    break;
                case KEY_DOWN:
                case KEY_KP2:
                    key_pressed = ClientCore::DOWN;
                    break;
                case KEY_RIGHT:
                case KEY_KP6:
                    key_pressed = ClientCore::RIGHT;
                    break;
                case KEY_LEFT:
                case KEY_KP4:
                    key_pressed = ClientCore::LEFT;
                    break;
                case KEY_KPPLUS:
                case KEY_EQUAL:
                    key_pressed = ClientCore::SPEEDUP;
                    break;
                case KEY_MINUS:
                case KEY_KPMINUS:
                    key_pressed = ClientCore::SPEEDDOWN;
                    break;
                default:
                    LOG4CXX_INFO(sampleClientInputLog, "Unhandled keypress on input buffer[" << c << "]  = " << buffnum << " ('" << buffer[c] << "')");
                    //char_pressed = buffer[0];
                    break;
                }
            }
        }

        // Check the input from the keyboard
        if (i->keyboardfd != -1 && FD_ISSET(i->keyboardfd, &readfds))
        {
            // We have data waiting
            memset(&buffer, 0, 128);
            bytes_read = read(i->keyboardfd, &buffer, sizeof(buffer));

            if (bytes_read)
            {
                // Check what bytes we read and process them
                for(int c = 0; c < bytes_read; c++){
                    int buffnum = buffer[c];
                    LOG4CXX_TRACE(sampleClientInputLog, "buffer["<< c << "] = " << buffnum << " '" << buffer[c] << "' ");
                }

                //buffer[bytes_read] = '\0';
                switch (buffer[0])
                {
                case 113:
                    key_pressed = ClientCore::EXIT;
                    break;
                case 97:
                case 10:
                    key_pressed = ClientCore::BACK;
                    break;
                case 32:
                    key_pressed = ClientCore::PAUSE;
                    break;
                case 98:
                    key_pressed = ClientCore::BOOKMARK;
                    break;
                case 109:
                    key_pressed = ClientCore::CONTEXTMENU;
                    break;
                case 66:
                    if (i->mousetype == MOUSE_TYPE_IMPS2)
                        key_pressed = ClientCore::CONTEXTMENU;
                    break;
                case 127:
                    key_pressed = ClientCore::BACK;
                    break;
                case 100:
                    key_pressed = ClientCore::NARRATOR_CONTROL_STOP; // d
                    break;
                case 104:
                    key_pressed = ClientCore::HOME;
                    break;
                case 115:
                    // cycle between sleep timer options when key 's' is pressed
                    sleepTimerOption = (SleepTimerOption)((sleepTimerOption + 1) % SLEEP_OPTION_COUNT);
                    key_pressed = (ClientCore::COMMAND)(ClientCore::SLEEP_OFF + sleepTimerOption);
                    break;
                case 27:
                    switch (buffer[1])
                    {
                    case 100:
                        key_pressed = ClientCore::NARRATOR_CONTROL_CONTINUE; // Alt + d
                        break;
                    case 91:
                        switch (buffer[2])
                        {
                        case 65:
                            key_pressed = ClientCore::UP;
                            break;
                        case 66:
                            key_pressed = ClientCore::DOWN;
                            break;
                        case 67:
                            key_pressed = ClientCore::RIGHT;
                            break;
                        case 68:
                            key_pressed = ClientCore::LEFT;
                            break;
                        case 52:
                            if (buffer[3] == 126)
                                key_pressed = ClientCore::PAUSE;
                            break;
                        case 53:
                            if (buffer[3] == 126)
                                key_pressed = ClientCore::SPEEDUP;
                            break;
                        case 54:
                            if (buffer[3] == 126)
                                key_pressed = ClientCore::SPEEDDOWN;
                            break;
                        case 91:
                            if (buffer[3] == 69 && i->mousetype == MOUSE_TYPE_IMPS2)
                                key_pressed = ClientCore::PAUSE;
                            break;
                        default:
                            LOG4CXX_WARN(sampleClientInputLog, "UNKNOWN KEY buffer[2] == " << buffer[2]);
                            break;
                        }
                        break;

                        default:
                            if (i->mousetype == MOUSE_TYPE_IMPS2)
                                key_pressed = ClientCore::PAUSE;
                            else
                                LOG4CXX_WARN(sampleClientInputLog, "UNKNOWN KEY buffer[1] == " << buffer[1]);
                            break;
                    }
                    break;
                    default:
                        LOG4CXX_WARN(sampleClientInputLog, "UNKNOWN KEY buffer[0] == " << buffer[0]);
                        break;
                }
            }

        }

        char data[4] = { 0, 0, 0, 0 };

        // Check the input from the mouse
        if (i->mousefd != -1 && FD_ISSET(i->mousefd, &readfds))
        {

            bytes_read = read(i->mousefd, data, bytes_to_read);
            if (bytes_read == bytes_to_read)
            {
                char dx, dy;
                // check for a button press
                char mbuttons = ((data[0] & 1) << 2) // left button
                                | ((data[0] & 6) >> 1); // middle and right button

                // check for mouse movement
                dx = (data[0] & 0x10) ? data[1] - 256 : data[1];
                dy = (data[0] & 0x20) ? -(data[2] - 256) : -data[2];

                if (i->mouse_state > 1)
                {
                    i->mx += dx;
                    i->my += dy;
                }
                else if (i->mouse_state == 0)
                {
                    //if(!(mb1 || mb2)) beep();
                    i->mx = dx;
                    i->my = dy;
                }

                if (!(i->mb1 || i->mb2 || i->mb3))
                {
                    //cout << "Mouse started moving" , INPUT_PREFIX);
                    i->mouse_state = 5;
                }
                else
                    i->mouse_state = 2;

                //printf("mbuttons %s\n", to_bin(mbuttons));
                //printf("mx: %d\t, my: %d\t mbuttons: %d mouse_state %d data %s %x %x %x %x\n", i->mx, i->my, i->mbuttons, i->mouse_state,
                //to_bin(data[0]),
                //     data[0], data[1], data[2], data[3]);

                if (mbuttons == 1)
                    i->mb2 = 1;
                else if (mbuttons == 2)
                    i->mb3 = 1;
                else if (mbuttons == 4)
                    i->mb1 = 1;
            } //else { printf("bytes_read: %d\n", bytes_read); }
        }

        // Check the mouse_state, if we've stopped moving make it a key
        if (i->mousefd != -1 && i->mouse_state > 1)
        {
            if (i->mouse_state > 1)
                i->mouse_state--;
            //printw("mouse_state: %d\n", mouse_state);
            if (i->mouse_state == 1)
            {
                //printf("Stopped moving at %d, %d with buttons %d and data %x %x %x %x\n", i->mx, i->my, i->mbuttons,
                //   data[0], data[1], data[2], data[3]);
                if (!(i->mb1 || i->mb2 || i->mb3))
                {
                    //cout << "Mouse stopped moving" , INPUT_PREFIX);
                    if (abs(i->mx) > abs(i->my))
                    {
                        if (i->mx > MOUSE_TRIG)
                            key_pressed = ClientCore::RIGHT; //RIGHT
                        else if (i->mx < -MOUSE_TRIG)
                            key_pressed = ClientCore::LEFT; //LEFT
                    }
                    else
                    {
                        if (i->my > MOUSE_TRIG)
                            key_pressed = ClientCore::DOWN; //DOWN
                        else if (i->my < -MOUSE_TRIG)
                            key_pressed = ClientCore::UP; //UP
                    }
                }
                if (!i->mbuttons && i->mb1)
                {
                    key_pressed = ClientCore::BACK;
                    i->mb1 = 0;
                } // pause
                if (!i->mbuttons && i->mb2)
                {
                    key_pressed = ClientCore::PAUSE;
                    i->mb2 = 0;
                } // repeat
                if (!i->mbuttons && i->mb3)
                {
                    key_pressed = ClientCore::CONTEXTMENU;
                    i->mb3 = 0;
                } // repeat
                i->mouse_state = 0;
            }
        }

        if (key_pressed != (ClientCore::COMMAND) -1)
        {
            // Add the key to the queue
            i->keyPressed_signal(key_pressed);
            key_pressed = (ClientCore::COMMAND) -1;
        }

        if (char_pressed != 0){
            //Do nothing
            char_pressed = 0;
        }

        pthread_mutex_lock(&i->inputMutex);
        isRunning = i->running;
        pthread_mutex_unlock(&i->inputMutex);

    } while (isRunning);

    LOG4CXX_INFO(sampleClientInputLog, "Input thread exiting");

    //pthread_mutex_lock(&n->inputMutex);
    //queueitems = n->playList.size();
    //pthread_mutex_unlock(&n->inputMutex);

    // Check for keys here
    return NULL;
}

int Input::read_mouse_id(int mousefd)
{
    unsigned char c = 0xF2;
    unsigned char id = 0;

    write(mousefd, &c, 1);
    read(mousefd, &c, 1);
    if (c != 0xFA)
    {
        return (-1);
    }
    read(mousefd, &id, 1);

    return (id);
}

int Input::write_to_mouse(int mousefd, unsigned char *data, size_t len)
{
    size_t i;
    int error = 0;
    for (i = 0; i < len; i++)
    {
        unsigned char c;
        write(mousefd, &data[i], 1);
        read(mousefd, &c, 1);
        if (c != 0xFA)
            error++; // not an ack
    }

    /* flush any left-over input */
    usleep(30000);
    tcflush(mousefd, TCIFLUSH);
    return (error);
}

int Input::fill_buffer(unsigned char buffer[], struct input_event ev[], int count)
{
    int keys = 0;
    for(int it=0; it < count / sizeof(struct input_event); it ++){
        if(ev[it].type == 1 && ev[it].value == 1){
            buffer[keys] = ev[it].code;
            keys++;
        }
    }
    return keys;
}
/*
 char *to_bin(unsigned char c) {
 static char str[9];

 sprintf(str, "%c%c%c%c%c%c%c%c\0",
 c & 1 ? '1' : '0',
 c & 2 ? '1' : '0',
 c & 4 ? '1' : '0',
 c & 8 ? '1' : '0',
 c & 16 ? '1' : '0',
 c & 32 ? '1' : '0',
 c & 64 ? '1' : '0',
 c & 128 ? '1' : '0');
 return str;
 }
 */

bool Input::set_input_device(std::string dev)
{
    //////////////////////////////////
    // Setup input file descriptor, assuming kbd is event0

    LOG4CXX_DEBUG(sampleClientInputLog, "Initializing input from " << dev);

    inputfd = open(dev.c_str(), O_RDONLY | O_NONBLOCK, NULL);

    if(inputfd != -1){
        // Check id
        char unique[256];
        if(ioctl(inputfd, EVIOCGUNIQ(sizeof(unique)), unique) < 0) {
            LOG4CXX_WARN(sampleClientInputLog, "failed to get event ioctl");
        }
        else
            LOG4CXX_INFO(sampleClientInputLog, "Keyboard identity is: " << unique);

        //Disable input from stdin
        tcsetattr(keyboardfd, TCSANOW, &kb_startup_settings);
        keyboardfd = -1;

        return true;
    }
    else
        LOG4CXX_ERROR(sampleClientInputLog, "failed to open device: " << dev);
    return false;
}
