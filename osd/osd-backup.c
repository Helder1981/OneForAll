//-------------------------------------------------------------------------
//
//
// This file originates from Vascofazza's Retropie open OSD project.
// Author: Federico Scozzafava
//
// THIS HEADER MUST REMAIN WITH THIS FILE AT ALL TIMES
//
// This firmware is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This firmware is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this repo. If not, see <http://www.gnu.org/licenses/>.
//
//
//-------------------------------------------------------------------------

#define _GNU_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <strings.h>
#include "font.h"
#include "imageGraphics.h"
#include "imageLayer.h"
#include "loadpng.h"
#include <math.h>

#include "bcm_host.h"

//-------------------------------------------------------------------------

#define NDEBUG

#define BATTERY_IMAGE "/home/pi/Retropie-open-OSD/resources/battery.png"
#define CHARGE_IMAGE "/home/pi/Retropie-open-OSD/resources/plug.png"
#define INFO_IMAGE "/home/pi/Retropie-open-OSD/resources/main16.png"
//#define INFO_IMAGE "/home/pi/Retropie-open-OSD/resources/battery.png"
#define BATTERY_TH 20
#define AUDIO_IMAGES (const char*[5]){"/home/pi/Retropie-open-OSD/resources/AUD0.png","/home/pi/Retropie-open-OSD/resources/AUD25.png","/home/pi/Retropie-open-OSD/resources/AUD50.png","/home/pi/Retropie-open-OSD/resources/AUD75.png","/home/pi/Retropie-open-OSD/resources/AUD100.png"}
#define WIFI_IMAGES (const char*[5]){"/home/pi/Retropie-open-OSD/resources/wifi_warning.png", "/home/pi/Retropie-open-OSD/resources/wifi_error.png", "/home/pi/Retropie-open-OSD/resources/wifi_1.png", "/home/pi/Retropie-open-OSD/resources/wifi_2.png", "/home/pi/Retropie-open-OSD/resources/wifi_3.png"}
#define BRIGHTNESS_MAX 7

volatile bool run = true;

//-------------------------------------------------------------------------

static RGBA8_T clearColor = {0,0,0,0};
static RGBA8_T backgroundColour = { 0, 0, 0, 100 };
static RGBA8_T textColour = { 255, 255, 255, 255 };
static RGBA8_T greenColour = { 0, 255, 0, 200 };
static RGBA8_T redColour = { 255, 0, 0, 200 };
static int battery = 0, infos = 0, hud = 1, brightness = 0, charge = 0, audio = 0, wifi = 0, voltage = 0;
static float temp = 0.f;

void updateInfo(IMAGE_LAYER_T*);
void getInput();
void clearLayer(IMAGE_LAYER_T*);
void updateBattery(float, IMAGE_LAYER_T*);

static void
signalHandler(int signalNumber)
{
    switch (signalNumber)
    {
        case SIGUSR1:
            getInput();
            break;
        case SIGINT:
        case SIGTERM:
            
            run = false;
            break;
    };
}

void updateBattery(float batval, IMAGE_LAYER_T* batteryLayer)
{
    IMAGE_T *image = &(batteryLayer->image);
    clearImageRGB(image,&clearColor);
    //to config params
    RGBA8_T *col = battery < BATTERY_TH ? &redColour : &greenColour;
    imageBoxFilledRGB(image, 2,3,2+(int)round((batteryLayer->image.width-7)*batval),batteryLayer->image.height-3, col);
    changeSourceAndUpdateImageLayer(batteryLayer);
}

