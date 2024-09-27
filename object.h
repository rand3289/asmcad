#pragma once
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <string>
#include <vector>
#include <memory>


// load SDL texture from a png image
// this class has to be initialized by calling ImageLoader::setRenderer()
class ImageLoader {
    static SDL_Renderer* renderer;
public:
    static void setRenderer(SDL_Renderer* rendereR){ renderer = rendereR; }
    static SDL_Texture* getImage(const std::string& filename);
};


struct Point {
    int x;
    int y;
    Point(): x(0), y(0) {}
    Point(int X, int Y): x(X), y(Y) {}
    Point& operator+(const Point& rhs);
    bool inRectangle(Point ul, Point lr);
};


struct Object: public std::enable_shared_from_this<Object>{
    SDL_Rect loc; // location of the image
    SDL_Texture* img; // this is an opaque pointer and can not be wrapped in shared_ptr. Own it
    Object(): img(nullptr) {}
    Object(const std::string imgFileName){ img = ImageLoader::getImage(imgFileName); }
    virtual ~Object(){ if(img){ SDL_DestroyTexture(img); } }

    // The way children are removed is by dragging them out but sometimes they also have to be deleted from other objects
    virtual bool removeChild(std::shared_ptr<Object> obj){ return true; };
    virtual bool saveScad(std::ofstream& file)=0; // save self and children into an openscad file
    virtual void setLayout(const Point& xy){}
    virtual void setImage(SDL_Texture* sdlTexture){ img = sdlTexture; }
    virtual void draw(SDL_Renderer* rend){ SDL_RenderCopy(rend, img, NULL, &loc); }
//    virtual std::shared_ptr<Object> clone()=0;

    virtual void click (const Point& xy){} // mouse click
    virtual void clickr(const Point& xy){} // right click
    virtual void scroll(const Point& xy, int y){} // mouse wheel scrolls in vertical direction

    virtual std::shared_ptr<Object> takeObject(const Point& xy){ return shared_from_this(); } // mouse started dragging within this object
    virtual void drag(const Point& xy){} // another object is dragged accross this one
    virtual void dragEnd(){}
    virtual void dropped(const Point& xy, std::shared_ptr<Object>& obj){} // another object was dropped on top of this one
};


// Vertical Layout will be used in the main frame as "lines" to contain Operator and in the MODULE list
// fixed horizontal & vertical size. Changes only when main window is resized.
// scrolls on up()/down() events
class VerticalLayout: public Object {
    std::vector<Object> children;
public:
    virtual bool removeChild(std::shared_ptr<Object&> obj);
    virtual bool saveScad(std::ofstream& file);
};

// Flow layout will be used in top menu and to contain operators' children within "lines"
// no scrolling. left-to-right layout
class FlowLayout: public Object {
    std::vector<Object> children;
public:
    virtual bool removeChild(std::shared_ptr<Object&> obj);
    virtual bool saveScad(std::ofstream& file);
};

// When Operator is dropped into the "module list" it should call saveScad() on self, 
// then generate an image from saved text and then call module->setImage()
class Operator: public Object {
    std::shared_ptr<Object> module; // openscad module
    FlowLayout layout;
    enum OperatorType {UNION, DIFFERENCE, INTERSECTION} type;
public:
    virtual bool saveScad(std::ofstream& file);
};

// translate/rotate
class Modifier: public Object{
    enum ModifierType {TRANSLATE,ROTATE} type;
public:
    virtual bool saveScad(std::ofstream& file);
};

// This is a component in the upper left corner
// When an Operator is dropped here, it is deleted if it is unused in the rest of the "code"
// When a Module is dropped here, module representation (name) is deleted from an Operator and Module list
class DropZoneDelete: public Object { // does not have children
public:
    virtual bool saveScad(std::ofstream& file);
};

// This is a component in the upper right corner
// When a module (operator) is dragged into it, it generates and saves OpenScad code into OUTPUT_FILE_SCAD
// Code in OUTPUT_FILE_SCAD should be opened in OpenScad for real-time display
class DropZoneView: public Object { // does not have children
public:
    virtual bool saveScad(std::ofstream& file);
};

// Floating point numeric input box from which Shape and translate/rotate take their parameters
// Changes via scroll wheel up/down.  
// Change increment depends on whether Input was selected via click or right-click
class Input: public Object {  // does not have children
    double value;
public:
    virtual bool saveScad(std::ofstream& file);
};

// An actual shape such as OpenScad's cube, cylinder and sphere
class Shape: public Object { // does not have children
    enum ShapeType {CUBE, CYLINDER, SPHERE} type;
public:
    virtual bool saveScad(std::ofstream& file);
};

// Graphical representation (picture) of an Operator and all its children
// This is an equivalent of OpenScad's module
class Module: public Object { // does not have children
public:
    virtual bool saveScad(std::ofstream& file);
};
