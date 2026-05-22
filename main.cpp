#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_video.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "Spring.h"
#include "Camera.h"
#include "Point.h"

#include "OBJ_Loader.h"
#include "Vec3.h"

void springsInPlace(std::vector<Point> &pointVector, std::vector<Spring> &springsVector, double k_val = 30.0, double damp = 0.9){
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
  Vec3 onPlane = p - dot(n, (p-o))*n; // https://stackoverflow.com/questions/9605556/how-to-project-a-point-onto-a-plane-in-3d
  Vec3 pushV = unit_vector(onPlane - o);
  return pushV; 

}


int main (int argc, char *argv[]) {
  Camera cam(1280,720);
  cam.pos = Vec3(0,0,-5);
  bool running = true;
  

  std::vector<Point> pointVector;

  objl::Loader Loader;

  bool loadout = Loader.LoadFile(argv[1]);
  if (!loadout){
    return 1;
  }
  printf("Loaded file\n");

  size_t totalVertices = 0;
  for (const auto& mesh : Loader.LoadedMeshes) {
    totalVertices += mesh.Vertices.size();
  }
  pointVector.reserve(totalVertices);

  for (int i = 0; i < Loader.LoadedMeshes.size();i++){
    objl::Mesh curMesh = Loader.LoadedMeshes[i];
    for (int j = 0; j < curMesh.Vertices.size(); j++){
      pointVector.push_back(Point(Vec3(curMesh.Vertices[j]), 0.5)); 
    }
    //for (int j = 0; j < curMesh.Indices.size()-1; j ++)
		//{
    //  Point* P1 = &pointVector[curMesh.Indices[j]+currIndex];
    //  Point* P2 = &pointVector[curMesh.Indices[j+1]+currIndex];
    //  printf("len %i index p1 %i p2 %i  %p %p\n",(int)pointVector.size(),curMesh.Indices[j]+currIndex,curMesh.Indices[j+1]+currIndex,P1,P2);
    //  springVector.push_back(Spring(P1,P2));
    //  printf("%p %p\n",springVector[springVector.size()-1].P1,springVector[springVector.size()-1].P2);
		//}
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
  for (int i = 0; i < pointVector.size(); i++){
    connectedPoints.push_back(emptyV);
  }

  std::vector<Spring> springVector;



  springVector.reserve(Loader.LoadedMeshes.size() * 100); 
   
  int currIndex = 0; 
  for (const auto& curMesh : Loader.LoadedMeshes) {
  
    for (int j = 0; j < curMesh.Indices.size() - 1; j ++) { 
      int oldIdx1 = curMesh.Indices[j];
      int oldIdx2 = curMesh.Indices[j + 1];
      if (oldIdx1 == 0 || oldIdx2 == 0){
        continue;
      }
       
      int newIdx1 = oldToNew[oldIdx1+currIndex];
      int newIdx2 = oldToNew[oldIdx2+currIndex];
      
      if (newIdx1 == -1 || newIdx2 == -1) {
          continue;
      }
        
      Point* P1 = &pointVector[newIdx1];
      Point* P2 = &pointVector[newIdx2];
      
      double length = (P1->pos-P2->pos).length();
      //length = 0.2;

      springVector.push_back(Spring(P1, P2,100,length,0.9));

      bool isDupe = false;
      for (auto check : connectedPoints[P1->index]){
        if (check == P2) {isDupe = true; break;}
      }
      if (!isDupe){
        connectedPoints[P1->index].push_back(P2);
      }

      isDupe = false;
      for (auto check : connectedPoints[P2->index]){
        if (check == P1) {isDupe = true; break;}
      }
      if (!isDupe){
        connectedPoints[P2->index].push_back(P1);
      }
    }
    currIndex += curMesh.Vertices.size();
  }
  
  printf("Got springs\n");
  //springsInPlace(pointVector, springVector);


  printf("Springs amount %i\n",(int)springVector.size());

  //springsInPlace(pointVector, springVector,1,0.1);

  //pointVector[0].applyImpulse(Vec3(0,2,0));

  Vec3 gravity(0,-9.81,0);

  SDL_Event e;
  while(running){
    
    for (auto &spring : springVector){
      spring.update(); 
    }
    for (auto &point : pointVector){
      std::cout << point.pos.x << "\n";
      point.applyForce(gravity);
      if (point.pos.y < -2){
        point.applyImpulse(Vec3(0,-point.impulse().y*1.5,0));
        point.pos.y = -2;
        point.vel.x *= 0.6;
        point.vel.z *= 0.6;
        
      }

      point.applyForce(-0.5*point.vel*point.vel*1.2250*0.47*0.00785);
      
      for (auto a_point: connectedPoints[point.index]){
        for (auto b_point: connectedPoints[point.index] ){
          if (a_point == b_point || a_point == &point || b_point == &point) continue;
          Vec3 planeNormal = unit_vector(point.pos-a_point->pos);
          Vec3 direction = findDirectionVector(a_point->pos, planeNormal, b_point->pos);
          Vec3 springDir = unit_vector(point.pos-b_point->pos);
          double dotP = dot(planeNormal,springDir)+1;
          a_point->applyForce(direction*0.5*dotP);
          //printf("Index %i %i %i",point.index,a_point->index,b_point->index); direction.repr(); printf("\n");

        }


      }



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
                point.vel *=0.94;

              }
              pointVector[1].pos = Vec3(0,0,0);
              pointVector[1].vel = Vec3(0,0,0);
              
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