void getInput()
{
    char buffer[100];
    char *bp = buffer;
    int c;
    while(run && '\n' != (c=fgetc(stdin)) && (bp-buffer) < sizeof(buffer))
        *bp++=c;
    *bp = 0;//terminator
#ifndef NDEBUG
    printf("buffer: %s\n", buffer);
#endif
    char *saveptr;
    char *word;
    char *str = buffer;
    while((word = strtok_r(str," ",&saveptr)))
    {
#ifndef NDEBUG
        printf("word: %s\n", word);
#endif
        str = NULL;
        if(word[0] == 'b')
        {
            //Battery
            battery = atoi(word+1);
        }
        else if(word[0] == 'w')
        {
            //wifi
            wifi= atoi(word+1);
        }
        else if(word[0] == 'a')
               {
                  //audio
                  audio= atoi(word+1);
               }
        else if(word[0] == 'l')
        {
            //Brightness
            brightness= atoi(word+1);
        }
        else if(word[0] == 't')
        {
            //temperature
            temp= atof(word+1);
        }
        else if(word[0] == 'v')
        {
            //voltage
            voltage= atoi(word+1);
        }
        else if(!strcmp(word, "on"))
        {
            infos = 1;
        }
        else if(!strcmp(word, "off"))
        {
            infos = 0;
        }
        else if(!strcmp(word, "allon"))
        {
            hud = 1;
        }
        else if(!strcmp(word, "alloff"))
        {
            hud = 0;
        }
        else if(!strcmp(word, "charge"))
        {
            charge = 1;
        }
        else if(!strcmp(word, "ncharge"))
        {
            charge = 0;
        }
        else if(!strcmp(word,"quit"))
            run = false;
    }
#ifndef NDEBUG
    printf("out\n");
#endif
}

void clearLayer(IMAGE_LAYER_T *layer)
{
    IMAGE_T *image = &(layer->image);
    clearImageRGB(image, &clearColor);
    changeSourceAndUpdateImageLayer(layer);
}

