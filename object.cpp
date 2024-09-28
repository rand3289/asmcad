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


/**************************** FLowLayout ***************************** */
FlowLayout::FlowLayout(int width) {
    loc.w = width;
}

void FlowLayout::setLocation(const Point& xy){
    Object::setLocation(xy);
    Point next = xy;
    for(auto& objPtr: children){
        int nextx = next.x+objPtr->loc.w;
        if(nextx > loc.x+loc.w){ // falls off the screen on the right
            next.x = loc.x;
            next.y = next.y+ITEM_HEIGHT;
        }
        objPtr->setLocation(next);
        next.x+=objPtr->loc.w;
    }
    loc.h = (next.y - loc.y)+ITEM_HEIGHT; // if we increased the size 
}


//int FlowLayout::findChildIndex(shared_ptr<Object>& obj){
//    auto it = find(begin(children), end(children), obj);
//    return end(children) == it ? -1 : distance(begin(children), it);
//}

void FlowLayout::add(shared_ptr<Object>const & obj){
    children.push_back(obj);
}

bool FlowLayout::saveScad(ostream& file){ 
    for(auto& o: children){
        o->saveScad(file);
    }
    return true;
}

bool FlowLayout::removeChild(shared_ptr<Object>& obj){
    auto it = find(begin(children), end(children), obj);
    if(end(children) == it) { return false; }
    children.erase(it);
    return true;
}

void FlowLayout::draw(SDL_Renderer* rend){
    for(auto& objPtr: children){
        objPtr->draw(rend);
    }
}

shared_ptr<Object> FlowLayout::takeObject(const Point& xy){
    for(auto it = begin(children); it!=end(children); ++it ){
        if(xy.inRectangle((*it)->loc)){
            auto ptr = (*it);
            children.erase(it);
            return ptr;
        }
    }
    return shared_ptr<Object>();
}

void FlowLayout::drag(const Point& xy){
    cout << ".";
}

void FlowLayout::dragEnd(){
    cout << "|";
}

void FlowLayout::dropped(const Point& xy, shared_ptr<Object>const & obj){
    add(obj);
    cout << "added object" << endl;
}


/******************************* VerticalLayout *************************************/
VerticalLayout::VerticalLayout(int width): FlowLayout(width) {}

void VerticalLayout::setLocation(const Point& xy){
    Point current = xy;
    for(auto objPtr: children){
        objPtr->setLocation(current);
        current.y+= objPtr->loc.h;
    }
    loc.h = current.y - loc.y;
}

//void VerticalLayout::draw(SDL_Renderer* rend){
//    for(auto objPtr: children){
//        objPtr->draw(rend);
//    }
//}
void VerticalLayout::scroll(const Point& xy, int y){}
void VerticalLayout::drag(const Point& xy){}
void VerticalLayout::dragEnd(){}
void VerticalLayout::dropped(const Point& xy, std::shared_ptr<Object>const & obj){}


bool DropZoneView::saveScad(ostream& file){ return true; }
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
