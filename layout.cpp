#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <algorithm>
#include <iostream>
#include "misc.h"
#include "object.h"
using namespace std;


void VerticalLayout::scroll(const Point& xy, int y){}

void VerticalLayout::setLocation(const Point& xy){
    cout << "VLayout setting location at ("<< xy.x << ","<<xy.y<<")"<< endl;
    Object::setLocation(xy);
    Point current = xy;
    for(auto objPtr: children){
        objPtr->setLocation(current);
        current.y+= objPtr->loc.h;
    }
    loc.h = current.y - xy.y;
}


void FlowLayout::setLocation(const Point& xy){
    cout << "HLayout setting location at ("<< xy.x << ","<<xy.y<<")"<< endl;
    Object::setLocation(xy);
    Point next = xy;
    for(auto& objPtr: children){
        int nextx = next.x+objPtr->loc.w;
        if(nextx > loc.x+loc.w){ // falls off the screen on the right
            next.x = xy.x;
            next.y = next.y+ITEM_HEIGHT;
        }
        objPtr->setLocation(next);
        next.x += objPtr->loc.w;
    }
    // TODO: this is wrong!!!
    loc.h = (next.y - xy.y)+ITEM_HEIGHT; // if we increased the size 
}

bool FlowLayout::saveScad(ostream& file){
    for(auto& o: children){
        o->saveScad(file);
    }
    return true;
}

void FlowLayout::addObject(shared_ptr<Object>const & obj){
    children.push_back(obj);
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
    if(draggedOver){
        SDL_SetRenderDrawColor(rend,255,0,0,255);
    } else {
        SDL_SetRenderDrawColor(rend,255,255,255,255);
    }
    SDL_RenderDrawRect(rend, &loc);
}

shared_ptr<Object> FlowLayout::takeObject(const Point& xy){
    for(auto it = begin(children); it!=end(children); ++it ){
        if(xy.inRectangle((*it)->loc)){
            auto obj = (*it)->takeObject(xy); // take from its children
            if(obj){
                cout << "Removing object from children. children size=" << children.size() <<endl;
                setLocation(Point(loc.x, loc.y)); // perform layout
                return obj;
            }
            if(disableDragDrop){ continue; }
            auto ptr = (*it);
            children.erase(it);
            cout << "Removing Object. children size=" << children.size() << endl;
            setLocation(Point(loc.x, loc.y)); // perform layout
            return ptr;
        }
    }
    return shared_ptr<Object>();
}

bool FlowLayout::dropped(const Point& xy, shared_ptr<Object>const & obj){
    for(auto& o: children){
        if(xy.inRectangle(o->loc)){
            if(o->dropped(xy,obj) ){
                setLocation(Point(loc.x,loc.y)); // perform layout
                cout << "Added an object to one of the children." << endl;
                return true;
            }
        }
    }
    if(disableDragDrop){ return false; }
    addObject(obj);
    setLocation(Point(loc.x,loc.y)); // perform layout
    cout << "Added an object. children size=" << children.size() << endl;
    return true;
}
