#include "precomp.h"

#include "gui/gui_element.h"

gui_element::gui_element(gui_environment* env,rect2d<int> dimensions)
{
    this->type=GUIET_element;
    absolute_rect = dimensions;
    relative_rect = dimensions;

    id=-1;

    hovered = false;
    visible = true;
    focused = false;
    enabled = true;
    accept_events = true;

    this->parent = nullptr;
    this->event_listener = nullptr;
    this->environment = env;
    set_name("gui_element_"+helpers::to_str(rand()%65535));
}

gui_element::~gui_element()
{
    destroy_children();
}

void gui_element::set_id(uint32_t id)
{
    this->id=id;
}

uint32_t gui_element::get_id()
{
    return this->id;
}

void gui_element::destroy_children()
{
    for(gui_element* el:children)
    {
        el->set_listening(false);
    }

    children.clear();
}

void gui_element::render()
{
    //will be overriden by everything
}

void gui_element::add_child(gui_element *e)
{
    std::vector<gui_element*>::iterator i=std::find(children.begin(),children.end(),e);
    if(i!=children.end())
        return;

    e->parent=this;
    children.push_back(e);
    printf("Definitely added a child. I has %d\n",children.size());
    e->relative_rect=e->absolute_rect;

    update_absolute_pos();
}

void gui_element::remove_child(gui_element *e)
{
    std::vector<gui_element*>::iterator i=std::find(children.begin(),children.end(),e);
    if(i!=children.end())
    {
        e->parent=nullptr;
        i=children.erase(i);
        (*i)->relative_rect=(*i)->absolute_rect;
        return;
    }
}

void gui_element::bring_to_front(gui_element *e)
{
    std::vector<gui_element*>::iterator i=std::find(children.begin(),children.end(),e);
    if(i!=children.end())
    {
        i=children.erase(i);
        children.push_back(e);
        return;
    }
}

void gui_element::render_children()
{
    for(gui_element* i : children)
    {
        if(i->is_visible())
            i->render();
    }
}

void gui_element::update_absolute_pos()
{
    if(this->parent!=nullptr)
        this->absolute_rect=
            rect2d<int>(parent->absolute_rect.x+relative_rect.x,
                        parent->absolute_rect.y+relative_rect.y,
                        this->absolute_rect.w, this->absolute_rect.h);
    for(gui_element *e : children)
        e->update_absolute_pos();
}

void gui_element::set_name(std::string name)
{
    this->name=name;
}

void gui_element::set_event_listener(gui_event_listener *listener)
{
    this->event_listener=listener;
}

gui_event_listener * gui_element::get_event_listener()
{
    return this->event_listener;
}

bool gui_element::on_event(const gui_event & e)
{
    if(this->event_listener && this->event_listener->on_event(e))
        return true;

    return parent ? parent->on_event(e) : false;
}

void gui_element::set_enabled(bool b)
{
    this->enabled=b;
    if(children.size()>0)
        for(gui_element *e : children)
            e->set_enabled(true);
}

void gui_element::set_focused(bool b)
{
    this->focused=b;
}

void gui_element::set_visible(bool b)
{
    this->visible=b;
    for(gui_element* e:children)
        e->set_visible(b);
}

void gui_element::set_hovered(bool b)
{
    this->hovered=b;
}

void gui_element::set_listening(bool b)
{
    this->accept_events=b;
}

void gui_element::set_parent(gui_element *e)
{
    if(e!=nullptr)
    {
        if(this->parent!=nullptr)
            this->parent->remove_child(this);
        e->add_child(this);
    }
    else
        return;
}

bool gui_element::is_enabled()
{
    return this->enabled;
}

bool gui_element::is_focused()
{
    return this->focused;
}

bool gui_element::is_visible()
{
    return this->visible;
}

bool gui_element::is_hovered()
{
    return this->hovered;
}

bool gui_element::accepts_events()
{
    return this->accept_events;
}

gui_environment *gui_element::get_environment()
{
    return this->environment;
}

gui_element *gui_element::get_parent()
{
    return this->parent;
}

std::vector<gui_element *> & gui_element::get_children()
{
    return this->children;
}

std::string gui_element::get_name()
{
    return this->name;
}

gui_element *gui_element::get_element_from_point(int x, int y)
{
    gui_element *ret=nullptr;
    std::vector<gui_element*>::reverse_iterator i=children.rbegin();
    for(; i!=children.rend(); i++)
    {
        if(*i!=nullptr)
        {
            ret=(*i)->get_element_from_point(x,y);
            if(ret!=nullptr&&ret->accepts_events())
                return ret;
        }
    }
    if(this->is_visible()&&absolute_rect.is_point_inside(x,y))
        ret=this;
    return ret;
}

rect2d<int> &gui_element::get_absolute_rect()
{
    return absolute_rect;
}

rect2d<int> &gui_element::get_relative_rect()
{
    return relative_rect;
}
