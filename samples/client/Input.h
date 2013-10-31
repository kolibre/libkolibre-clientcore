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

#ifndef INPUT_H
#define INPUT_H

#include <ClientCore.h>

#include <pthread.h>
#include <termio.h>
#include <boost/signals2.hpp>

class Input
{
protected:
    Input();
public:
    static Input *Instance();
    ~Input();

    boost::signals2::signal<void(ClientCore::COMMAND)> keyPressed_signal;
    bool set_input_device(std::string dev);

private:
    pthread_mutex_t inputMutex;
    static Input *pinstance;

    pthread_t inputThread;
    bool running;

    friend void *input_thread(void *input);

    int kb_startup_mode;
    struct termios kb_startup_settings;
    int keyboardfd;
    int inputfd;

    int read_mouse_id(int mousefd);
    int write_to_mouse(int mousefd, unsigned char *data, size_t len);
    int fill_buffer(unsigned char buffer[], struct input_event ev[], int count);

    int mousefd;
    int mousetype;
    unsigned char mbuttons;
    int mx, my, mouse_state;
    int mb1, mb2, mb3;
};

#endif
