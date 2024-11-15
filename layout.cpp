#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <algorithm>
#include <iostream>
#include "misc.h"
#include "object.h"
using namespace std;


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
//    cout << "HLayout setting location at ("<< xy.x << ","<<xy.y<<") size (" << loc.w << "," << loc.h << ")" << endl;
    cout << '_';
}

bool FlowLayout::saveScad(ostream& file){
    for(auto& o: children){
        o->saveScad(file);
    }
    return true;
}

void FlowLayout::addObject(shared_ptr<Object>const & obj){
    if( find(begin(children), end(children), obj) != end(children) ) { return; } // duplicate
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
//                if(*it == obj){ // it is the child itself
//                    children.erase(it);
//                }
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
    if( end(children) == find(begin(children), end(children), obj)) {
        addObject(obj);
        cout << "Added an object. children size=" << children.size() << endl;
    }
    setLocation(Point(loc.x,loc.y)); // perform layout
    return true;
}

std::shared_ptr<Object> FlowLayout::click(const Point& xy){
    for(auto& c: children){
        if(xy.inRectangle(c->loc)){
            auto o = c->click(xy);
            if(o){ return o; }
        }
    }
    // Flow Layout should never be in focus because it does not respond to mouse wheel events
    // TODO for vertical layout !!!
    return shared_ptr<Object>();
}

std::shared_ptr<Object> FlowLayout::clickr(const Point& xy){
    for(auto& c: children){
        if(xy.inRectangle(c->loc)){
            auto o = c->clickr(xy);
            if(o){ return o; }
        }
    }
    return shared_ptr<Object>();
}


/*************************************************************************/
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
//    cout << "VLayout setting location at ("<< xy.x << ","<<xy.y<<") size (" << loc.w << "," << loc.h << ")" << endl;
    cout << '|';
}


bool Labels::dropped(const Point& xy, shared_ptr<Object>const & obj){
    auto op = dynamic_pointer_cast<Operator>(obj);
    if(!op || !op->isClone){
        setLocation(Point(loc.x, loc.y));
        cout << "Object is not an operator." << endl;
        return false; 
    }
    auto module = op->getModule();
    if( end(children) == find(begin(children), end(children), module)) {
        addObject(module);
        cout << "Added a label to Labels at "<< loc.x << "," << loc.y << endl;
    }
    setLocation(Point(loc.x, loc.y));
    return true;
}


shared_ptr<Object> Labels::takeObject(const Point& xy){
    for(auto& c: children){
        if(xy.inRectangle(c->loc)){
            auto obj = c;
            children.erase( remove(begin(children), end(children), c), end(children) );
            return obj;
        }
    }
    return shared_ptr<Object>();
}


bool Main::dropped(const Point& xy, shared_ptr<Object>const & obj){
    for(auto& o: children){
        if(xy.inRectangle(o->loc) && o->dropped(xy,obj) ){
            setLocation(Point(loc.x, loc.y));
            return true;
        }
    }
    auto op = dynamic_pointer_cast<Operator>(obj);
    if(!op){
        cout << "Object is not an operator." << endl;
        return false;
    }
    if( end(children) == find(begin(children), end(children), obj)) {
        addObject(obj);
        cout << "Added an Operator to Main." << endl;
    }
    setLocation(Point(loc.x, loc.y));
    return true;
}
