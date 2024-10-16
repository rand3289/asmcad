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
    SDL_RenderCopy(rend, img.get(), NULL, &loc);
    if(draggedOver){
        SDL_SetRenderDrawColor(rend, 255, 0, 0, SDL_ALPHA_OPAQUE);
    } else {
        SDL_SetRenderDrawColor(rend, 255, 255, 255, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderDrawRect(rend, &loc);
}


// Module does not have any openscad code.  It's parent Operator has the code.
bool Module::saveScad(ostream& file){
    auto sp = parent.lock();
    if(sp){
        if(isClone){ // clones call the module.  Non-clones save the code. TODO: will this work???
            file << "mod"<< sp.get() << "();";
            return true;
        }
        return sp->saveScad(file);
    }
    return false;
}

std::shared_ptr<Object> Module::clone(){
    auto sp = parent.lock();
    if(!sp){ return shared_ptr<Object>(); }
    auto obj = std::make_shared<Module>(sp);
    obj->isClone = true;
    return obj;
}

shared_ptr<Operator> Module::getOperator(){
    return dynamic_pointer_cast<Operator>( parent.lock() );
}



bool Input::saveScad(ostream& file){
    file << value;
    return true;
}

std::shared_ptr<Text> Input::printer;

void Input::draw(SDL_Renderer* rend){
    if(!enabled){ return; }
    SDL_SetRenderDrawColor(rend, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(rend, &loc);
    char buff[64];
    sprintf(buff,"%.2f", value);
    static SDL_Color color = { 255, 255, 0, SDL_ALPHA_OPAQUE };
    printer->print(string(buff), loc.x, loc.y, rend, color); // TODO: fix that string conversion
}

std::shared_ptr<Object> Input::click(const Point& xy){
    if( !xy.inRectangle(loc) ){ return shared_ptr<Object>(); }
    std::cout << '<' << endl;
    delta = 1.0;
    return shared_from_this();
}

std::shared_ptr<Object> Input::clickr(const Point& xy){ // change it slowly after right click
    if( !xy.inRectangle(loc) ){ return shared_ptr<Object>(); }
    std::cout << '>' << endl;
    delta = 0.01;
    return shared_from_this();
}


Modifier::Modifier(ModifierType mt): type(mt) {
    string fName;
    switch(type){
        case ROTATE:    fName = "img/rotate.png";    break;
        case TRANSLATE: fName = "img/translate.png"; break;
        case SCALE:     fName = "img/scale.png";     break;
    }
    img = ImageLoader::getImage(fName);
}

bool Modifier::saveScad(ostream& file){
    switch(type){
        case ROTATE:    file << "rotate";    break;
        case TRANSLATE: file << "translate"; break;
        case SCALE:     file << "scale";     break;
    }
    file << "([";
    x->saveScad(file);
    file << ",";
    y->saveScad(file);
    file << ",";
    z->saveScad(file);
    file << "]) ";
    return true;
}

std::shared_ptr<Object> Modifier::clone(){
    auto obj = std::make_shared<Modifier>(type);
    obj->isClone = true;
    return obj;
}


Shape::Shape(ShapeType st): type(st) {
    x->setValue(10.0);
    y->setValue(10.0);
    z->setValue(10.0);
    string imgFileName;
    switch(type){
        case CUBE: imgFileName = "img/cube.png"; break;
        case CYLINDER: imgFileName = "img/cylinder.png"; break;
        case SPHERE: imgFileName = "img/sphere.png"; 
            x->disable(); // sphere has only one variable
            y->disable();
            break;
    }
    img = ImageLoader::getImage(imgFileName); 
}

bool Shape::saveScad(ostream& file){
    switch(type){
        case CUBE: file << "cube([";
            x->saveScad(file);
            file << ",";
            y->saveScad(file);
            file << ",";
            z->saveScad(file);
            file << "],center=true);" << endl; 
            break;
        case CYLINDER: file << "cylinder(h=";
            x->saveScad(file);
            file << ",d1=";
            y->saveScad(file);
            file << ",d2=";
            z->saveScad(file);
            file << ",center=true);" << endl;
            break;
        case SPHERE: file << "sphere(d=";
            z->saveScad(file);
            file << ");" << endl;
            break;
        default: break;
    }
    return true;
}

std::shared_ptr<Object> Shape::clone(){
    auto obj = std::make_shared<Shape>(type);
    obj->isClone = true;
    return obj;
}


XYZ::XYZ(){
    x = make_shared<Input>();
    y = make_shared<Input>();
    z = make_shared<Input>();
}

void XYZ::draw(SDL_Renderer* rend){
    Object::draw(rend);
    x->draw(rend);
    y->draw(rend);
    z->draw(rend);
}

void XYZ::setLocation(const Point& xy){
    Object::setLocation(xy);
    x->setLocation(Point(xy.x+10,xy.y+90));
    y->setLocation(Point(xy.x+10,xy.y+110));
    z->setLocation(Point(xy.x+10,xy.y+130));
}

std::shared_ptr<Object> XYZ::click(const Point& xy){
    auto o = x->click(xy);
    if(o){ return o; }
    o = y->click(xy);
    if(o){ return o; }
    o = z->click(xy);
    if(o){ return o; }
    return shared_ptr<Object>();
}

std::shared_ptr<Object> XYZ::clickr(const Point& xy){
    auto o = x->clickr(xy);
    if(o){ return o; }
    o = y->clickr(xy);
    if(o){ return o; }
    o = z->clickr(xy);
    if(o){ return o; }
    return shared_ptr<Object>();
}


bool DropZone::dropped(const Point& xy, std::shared_ptr<Object>const & obj){
    if(VIEW == type){ // if Module or Operator are dropped, generate code
        shared_ptr<Module> mod = dynamic_pointer_cast<Module>(obj);
        shared_ptr<Operator> op = dynamic_pointer_cast<Operator>(obj);
        if(op){
            mod = op->getModule();
        } else {
            op = mod->getOperator();
        }
        if(mod){
            img = mod->img;
        }
        if(op){
            ofstream file(OUTPUT_FILE_SCAD, ios_base::out | ios::trunc);
            root->saveScad(file);
            file << endl << "mod" << op.get() << "();" << endl;
        }
    } else {
        // TODO: remove the object from main view, module view and DropZoneView
    }
    return true;
}
