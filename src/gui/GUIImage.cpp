#include "Precomp.h"

#include "opengl/Texture.h"

#include "GUIEnvironment.h"

#include "GUIImage.h"

gui_image::gui_image(gui_environment* env, rect2d<int> dimensions, std::shared_ptr<texture> tex):gui_element(env,dimensions)
{
    this->type=GUIET_element;
    environment=env;

    absolute_rect=dimensions;
    relative_rect=absolute_rect;

    m_tex=tex;

    //it's an image...
    this->set_listening(false);

    this->set_parent(env);
}

gui_image::~gui_image()
{
}

void gui_image::render()
{
    environment->draw_gui_quad(absolute_rect,m_tex,true);

    this->render_children();
}
