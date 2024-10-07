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


/******************************** Operator *****************************************/
Operator::Operator(OperatorType ot): layout(0), type(ot){
    string imgFileName;
    switch(type){
        case UNION: imgFileName = "img/union.png"; break;
        case DIFFERENCE: imgFileName = "img/difference.png"; break;
        case INTERSECTION: imgFileName = "img/intersection.png"; break;
    }
    img = ImageLoader::getImage(imgFileName); 
}

bool Operator::saveScad(ostream& file){
    if(module){
        file << "module mod" << module.get() << "(){" << endl;
    }
    switch(type){
        case UNION: file << "union(){" << endl; break;
        case DIFFERENCE: file << "difference(){" << endl; break;
        case INTERSECTION: file << "intersection(){" << endl; break;
        default: break;
    }
    layout.saveScad(file);
    file << "}" << endl; // close operator
    if(module){ file << "}" << endl << endl << "mod" << module.get() << "();" << endl; }
    return true;
}


std::shared_ptr<Object> Operator::clone(){
    auto obj = std::make_shared<Operator>(type);
    obj->isClone = true;
    return obj;
}

bool Operator::dropped(const Point& xy, std::shared_ptr<Object>const & obj){
    // TODO: check if xy is in loc?
    if(!isClone){ return false; } // originals can only be dragged
    cout << "Adding an object to an operator." << endl;
    layout.addObject(obj);
    layout.loc.w += ITEM_WIDTH; // TODO: this is wrong
    layout.setLocation(Point(loc.x+ITEM_WIDTH*(module?2:1), loc.y));
    return true;
}

std::shared_ptr<Object> Operator::getModule(){ // not virtual
    if(!module){ module = std::make_shared<Module>(shared_from_this()); }
    if( !makeObjectImage( module ) ){
        std::cout << "ERROR while creating module image." << std::endl;
    }
    return module;
}

void Operator::setLocation(const Point& xy){
    Object::setLocation(xy);
    loc.w = ITEM_WIDTH;
    if(module){
        module->setLocation( Point(xy.x+ITEM_WIDTH, xy.y) );
        loc.w += module->loc.w;
    }
    layout.setLocation(Point(xy.x+ITEM_WIDTH*(module?2:1), xy.y));
    loc.w += layout.loc.w;
}

void Operator::draw(SDL_Renderer* rend){
    SDL_Rect r;  // TODO: do this better
    r.x = loc.x;
    r.y = loc.y;
    r.w = ITEM_WIDTH;
    r.h = ITEM_HEIGHT;
    SDL_RenderCopy(rend, img, NULL, &r);
    if(module){ module->draw(rend); }
    layout.draw(rend);

    if(draggedOver){
        SDL_SetRenderDrawColor(rend,255,0,0,255);
    } else {
        SDL_SetRenderDrawColor(rend,255,255,255,255);
    }
    SDL_RenderDrawRect(rend, &loc);
}
