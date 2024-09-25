#pragma once
#include <string>
#include <vector>
#include <memory>


struct Point{
    int x;
    int y;
    Point(int X, int Y): x(X),y(Y){}
    Point& operator+(const Point& rhs);
    bool inRectangle(Point ul, Point lr);
};

//struct Image {
//    void draw(const Point& xy);
//};

//enum ObjType{DROPZONE, LAYOUT, OPERATOR, SHAPE, MODULE};

//static int getUniqueId();
//static int getUniqueId(){ // TODO: move to .cpp
//    static int lastId = 0;;
//    return ++lastId;
//}


struct Object {
//    int id;
//    ObjType type;
//    std::shared_ptr<Image> img;
// Operators might have just 2 children: "self image" and "layout".
//    std::vector<std::shared_ptr<Object>> children;
//    Point loc; // location x,y.  Location has to be passed in.
//    Point dim; // width, height.  Object determines its own dimentions during layout.
    std::shared_ptr<SDL_Texture> img;
    SDL_Rect loc; // location of the image

    // The way children are removed is by dragging them out but sometimes they also have to be deleted from other objects
    virtual bool removeChild(std::shared_ptr<Object&> obj);
    virtual void saveScad(std::ofstream& file); // save self and children into an openscad file
    virtual void setLayout(Point xy);
    virtual void setImage(std::shared_ptr<SDL_Texture>& sdlTexture){ img = sdlTexture; }
    virtual void draw(SDL_Renderer* rend){ SDL_RenderCopy(rend, img, NULL, &loc); }
    virtual std::shared_ptr<Object>& clone();

    virtual void click(const Point& xy); // mouse click
    virtual void clickr(const Point& xy); // right click
    virtual void scroll(int y); // mouse wheel scrolls in vertical direction

    virtual std::shared_ptr<Object> takeObject(const Point& xy); // mouse started dragging within this object
    virtual void drag(const Point& xy); // another object is dragged accross this one
    virtual void dragEnd();
    virtual void dropped(const Point& xy, std::shared_ptr<Object>& obj); // another object was dropped on top of this one
};

// Should "delete drop zone" be a root?
// Root object (DROPZONE) will have a speciall object called "Dragged"
// Operator will have a special object 'image of itself"

// Need 2 layouts Vertical and Flow.  Vertical will be used in the main frame as "lines" and in the MODULE list
// Flow layout will be used in top menu and to contain operators within "lines"
// Vertical layout needs to scroll
// Use shared_ptr value converted to int as a unique ID when saving code?

// fixed horizontal & vertical size
// scrolls on up()/down() events
class VerticalLayout: public Object {
    std::vector<Object> children;
};
// no scrolling left-to-right layout
class FlowLayout: public Object {
    std::vector<Object> children;
}

// Operator's takeObject() and dropped() call saveScad() on self, 
// then generate an image from saved text and then call module->setImage()

class Operator: public Object {
    std::shared_ptr<Module> module;
    HorisontalLayout layout;
};
class DropZone: public Object { // does not have children

};
class Input: public Object {  // does not have children // simple input box
    double value;

}
class Shape: public Object { // does not have children

};
class Module: public Object { // does not have children

};