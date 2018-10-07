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
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <strings.h>
#include "font.h"
#include "imageGraphics.h"
#include "imageLayer.h"
#include "loadpng.h"
#include <unistd.h>
#include <math.h>

#include "bcm_host.h"

//-------------------------------------------------------------------------

#define NDEBUG
#define BATTERY_IMAGE "./resources/battery.png"
#define CHARGE_IMAGE "./resources/plug.png"
#define INFO_IMAGE "./resources/main.png"
#define JOYSTICK_IMAGE "./resources/joystick.png"
#define BLUETOOTH_IMAGE "./resources/bluetooth.png"
#define BATTERY_TH 20
#define AUDIO_IMAGES (const char*[5]){"./resources/AUD0.png","./resources/AUD25.png","./resources/AUD50.png","./resources/AUD75.png","./resources/AUD100.png"}
#define WIFI_IMAGES (const char*[5]){"./resources/wifi_warning.png", "./resources/wifi_error.png", "./resources/wifi_1.png", "./resources/wifi_2.png", "./resources/wifi_3.png"}
#define BRIGHTNESS_MAX 7

volatile bool run = true;

//-------------------------------------------------------------------------

static RGBA8_T clearColor = {0,0,0,0};
static RGBA8_T backgroundColour = { 0, 0, 0, 100 };
static RGBA8_T textColour = { 255, 255, 255, 255 };
static RGBA8_T greenColour = { 0, 255, 0, 200 };
static RGBA8_T redColour = { 255, 0, 0, 200 };
static int battery = 0, infos = 0, hud = 1, brightness = 0, charge = 0, audio = 0, wifi = 0, wifi_loaded = 0, voltage = 0, vol_image = 0, infos_loaded = 0, joystick = 0, bluetooth = 0;
static float temp = 0.f;

void updateInfo(IMAGE_LAYER_T*);
void updateInfoText(IMAGE_LAYER_T*);
void getInput();
void clearLayer(IMAGE_LAYER_T*);
void updateBattery(float, IMAGE_LAYER_T*);
char *getcwd(char *buf, size_t size);

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

