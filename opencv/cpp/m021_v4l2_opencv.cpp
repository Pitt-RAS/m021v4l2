/*
  
   m021_v4l2_opencv.cpp - C++ source for Leopard Imaging M021 camera capture using OpenCV on Linux.
  
   Copyright (C) 2016 Simon D. Levy

   This file is part of M021_V4L2.

   M021_V4L2 is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   BreezySTM32 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with M021_V4L2.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "m021_v4l2.h"
#include "m021_v4l2_opencv.hpp"

#include <stdio.h>

typedef struct {

    int rows;
    int cols;
    uint8_t * bytes;
    pthread_mutex_t lock;
    unsigned long long count;
    int bcorrect;
    int gcorrect;
    int rcorrect;

} data_t;

static void * loop(void * arg)
{

    data_t * data = (data_t *)arg;
    pthread_mutex_t lock = data->lock;

    m021_t cap;
    m021_init(0, &cap, data->cols, data->rows);

    data->count = 0;

    while (true) {

        pthread_mutex_lock(&lock);

        m021_grab_bgr(&cap, data->bytes, data->bcorrect, data->gcorrect, data->rcorrect);

        pthread_mutex_unlock(&lock);

        data->count++;
    }

    m021_free(&cap);

    return (void *)0;
}

M021_Capture::M021_Capture(Mat & mat, int width, int height, int bcorrect, int gcorrect, int rcorrect)
{
    this->bcorrect = bcorrect;
    this->gcorrect = gcorrect;
    this->rcorrect = rcorrect;

    mat = Mat(height, width, CV_8UC3);

    pthread_mutex_t lock;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        exit(1);
    }

    data_t * data = new data_t;
    data->rows = mat.rows;
    data->cols = mat.cols;
    data->bytes = mat.data;
    data->lock = lock;
    data->bcorrect = bcorrect;
    data->gcorrect = gcorrect;
    data->rcorrect = rcorrect;

    this->data = data;

    if (pthread_create(&this->video_thread, NULL, loop, data)) {
        fprintf(stderr, "Failed to create thread\n");
        exit(1);
    }
}
        
M021_Capture::~M021_Capture(void)
{
    free(this->data);
}

unsigned long long M021_Capture::getCount(void) 
{
    data_t * data = (data_t *)this->data;

    return data->count;
}