//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    uint32_t displayNumber = 0;

    //-------------------------------------------------------------------

    int opt;

    while ((opt = getopt(argc, argv, "d:")) != -1)
    {
        switch (opt)
        {
        case 'd':

            displayNumber = atoi(optarg);
            break;

        default:

            fprintf(stderr, "Usage: %s [-d <number>]\n", basename(argv[0]));
            fprintf(stderr, "    -d - Raspberry Pi display number\n");
            exit(EXIT_FAILURE);
            break;
        }
    }
    
    if (signal(SIGINT, signalHandler) == SIG_ERR)
    {
        perror("installing SIGINT signal handler");
        exit(EXIT_FAILURE);
    }
    
    //---------------------------------------------------------------------
    
    if (signal(SIGTERM, signalHandler) == SIG_ERR)
    {
        perror("installing SIGTERM signal handler");
        exit(EXIT_FAILURE);
    }
    
    //---------------------------------------------------------------------
    
    if (signal(SIGUSR1, signalHandler) == SIG_ERR)
    {
        perror("installing SIGTERM signal handler");
        exit(EXIT_FAILURE);
    }

    //-------------------------------------------------------------------

    VC_IMAGE_TYPE_T type = VC_IMAGE_RGBA32;

    //---------------------------------------------------------------------

    bcm_host_init();

    //---------------------------------------------------------------------

    DISPMANX_DISPLAY_HANDLE_T display
        = vc_dispmanx_display_open(displayNumber);
    assert(display != 0);

    //---------------------------------------------------------------------

    DISPMANX_MODEINFO_T info;
    int result = vc_dispmanx_display_get_info(display, &info);
    assert(result == 0);

    //---------------------------------------------------------------------
    
    static int layer = 30000;
    
    IMAGE_LAYER_T infoLayer;

    if (loadPng(&(infoLayer.image), INFO_IMAGE) == false)
        {
            fprintf(stderr, "unable to load %s\n", INFO_IMAGE);
            exit(EXIT_FAILURE);
        }


    createResourceImageLayer(&infoLayer, layer + 3);

    
    IMAGE_LAYER_T bimageLayer;
    if (loadPng(&(bimageLayer.image), BATTERY_IMAGE) == false)
    {
        fprintf(stderr, "unable to load %s\n", argv[optind]);
    }
    createResourceImageLayer(&bimageLayer, layer+2);
    
    IMAGE_LAYER_T batteryLayer;
    initImageLayer(&batteryLayer,
                   bimageLayer.image.width,
                   bimageLayer.image.height,
                   VC_IMAGE_RGBA16);
    createResourceImageLayer(&batteryLayer, layer+1);
    
    IMAGE_LAYER_T cimageLayer;
    initImageLayer(&cimageLayer,
                   bimageLayer.image.width,
                   bimageLayer.image.height,
                   type);
    createResourceImageLayer(&cimageLayer, layer+2);
    
    IMAGE_LAYER_T wimageLayer;
    initImageLayer(&wimageLayer,
                   bimageLayer.image.height,
                   bimageLayer.image.height,
                   type);
    createResourceImageLayer(&wimageLayer, layer+2);

     IMAGE_LAYER_T aimageLayer;
        initImageLayer(&aimageLayer,
                       20,
                       11,
                       type);
     createResourceImageLayer(&aimageLayer, layer+2);
    
    int xOffset = info.width-bimageLayer.image.width-1;
    int yOffset = 1;
    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);

    addElementImageLayerOffset(&infoLayer,
                               (info.width - infoLayer.image.width) / 2,
                               (info.height - infoLayer.image.height) / 2,
                               display,
                               update);

    addElementImageLayerOffset(&wimageLayer,
                               xOffset-wimageLayer.image.width-2,
                               yOffset,
                               display,
                               update);

    addElementImageLayerOffset(&aimageLayer,
                               (xOffset-wimageLayer.image.width)-aimageLayer.image.width-8,
                               yOffset,
                               display,
                               update);
    
    addElementImageLayerOffset(&batteryLayer,
                               xOffset,
                               yOffset,
                               display,
                               update);
    addElementImageLayerOffset(&bimageLayer,
                               xOffset,
                               yOffset,
                               display,
                               update);
    addElementImageLayerOffset(&cimageLayer,
                               xOffset,
                               yOffset,
                               display,
                               update);
    
    
    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);
    
    while (run)
    {
        float batval = battery/100.f;
        {
            updateBattery(batval, &batteryLayer);
        }
        if(charge > 0 && hud)
        {
            //TODO preload for efficiency
            if (loadPng(&(cimageLayer.image), CHARGE_IMAGE) == false)
            {
                fprintf(stderr, "unable to charge load %s\n", argv[optind]);
            }
            changeSourceAndUpdateImageLayer(&cimageLayer);
            charge = -1;
        }
        else if(!charge && hud)
        {
            clearLayer(&cimageLayer);
            charge = -1;
        }
        if(wifi > 0 && hud)
        {
            //TODO preload for efficienty
            if (loadPng(&(wimageLayer.image), WIFI_IMAGES[wifi-1]) == false)
            {
                fprintf(stderr, "unable to wifi load %s\n", argv[optind]);
            }
            changeSourceAndUpdateImageLayer(&wimageLayer);
            wifi = -1;
        }
        else if (!wifi)
        {
            clearLayer(&wimageLayer);
        }
         if(audio > 0)
                {
                    //TODO preload for efficienty
                    if (loadPng(&(aimageLayer.image), AUDIO_IMAGES[audio-1]) == false)
                    {
                        fprintf(stderr, "unable to audio load %s\n", argv[optind]);
                    }
                    changeSourceAndUpdateImageLayer(&aimageLayer);
                    audio = -1;
                }
                else if (!audio)
                {
                    clearLayer(&aimageLayer);
                }
        if (!hud) {
            clearLayer(&batteryLayer);
            clearLayer(&wimageLayer);
            clearLayer(&aimageLayer);
            clearLayer(&bimageLayer);
            clearLayer(&cimageLayer);
        }
        if(infos > 0)
        {
            updateInfo(&infoLayer);
        }
        else if(!infos)
        {
            infos = -1;
            clearLayer(&infoLayer);
        }
        else
        {
            pause(); //stop thread
        }
        usleep(1000000); //sleep 1sec
    }
    //---------------------------------------------------------------------

    destroyImageLayer(&infoLayer);
    destroyImageLayer(&batteryLayer);
    destroyImageLayer(&wimageLayer);
    destroyImageLayer(&aimageLayer);
    destroyImageLayer(&bimageLayer);
    destroyImageLayer(&cimageLayer);
    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    return 0;
}

void updateInfo(IMAGE_LAYER_T *infoLayer)
{
    IMAGE_T *image = &(infoLayer->image);
//    clearImageRGB(image, &backgroundColour);
    loadPng(&(infoLayer->image), INFO_IMAGE);
//    int x = 1, y = 1;
//    drawStringRGB(x, y, buffer, &textColour, image);
    changeSourceAndUpdateImageLayer(infoLayer);
}