int getImageIconFromVolume(int vol) {
    if (vol == 0) {
        return 1;
        }
    if (vol <= 25) {
        return 2;
        }
    if (vol <= 50) {
        return 3;
        }
    if (vol <= 75) {
        return 4;
        }
    if (vol <= 100) {
        return 5;
        }
    return 1;
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
        else if(word[0] == 'j')
               {
                  joystick= atoi(word+1);
               }
        else if(word[0] == 'u')
               {
                  bluetooth= atoi(word+1);
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
            hud = 1;
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

    if(argc==1) {
        perror("No file path provided");
        return 1;
    }

    chdir(argv[1]);

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

    IMAGE_LAYER_T infoTextLayer;
    initImageLayer(&infoTextLayer,
                       320,
                       240,
                       type);
    createResourceImageLayer(&infoTextLayer, layer + 4);

    IMAGE_LAYER_T infoLayer;
    initImageLayer(&infoLayer,
                   320,
                   240,
                   type);
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
                       22,
                       13,
                       type);
     createResourceImageLayer(&aimageLayer, layer+2);

     IMAGE_LAYER_T joystickImageLayer;
        initImageLayer(&joystickImageLayer,
                       11,
                       11,
                       type);
     createResourceImageLayer(&joystickImageLayer, layer+2);

     IMAGE_LAYER_T bluetoothImageLayer;
        initImageLayer(&bluetoothImageLayer,
                       15,
                       13,
                       type);
     createResourceImageLayer(&bluetoothImageLayer, layer+2);
    
    int xOffset = info.width-bimageLayer.image.width-1;
    int yOffset = 1;
    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);
    addElementImageLayerOffset(&infoTextLayer,
                               (info.width - infoTextLayer.image.width) / 2,
                               (info.height - infoTextLayer.image.height) / 2,
                               display,
                               update);
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
                               (xOffset-wimageLayer.image.width)-aimageLayer.image.width-7,
                               yOffset,
                               display,
                               update);

    addElementImageLayerOffset(&joystickImageLayer,
                               (xOffset-wimageLayer.image.width)-aimageLayer.image.width - 22,
                               yOffset + 1,
                               display,
                               update);

    addElementImageLayerOffset(&bluetoothImageLayer,
                               (xOffset-wimageLayer.image.width)-aimageLayer.image.width - 38,
                               yOffset ,
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
            if (wifi_loaded == 0) {
                //TODO preload for efficienty
                if (loadPng(&(wimageLayer.image), WIFI_IMAGES[wifi-1]) == false)
                {
                    fprintf(stderr, "unable to wifi load %s\n", argv[optind]);
                }
                changeSourceAndUpdateImageLayer(&wimageLayer);
                wifi_loaded = 1;
            }
        }
        else if (!wifi)
        {
            clearLayer(&wimageLayer);
            wifi_loaded = 0;
        }
         if(audio >= 0)
                {
                    vol_image = getImageIconFromVolume(audio);
                    //TODO preload for efficienty
                    if (loadPng(&(aimageLayer.image), AUDIO_IMAGES[vol_image-1]) == false)
                    {
                        fprintf(stderr, "unable to audio load %s\n", argv[optind]);
                    }
                    changeSourceAndUpdateImageLayer(&aimageLayer);
                    //audio = -1;
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
        if(joystick > 0) {
            if (loadPng(&(joystickImageLayer.image), JOYSTICK_IMAGE) == false) {
                fprintf(stderr, "unable to joystick load %s\n", argv[optind]);
            }
            changeSourceAndUpdateImageLayer(&joystickImageLayer);
        }
        else if(joystick <= 0) {
            clearLayer(&joystickImageLayer);
        }
        if(bluetooth > 0) {
            if (loadPng(&(bluetoothImageLayer.image), BLUETOOTH_IMAGE) == false) {
                fprintf(stderr, "unable to bluetooth load %s\n", argv[optind]);
            }
            changeSourceAndUpdateImageLayer(&bluetoothImageLayer);
        }
        else if(bluetooth <= 0) {
            clearLayer(&bluetoothImageLayer);
        }
        if(infos > 0)
        {
            updateInfo(&infoLayer);
            updateInfoText(&infoTextLayer);
        }
        else if(infos <= 0)
        {
            clearLayer(&infoLayer);
            clearLayer(&infoTextLayer);
            infos_loaded = 0;
        }
        else
        {
            pause(); //stop thread
        }
        usleep(1000000); //sleep 1sec
    }
    //---------------------------------------------------------------------

    destroyImageLayer(&infoLayer);
    destroyImageLayer(&infoTextLayer);
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
    if (infos_loaded == 0) {
//    clearImageRGB(image, &backgroundColour);
    loadPng(&(infoLayer->image), INFO_IMAGE);
    changeSourceAndUpdateImageLayer(infoLayer);
    infos_loaded = 1;
    }
}

void updateInfoText(IMAGE_LAYER_T *infoLayer)
{
    clearImageRGB(&(infoLayer->image), &clearColor);
    char volumeText[60];
    char wifiText[20];
    char joystickText[20];
    char bluetoothText[20];
    snprintf(volumeText, sizeof(volumeText),"Volume: %d%%", audio);

    if (wifi > 0) {
        snprintf(wifiText, sizeof(wifiText),"WiFi Enabled");
    } else if (wifi <= 0) {
        snprintf(wifiText, sizeof(wifiText),"WiFi Disabled");
    }

    if (joystick > 0) {
        snprintf(joystickText, sizeof(joystickText),"Joystick Enabled");
    } else if (joystick <= 0) {
        snprintf(joystickText, sizeof(joystickText),"Joystick Disabled");
    }

    if (bluetooth > 0) {
        snprintf(bluetoothText, sizeof(bluetoothText),"Bluetooth Enabled");
    } else if (joystick <= 0) {
        snprintf(bluetoothText, sizeof(bluetoothText),"Bluetooth Disabled");
    }

    drawStringRGB(137, 29, volumeText, &textColour, &(infoLayer->image));
    drawStringRGB(137, 76, volumeText, &textColour, &(infoLayer->image));

    drawStringRGB(137, 124, wifiText, &textColour, &(infoLayer->image));
    drawStringRGB(137, 167, joystickText, &textColour, &(infoLayer->image));
    drawStringRGB(137, 214, bluetoothText, &textColour, &(infoLayer->image));

    changeSourceAndUpdateImageLayer(infoLayer);
}


