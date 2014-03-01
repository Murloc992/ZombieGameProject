#include "precomp.h"
#include "isg_object.h"
#include "isg_render_queue.h"
#include "scenegraph.h"

namespace sg
{

isg_object::isg_object(scenegraph * sg):m_scenegraph(sg), m_scale(1,1,1), m_rotation(1,0,0,0), m_parent(0), m_flags(SGOF_TRANSFORM_OUTDATED)
{
}

isg_object::~isg_object()
{
    //dtor
}

const glm::mat4x4 & isg_object::get_absolute_transform()
{
    if(m_flags&SGOF_TRANSFORM_OUTDATED)
    {
        update_absolute_transform();
    }

    return m_absolute_transform;
}

std::string & isg_object::get_name()
{
    return m_name;
}

const sg_aabb & isg_object::get_aabb()
{
    return m_aabb;
}

const glm::vec3 &  isg_object::get_position() const
{
    return m_position;
}

const glm::quat &  isg_object::get_rotation() const
{
    return m_rotation;
}

const glm::vec3 &  isg_object::get_scale() const
{
    return m_scale;
}

void isg_object::set_position(const glm::vec3 & v)
{
    m_position = v;
    m_flags |= SGOF_TRANSFORM_OUTDATED;
}

void isg_object::set_rotation(const glm::quat & v)
{
    m_rotation = v;
    m_flags |= SGOF_TRANSFORM_OUTDATED;
}

void isg_object::set_scale(const glm::vec3 & v)
{
    m_scale = v;
    m_flags |= SGOF_TRANSFORM_OUTDATED;
}

glm::mat4x4 isg_object::get_relative_transform()
{
    glm::mat4x4 rt = glm::translate(glm::mat4(),m_position) * glm::toMat4(m_rotation);
    return rt;
}

void isg_object::update_absolute_transform()
{
    if(m_parent)
        m_absolute_transform = get_relative_transform() * m_parent->get_absolute_transform();
    else
        m_absolute_transform = get_relative_transform();

    rmbit(m_flags,SGOF_TRANSFORM_OUTDATED);
}

void isg_object::update(float delta_time)
{
    this->update_absolute_transform();

    for(sg_object_ptr child: m_children)
    {
        child->update(delta_time);
    }
}

bool isg_object::register_for_rendering()
{
    sg_render_queue_ptr rq = this->m_scenegraph->get_render_queue();
    rq->add_object(this);

    for(sg_object_ptr obj: m_children)
        rq->add_object(obj.get());

    return true;
}

void isg_object::add_child(sg_object_ptr child)
{
    if(child->get_parent())
    {
        child->get_parent()->remove_child(child);
    }

    m_children.push_back(child);
    child->set_parent(this);
    child->update_absolute_transform();
}

void isg_object::remove_child(sg_object_ptr child)
{
    auto it = std::find(m_children.begin(), m_children.end(), child);

    if(it!=m_children.end())
        m_children.erase(it);
}

isg_object * isg_object::get_parent()
{
    return m_parent;
}

void isg_object::set_parent(isg_object * parent)
{
    m_parent = parent;
}

const std::vector<sg_object_ptr> & isg_object::get_children()
{
    return m_children;
}

}
