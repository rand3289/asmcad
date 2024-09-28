#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <algorithm>
#include <iostream>
#include "misc.h"
#include "object.h"
using namespace std;


VerticalLayout::VerticalLayout(int width): FlowLayout(width) {}
void VerticalLayout::scroll(const Point& xy, int y){}
void VerticalLayout::drag(const Point& xy){}
void VerticalLayout::dragEnd(){}
void VerticalLayout::dropped(const Point& xy, std::shared_ptr<Object>const & obj){}

void VerticalLayout::setLocation(const Point& xy){
    Point current = xy;
    for(auto objPtr: children){
        objPtr->setLocation(current);
        current.y+= objPtr->loc.h;
    }
    loc.h = current.y - loc.y;
}


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
