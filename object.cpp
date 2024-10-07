#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <algorithm>
#include <iostream>
#include <fstream>
#include "misc.h"
#include "object.h"
using namespace std;
const string OUTPUT_FILE_SCAD = "asm.scad";


Object::Object(): img(nullptr) {
    loc.x=0;
    loc.y=0;
    loc.w = ITEM_WIDTH;
    loc.h = ITEM_HEIGHT;
}

void Object::setLocation(const Point& xy){
    loc.x = xy.x;
    loc.y = xy.y;
}

void Object::draw(SDL_Renderer* rend){
    SDL_RenderCopy(rend, img, NULL, &loc);
    if(draggedOver){
        SDL_SetRenderDrawColor(rend,255,0,0,255);
    } else {
        SDL_SetRenderDrawColor(rend,255,255,255,255);
    }
    SDL_RenderDrawRect(rend, &loc);
}


// Module does not have any openscad code.  It's parent Operator has the code.
bool Module::saveScad(ostream& file){
    auto sp = parent.lock();
    if(sp){
        return sp->saveScad(file);
    }
    return false;
}


bool Input::saveScad(ostream& file){
    file << value;
    return true;
}
void Input::draw(SDL_Renderer* rend){
    // TODO: draw 'value' on screen
}


Modifier::Modifier(ModifierType mt): type(mt) {
    const string imgFileName = ROTATE == type ? "img/rotate.png" : "img/translate.png";
    img = ImageLoader::getImage(imgFileName);
}

bool Modifier::saveScad(ostream& file){
    file << (ROTATE==type ? "rotate" : "translate");
    file << "([";
    x.saveScad(file);
    file << ",";
    y.saveScad(file);
    file << ",";
    z.saveScad(file);
    file << "]) ";
    return true;
}

void Modifier::setLocation(const Point& xy){
    Object::setLocation(xy);
    int dx = loc.w - x.loc.w;
    Point delta(dx/2,loc.h-4*x.loc.y);
// TODO:
//    x.setLocation(xy+delta);
//    y.setLocation();
//    z.setLocation();
}

void Modifier::draw(SDL_Renderer* rend){
    Object::draw(rend);
    // TODO: draw inputs x,y,z
}

std::shared_ptr<Object> Modifier::clone(){
    auto obj = std::make_shared<Modifier>(type);
    obj->isClone = true;
    return obj;
}


Shape::Shape(ShapeType st): type(st) {
    string imgFileName;
    switch(type){
        case CUBE: imgFileName = "img/cube.png"; break;
        case CYLINDER: imgFileName = "img/cylinder.png"; break;
        case SPHERE: imgFileName = "img/sphere.png"; break;
    }
    img = ImageLoader::getImage(imgFileName); 
}

bool Shape::saveScad(ostream& file){
    switch(type){
        case CUBE: file << "cube([";
            a.saveScad(file);
            file << ",";
            b.saveScad(file);
            file << ",";
            c.saveScad(file);
            file << "],center=true);" << endl; 
            break;
        case CYLINDER: file << "cylinder(d=";
            a.saveScad(file);
            file << ",h=";
            b.saveScad(file);
            file << ",center=true);" << endl;
            break;
        case SPHERE: file << "sphere(d=";
            a.saveScad(file);
            file << ");" << endl;
            break;
        default: break;
    }
    return true;
}

void Shape::draw(SDL_Renderer* rend){
    Object::draw(rend);
    // TODO: draw inputs a,b,c
}
std::shared_ptr<Object> Shape::clone(){
    auto obj = std::make_shared<Shape>(type);
    obj->isClone = true;
    return obj;
}


bool DropZone::dropped(const Point& xy, std::shared_ptr<Object>const & obj){
    if(VIEW == type){ // if Module or Operator are dropped, generate code
        shared_ptr<Object> mod = dynamic_pointer_cast<Module>(obj);
        auto op = dynamic_pointer_cast<Operator>(obj);
        if(op){
            mod = op->getModule();
        }
        if(mod){
            img = mod->img;
            ofstream file(OUTPUT_FILE_SCAD, ios_base::out);
            mod->saveScad(file);
        }
    } else {
        // TODO: remove the object from main view, module view and DropZoneView
    }
    return true;
}
