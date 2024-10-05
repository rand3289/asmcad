#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <algorithm>
#include <iostream>
#include "misc.h"
#include "object.h"
using namespace std;


void VerticalLayout::scroll(const Point& xy, int y){
    cout << y << " ";
    cout.flush();
}

void VerticalLayout::setLocation(const Point& xy){
    Object::setLocation(xy);
    Point next = xy;
    for(auto& objPtr: children){
        objPtr->setLocation(next);
        next.y+= objPtr->loc.h;
    }
    cout << "VLayout setting location at ("<< xy.x << ","<<xy.y<<") size (" << loc.w << "," << loc.h << ")" << endl;
}


void FlowLayout::setLocation(const Point& xy){
    Object::setLocation(xy);
    Point next = xy;
    int rowHeight = 0;
    for(auto& objPtr: children){
        rowHeight = max(rowHeight, objPtr->loc.h);
        int xmax = next.x+objPtr->loc.w;
        if(xmax-2 > loc.x+loc.w){ // falls off the screen on the right
            next.x = xy.x;
            next.y = next.y+rowHeight;
            rowHeight = objPtr->loc.h;
        }
        objPtr->setLocation(next);
        next.x += objPtr->loc.w;
    }
    loc.h = (next.y - xy.y)+rowHeight; // if it increased in size 
    cout << "HLayout setting location at ("<< xy.x << ","<<xy.y<<") size (" << loc.w << "," << loc.h << ")" << endl;
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
                if(*it == obj){ // it is the child itself
                    children.erase(it);
                }
                cout << "Removing an object from one of the children. children size=" << children.size() <<endl;
                setLocation(Point(loc.x, loc.y)); // perform layout
                return obj;
            }
            if(disableDragDrop){ return shared_ptr<Object>(); }
            auto ptr = (*it);
            cout << "Removing Object.  Children size=" << children.size() << endl;
            children.erase(it);
            cout << "Revoved Object.  Children size=" << children.size() << endl;
            setLocation(Point(loc.x, loc.y)); // perform layout
            return ptr;
        }
    }
    return shared_ptr<Object>();
}

bool FlowLayout::dropped(const Point& xy, shared_ptr<Object>const & obj){
    if( ! xy.inRectangle(loc)){ return false; }
    for(auto& o: children){
        if(xy.inRectangle(o->loc)){
            if(o->dropped(xy,obj) ){
                setLocation(Point(loc.x,loc.y)); // perform layout
                cout << "Added an object to one of the children." << endl;
                return true;
//            } else {
//                return false;
            }
        }
    }
    if(disableDragDrop){ return false; }
    addObject(obj);
    setLocation(Point(loc.x,loc.y)); // perform layout
    cout << "Added an object. children size=" << children.size() << endl;
    return true;
}


bool Labels::dropped(const Point& xy, shared_ptr<Object>const & obj){
    auto op = dynamic_pointer_cast<Operator>(obj);
    if(!op){
        cout << "Object is not an operator." << endl;
        return false; 
    }
    auto module = op->getModule();
    addObject(module);
    setLocation(Point(loc.x, loc.y));
    cout << "Added a label to Labels." << endl;
    return true;
}


bool Main::dropped(const Point& xy, shared_ptr<Object>const & obj){
    auto op = dynamic_pointer_cast<Operator>(obj);
    if(!op){
        cout << "Object is not an operator." << endl;
        return false;
    }
    addObject(obj);
    setLocation(Point(loc.x, loc.y));
    cout << "Added an Operator to Main." << endl;
    return true;
}
