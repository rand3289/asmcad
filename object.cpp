#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <algorithm>
#include <iostream>
#include "misc.h"
#include "object.h"
using namespace std;


int FlowLayout::findChildIndex(std::shared_ptr<Object>& obj){
    auto it = find(begin(children), end(children), obj);
    return end(children) == it ? -1 : distance(begin(children), it);
}

void FlowLayout::add(std::shared_ptr<Object>const & obj){
    children.push_back(obj);
}
