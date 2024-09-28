#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <algorithm>
#include <iostream>
#include "misc.h"
#include "object.h"
using namespace std;

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


bool Module::saveScad(ostream& file){ return true; }


bool Input::saveScad(ostream& file){
    cout << value;
    return true;
}
void Input::draw(SDL_Renderer* rend){
    // TODO: draw 'value' on screen
}


Modifier::Modifier(ModifierType mt): type(mt) {
    const string imgFileName = ROTATE == type ? "rotate.png" : "translate.png";
    img = ImageLoader::getImage(imgFileName);
}

bool Modifier::saveScad(ostream& file){
    file << (ROTATE==type ? "rotate([" : "translate([");
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



Operator::Operator(int width, OperatorType ot): layout(width), type(ot){
    string imgFileName;
    switch(type){
        case UNION: imgFileName = "union.png"; break;
        case DIFFERENCE: imgFileName = "difference.png"; break;
        case INTERSECTION: imgFileName = "intersection.png"; break;
    }
    img = ImageLoader::getImage(imgFileName); 
}
bool Operator::saveScad(ostream& file){ return true; }


Shape::Shape(ShapeType st): type(st) {
    string imgFileName;
    switch(type){
        case CUBE: imgFileName = "cube.png"; break;
        case CYLINDER: imgFileName = "cylinder.png"; break;
        case SPHERE: imgFileName = "sphere.png"; break;
    }
    img = ImageLoader::getImage(imgFileName); 
}
bool Shape::saveScad(ostream& file){ return true; }
void Shape::draw(SDL_Renderer* rend){
    Object::draw(rend);
    // TODO: draw inputs a,b,c
}


void DropZoneView::drag(const Point& xy){}
void DropZoneView::dragEnd(){}
void DropZoneView::dropped(const Point& xy, std::shared_ptr<Object>const & obj){}

void DropZoneDelete::drag(const Point& xy){}
void DropZoneDelete::dragEnd(){}
void DropZoneDelete::dropped(const Point& xy, std::shared_ptr<Object>const & obj){}
