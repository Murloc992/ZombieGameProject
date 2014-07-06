#pragma once
#include "GUIEventListener.h"
#include "utility/Rect2d.h"
enum gui_element_type
{
    GUIET_element,

    GUIET_button,
    GUIET_checkbox,
    GUIET_edit_box,
    GUIET_image,
    GUIET_pane,
    GUIET_slider,
    GUIET_static_text,
    GUIET_window,

    GUIET_count
};
class gui_environment;
class gui_element:public gui_event_listener
{
public:
    gui_element(gui_environment* env,rect2d<int> dimensions);

    ~gui_element();

    gui_element_type get_element_type(){return this->type;}

    void destroy_children();

    void set_id(uint32_t);

    uint32_t get_id();

    //each element handles it's own rendering
    virtual void render();

    void render_children();

    void add_child(gui_element *e);

    void remove_child(gui_element *e);

    void bring_to_front(gui_element *e);

    void update_absolute_pos();

    void set_name(std::string name);

    void set_event_listener(gui_event_listener *listener);

    gui_event_listener * get_event_listener();

    virtual bool on_event(const gui_event & e);

    void set_enabled(bool b);

    void set_focused(bool b);

    void set_visible(bool b);

    //!Purpose: Debug
    void set_hovered(bool b);

    void set_listening(bool b);

    bool is_enabled();

    bool is_focused();

    bool is_visible();

    bool is_hovered();

    bool accepts_events();

    gui_environment *get_environment();

    void set_parent(gui_element *e);

    gui_element *get_parent();

    std::vector<gui_element *> & get_children();

    std::string get_name();

    gui_element *get_element_from_point(int x, int y);

    rect2d<int> &get_absolute_rect();
    rect2d<int> &get_relative_rect();
private:
protected:
    gui_element_type type;
    uint32_t id;
    int disp_w, disp_h;
    std::string name;
    gui_element *parent;
    std::vector<gui_element*> children;
    gui_environment *environment;
    gui_event_listener *event_listener;
    rect2d<int> absolute_rect, relative_rect;
    bool visible, enabled, accept_events, focused, hovered;
};