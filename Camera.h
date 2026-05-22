#pragma once
#include "Spring.h"
#include "Vec3.h"
#include "Point.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <vector>

class Camera{
  private:
    SDL_Renderer* renderer;
    SDL_Window* window;
  
    int w,h;

  public:
    Vec3 pos;
    double fol = 0.5;

    Camera(int xw, int yh){
      
      w = xw;
      h = yh;

      SDL_Init(SDL_INIT_VIDEO);
      window = SDL_CreateWindow("Name", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, xw, yh, 0);
      renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    };

    bool project(Vec3 p, Vec3 &p_prime){

      p_prime.x = (p.x-pos.x)/(p.z-pos.z);
      p_prime.y = -(p.y-pos.y)/(p.z-pos.z);


      return p.z >= pos.z + fol; 
    }

    void render(std::vector<Point> &pS, std::vector<Spring> &sP){
      SDL_SetRenderDrawColor( renderer, 60, 60, 60, 255 );
      SDL_RenderClear( renderer );
      

      
      Vec3 proj1, proj2;

      SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );

      for (auto s: sP){
        if(project(s.P1->pos,proj1) && project(s.P2->pos,proj2)){
          proj1*=h;
          proj2*=h;
          
          SDL_RenderDrawLine(renderer, proj1.x+w/2.0, proj1.y+h/2.0, proj2.x+w/2.0, proj2.y+h/2.0);

        }
      }


      Vec3 projected;
      SDL_Rect draw;
      
      draw.w = 4;
      draw.h = 4;

      SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
      for (auto p: pS){
        if (project(p.pos,projected)){

          projected*=h;

          draw.x = projected.x-draw.w/2.0+w/2.0;
          draw.y = projected.y-draw.h/2.0+h/2.0;


          //printf("Rendering point: %i %i\n",draw.x,draw.y); 

          SDL_RenderFillRect(renderer,&draw);
        };
      }
      SDL_RenderPresent(renderer);
    }
};


