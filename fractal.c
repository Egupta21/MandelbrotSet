// fractal.c

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <stdbool.h>
#include <math.h>
#include <complex.h>

#define IMAGEWIDTH            980
#define IMAGEHEIGHT           720
#define IMAGEXPOS             100
#define IMAGEYPOS             50

#define IMAGEMINX             0
#define IMAGEMINY             0

// change gamma and max it count to adjust look
#define MAXITCOUNT            500
#define GAMMA                 2.1

#define REMINANIMATION        -2.0
#define REMAXANIMATION        1.0
#define IMMINANIMATION        -1.5
#define IMMAXANIMATION        1.5

#define REMINIMAGE            -2.0
#define REMAXIMAGE            1.5
#define IMMINIMAGE            -1.5
#define IMMAXIMAGE            1.5

typedef struct
{
   double ReMin;
   double ReMax;
   double ImMin;
   double ImMax;
} mouseConfig;


// void colorPixel(SDL_Renderer * rend, int exitIterationCount, int x, int y)
// {
//    uint8_t brightness = 255 * ((double) exitIterationCount / MAXITCOUNT);
//    SDL_SetRenderDrawColor(rend, brightness, brightness, brightness, brightness);
//    SDL_RenderDrawPoint(rend, x, y);
// }

uint16_t calculatePixelColor(double Re, double Imaginary)
{
   int exitItCount = 0;
   double complex z = 0;
   for(exitItCount = 0; exitItCount < MAXITCOUNT; ++exitItCount){
      z = z*z + (Re + Imaginary*I);
      if(cabs(z) > 2)
      {
         break;
      }
   }

    double t = (double)exitItCount / MAXITCOUNT;

    t = pow(t, GAMMA);

   return (uint16_t)(t * 65535);
   
   // user this return statement to allow for more midtones
   //return (65535 * ((double) exitItCount / MAXITCOUNT));
}

/*
Returns the real number value of c derived from mapping the pixel value in the x axis.

xAxisPixel - current X axis pixel value
ReMin - Minimum possible real number value for c
ReMax - Maximum possible real number value for c
*/
double pixelXAxisToRealNumber(int xAxisPixel, double ReMin, double ReMax)
{
   double slope = (ReMax - ReMin) / (IMAGEWIDTH - IMAGEMINX);
   return ReMin + slope * (xAxisPixel);
}

void writeToPixelBuffer(uint16_t pixelBuffer[IMAGEHEIGHT*IMAGEWIDTH], uint16_t pixelColor, int r, int c)
{
    pixelBuffer[r * IMAGEWIDTH + c] = pixelColor;
}

/*
Returns the real number value of c derived from mapping the pixel value in the x axis.

YAxisPixel - current Y axis pixel value
ImaginaryMin - Minimum possible imaginary number value for c
ImaginaryMax - Maximum possilbe imaginary number value for c
*/
double pixelYAxisToImaginaryNumber(int yAxisPixel, double ImaginaryMin, double ImaginaryMax)
{
   double slope = (ImaginaryMax - ImaginaryMin) / (IMAGEHEIGHT - 1);
   return ImaginaryMax - slope * yAxisPixel;
}

int main(void)
{
   if(SDL_Init(SDL_INIT_VIDEO) != 0)
   {
      return 1;
   }
   bool isRunning = true;
   SDL_Window *FractalWindow = SDL_CreateWindow("Fractal Image", IMAGEXPOS, IMAGEYPOS, IMAGEWIDTH, IMAGEHEIGHT, SDL_WINDOW_SHOWN);
   SDL_Renderer* rend = SDL_CreateRenderer(FractalWindow, -1, SDL_RENDERER_ACCELERATED);
   SDL_Texture* texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, IMAGEWIDTH, IMAGEHEIGHT);
   SDL_Event event;

   mouseConfig config = {-2.0, 1.5, -1.5, 1.5};

   double targetRe = -0.743643887037151;
   double targetIm = 0.13182590420533;

   // make false to not autozoom
   bool autoZoom = false;

   if (!FractalWindow) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

   while(isRunning)
   {

      if(autoZoom)
      {
         //code for auto zoom

         double width  = config.ReMax - config.ReMin;
         double height = config.ImMax - config.ImMin;

         double centerX = (config.ReMin + config.ReMax) / 2;
         double centerY = (config.ImMin + config.ImMax) / 2;

         double step = 0.1;
         double newCenterX = centerX + (targetRe - centerX) * step;
         double newCenterY = centerY + (targetIm - centerY) * step;

         double zoomFactor = 0.9;
         double newWidth  = width * zoomFactor;
         double newHeight = height * zoomFactor;

         config.ReMin = newCenterX - newWidth/2;
         config.ReMax = newCenterX + newWidth/2;
         config.ImMin = newCenterY - newHeight/2;
         config.ImMax = newCenterY + newHeight/2;
      }


      while(SDL_PollEvent(&event))
      {
         if(event.type == SDL_QUIT)
         {
            isRunning = false;
         }
         if(event.type == SDL_MOUSEWHEEL)
         {
            if(event.type == SDL_MOUSEWHEEL)
            {
               int mouseX, mouseY;
               SDL_GetMouseState(&mouseX, &mouseY);

               double ReMouse = config.ReMin + (mouseX / (double)IMAGEWIDTH) * (config.ReMax - config.ReMin);
               double ImMouse = config.ImMax - (mouseY / (double)IMAGEHEIGHT) * (config.ImMax - config.ImMin);

               double zoomFactor = 1.0;
               if(event.wheel.y > 0)
                  zoomFactor = 1.0 / 1.1;
               else if(event.wheel.y < 0)
                  zoomFactor = 1.1;

               double width = (config.ReMax - config.ReMin) * zoomFactor;
               double height = (config.ImMax - config.ImMin) * zoomFactor;

               config.ReMin = ReMouse - width * ((double)mouseX / IMAGEWIDTH);
               config.ReMax = config.ReMin + width;
               config.ImMin = ImMouse - height * (1.0 - (double)mouseY / IMAGEHEIGHT);
               config.ImMax = config.ImMin + height;
            }

         }
         
      }

   // SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
   // SDL_RenderClear(rend);

   uint16_t pixelBuffer[IMAGEHEIGHT*IMAGEWIDTH];
   
   int r;
   int c;
   for(r = 0; r < IMAGEHEIGHT; r++)
   {
      for(c = 0; c < IMAGEWIDTH; c++)
      {
         uint16_t pixelColor = calculatePixelColor(pixelXAxisToRealNumber(c, config.ReMin, config.ReMax), pixelYAxisToImaginaryNumber(r, config.ImMin, config.ImMax));
         writeToPixelBuffer(pixelBuffer, pixelColor, r, c);
      }
      //SDL_RenderPresent(rend);
      //SDL_Delay(2);
   }

   SDL_UpdateTexture(texture, NULL, pixelBuffer, IMAGEWIDTH * sizeof(pixelBuffer[0]));
   SDL_RenderCopy(rend, texture, NULL, NULL);
   SDL_RenderPresent(rend);

   SDL_Delay(50);
   }
   printf("Closed Window\n");
   return 0;
}
