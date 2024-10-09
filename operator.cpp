#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <iostream>
#include <fstream>
#include "object.h"
using namespace std;


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
    layout.loc.w += ITEM_WIDTH; // TODO: this is wrong (for 2 reasons!!!)
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
    SDL_RenderCopy(rend, img.get(), NULL, &r);
    if(module){ module->draw(rend); }
    layout.draw(rend);

    if(draggedOver){
        SDL_SetRenderDrawColor(rend,255,0,0,255);
    } else {
        SDL_SetRenderDrawColor(rend,255,255,255,255);
    }
    SDL_RenderDrawRect(rend, &loc);
}
