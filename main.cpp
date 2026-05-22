#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_video.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>

#include "Spring.h"
#include "Camera.h"
#include "Point.h"
#include "Vec3.h"

// Define the implementation for tinyobjloader before including it
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void springsInPlace(std::vector<Point> &pointVector, std::vector<Spring> &springsVector, double k_val = 50.0, double damp = 0.9){
  for (int j = 0; j < (int)pointVector.size(); j++){
    for (int k = 0; k < (int)pointVector.size(); k++){
      double length = (pointVector[j].pos-pointVector[k].pos).length();
      length *= 1;
      if (j > k){
        springsVector.push_back(Spring(&pointVector[j],&pointVector[k],k_val,length,damp));
      }else if (j != k){
        springsVector.push_back(Spring(&pointVector[k],&pointVector[j],k_val,length,damp));
      }
    }
  }
}

Vec3 findDirectionVector(Vec3 o, Vec3 n, Vec3 p){
  Vec3 onPlane = p - dot(n, (p-o))*n; 
  Vec3 pushV = unit_vector(onPlane - o);
  return pushV; 
}

int main (int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <model.obj>\n", argv[0]);
    return 1;
  }

  Camera cam(1280,720);
  cam.pos = Vec3(0,0,-5);
  bool running = true;
  
  std::vector<Point> pointVector;

  // tinyobjloader setup variables

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  
  std::string warn;
  std::string err;
  
  bool loadout = tinyobj::LoadObj(&attrib, &shapes, &materials,&err, argv[1],nullptr,false);

  if (!warn.empty()) {
    printf("Warning: %s\n", warn.c_str());
  }
  if (!err.empty()) {
    fprintf(stderr, "Error: %s\n", err.c_str());
  }
  if (!loadout) {
    return 1;
  }
  printf("Loaded file via tinyobjloader\n");

  // Determine total indices across all shapes to properly size/reserve vectors
  size_t totalVertices = 0;
  for (const auto& shape : shapes) {
    totalVertices += shape.mesh.indices.size();
  }
  pointVector.reserve(totalVertices);

  // Read vertices out using shape indices
  for (size_t s = 0; s < shapes.size(); s++) {
    for (size_t i = 0; i < shapes[s].mesh.indices.size(); i++) {
      tinyobj::index_t idx = shapes[s].mesh.indices[i];
      
      float vx = attrib.vertices[3 * idx.vertex_index + 0];
      float vy = attrib.vertices[3 * idx.vertex_index + 1];
      float vz = attrib.vertices[3 * idx.vertex_index + 2];
      
      pointVector.push_back(Point(Vec3(vx, vy, vz), 0.5)); 
    }
  }

  std::vector<Point> uniquePoints;
  uniquePoints.reserve(pointVector.size());

  std::vector<int> oldToNew(pointVector.size(), -1);

  for (size_t i = 0; i < pointVector.size(); ++i) {
    bool isDuplicate = false;
    for (size_t j = 0; j < uniquePoints.size(); ++j) {
      if ((pointVector[i].pos - uniquePoints[j].pos).length() < 0.0001f) {
        oldToNew[i] = static_cast<int>(j);
        isDuplicate = true;
        break;
      }
    }
    if (!isDuplicate) {
      oldToNew[i] = static_cast<int>(uniquePoints.size());
      uniquePoints.push_back(pointVector[i]);
    }
  }

  pointVector = std::move(uniquePoints);
  printf("Got Points\n");

  std::vector<std::vector<Point*>> connectedPoints;
  
  int i = 0;
  for (auto& p_p: pointVector){
    p_p.index = i;
    i++;
  }

  printf("Got pvector\n");
  connectedPoints.reserve(i);

  std::vector<Point*> emptyV;
  for (size_t i = 0; i < pointVector.size(); i++){
    connectedPoints.push_back(emptyV);
  }

  std::vector<Spring> springVector;
  springVector.reserve(shapes.size() * 100); 
   
  int currIndex = 0; 
  for (const auto& shape : shapes) {
    // tinyobjloader groups faces by vertex loops. 
    // We parse indices step by step to connect neighboring points as springs
    for (size_t j = 0; j < shape.mesh.indices.size() - 1; j++) { 
      int oldIdx1 = static_cast<int>(j);
      int oldIdx2 = static_cast<int>(j + 1);
      
      if (oldIdx1 == 0 || oldIdx2 == 0){
        // Retaining your original code logic constraint
        continue;
      }
       
      int newIdx1 = oldToNew[oldIdx1 + currIndex];
      int newIdx2 = oldToNew[oldIdx2 + currIndex];
      
      if (newIdx1 == -1 || newIdx2 == -1) {
          continue;
      }
        
      Point* P1 = &pointVector[newIdx1];
      Point* P2 = &pointVector[newIdx2];
      
      double length = (P1->pos - P2->pos).length();

      springVector.push_back(Spring(P1, P2, 50, 0.5, 0.9));

      bool isDupe = false;
      for (auto check : connectedPoints[P1->index]){
        if (check == P2) { isDupe = true; break; }
      }
      if (!isDupe){
        connectedPoints[P1->index].push_back(P2);
      }

      isDupe = false;
      for (auto check : connectedPoints[P2->index]){
        if (check == P1) { isDupe = true; break; }
      }
      if (!isDupe){
        connectedPoints[P2->index].push_back(P1); // Fixed copy-paste bug from your original snippet: changed P2 to P1 here
      }
    }
    currIndex += shape.mesh.indices.size();
  }
  
  printf("Got springs\n");
  printf("Springs amount %i\n",(int)springVector.size());

  Vec3 gravity(0,-9.81,0);

  SDL_Event e;
  while(running){
    
    for (auto &spring : springVector){
      spring.update(); 
    }
    for (auto &point : pointVector){
      //std::cout << point.pos.x << "\n";
      //point.applyForce(gravity);
      


      //point.applyForce(-0.5*point.vel*point.vel*1.2250*0.47*0.00785);
      
      for (auto a_point: connectedPoints[point.index]){
        for (auto b_point: connectedPoints[point.index] ){
          if (a_point == b_point || a_point == &point || b_point == &point) continue;
          Vec3 planeNormal = unit_vector(point.pos-a_point->pos);
          Vec3 direction = findDirectionVector(a_point->pos, planeNormal, b_point->pos);
          Vec3 springDir = unit_vector(point.pos-b_point->pos);
          double dotP = 0.5*(dot(planeNormal,springDir)-1);
          Vec3 force = direction*dotP*50;
          force.repr();
          a_point->applyForce(force);
        }
      }


      if ((point.pos.y < -2) && false){
        point.applyImpulse(Vec3(0,-point.impulse().y,0));
        point.pos.y = -2;
        point.vel *= 0.7;
        point.vel.y = 0;
        //Vec3 resistance = -unit_vector(point.vel);
        //resistance.y = 0;
        //resistance *= point.forceAcc.y*0.4;
        
        //point.applyForce(resistance);
      }

      point.vel *= 0.96;
      point.tickTime(0.016/4);
    }
    cam.render(pointVector,springVector);
    SDL_Delay(16);

    while (SDL_PollEvent(&e)){
      switch (e.type) {
        case SDL_QUIT:
          running = false;
          break;
        case SDL_KEYDOWN:
          switch (e.key.keysym.sym){
            case SDLK_w:
              pointVector[0].applyImpulse(Vec3(-1,20,0));
            break;
            case SDLK_m:
              for (auto &point : pointVector){
                point.vel *=0.999;
              }
              pointVector[1].pos = Vec3(0,0,0);
              pointVector[1].vel = Vec3(0,0,0);
            break;
            case SDLK_i:
              for (auto &point : pointVector){
                point.applyImpulse(Vec3(0,0,1));
              }
            break;
          };
          break;
        default:
          break;
      }
    }
  }

  SDL_Quit();
  return 0;
}
