#include "Precomp.h"
#include "Scenegraph.h"
#include "SGGraphicsManager.h"
#include "SGDefaultRenderQueue.h"
#include "opengl/Shader.h"
#include "opengl/Mesh.h"
#include "utility/Logger.h"
#include "SGSkyboxObject.h"

namespace sg
{

scenegraph::scenegraph(logger * l, timer_ptr app_timer): m_render_queue(nullptr)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glClearColor(0.0f, 0.0f, 0.568f, 1.0f);

    m_logger = l;
    m_graphics_manager = share(new sg_graphics_manager(l));
    m_timer = app_timer;
    m_render_queue = share(new sg_default_render_queue(this));
}

scenegraph::~scenegraph()
{
    //dtor
}

void scenegraph::clear()
{
    m_objects.clear();
    m_triggers.clear();
}

sg_object_ptr scenegraph::add_object(sg_object_ptr object)
{
    if(object->get_type()==SGO_CAMERA && !m_active_camera)
        m_active_camera = std::static_pointer_cast<sg_camera_object>(object);

    m_objects.push_back(object);
    return object;
}

sg_light_object_ptr scenegraph::add_light_object()
{
    sg_light_object_ptr obj = share(new sg_light_object(this));
    add_object(obj);
    return obj;
}

logger * scenegraph::GetLogger()
{
    return m_logger;
}

void scenegraph::pre_render()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if(m_active_camera)
    {
        m_active_camera->update(this);
        sg_shared_mat_vars & v = m_shared_mat_vars;
        v.view = m_active_camera->get_absolute_transform();
        v.proj = m_active_camera->get_projection();
        v.view_proj = v.proj.value * v.view.value;
    }
    else

        throw "At least one camera is required";
}

bool scenegraph::validate_transforms()
{
    bool success = true;

    for(sg_object_ptr o: m_objects)
    {
        isg_object * obj = o.get();

        glm::vec3 pos = obj->get_position();
        glm::quat rot = obj->get_rotation();
        //glm::vec3 scale = obj->get_scale();
        glm::mat4 transf = obj->get_absolute_transform();

        glm::vec3 pos2(transf[3].x,transf[3].y,transf[3].z);
        transf[3]=glm::vec4(0,0,0,0);

        glm::quat rot2 = glm::toQuat(transf);

        if(!helpers::equals(pos,pos2))
        {
            m_logger->log(LOG_WARN,"{%s}(pos)[%f,%f,%f]!=[%f,%f,%f]",obj->get_name().c_str(),pos.x,pos.y,pos.z,pos2.x,pos2.y,pos2.z);
            success = false;
        }

        if(!helpers::equals(rot,rot2) && !helpers::equals(rot,-1.0f*rot2))
        {
            m_logger->log(LOG_WARN,"{%s}(rot)[%f,%f,%f,%f]!=[%f,%f,%f,%f]",obj->get_name().c_str(),rot.x,rot.y,rot.z,rot.w,rot2.x,rot2.y,rot2.z,rot2.w);
            success = false;
        }

    }

    return success;
}

bool scenegraph::register_objects_for_rendering()
{
    sg_render_queue_ptr rq = this->get_render_queue();

    for(sg_object_ptr obj: m_objects)
        rq->add_object(obj.get());

    return true;
}


void scenegraph::render_all()
{
    pre_render();

    register_objects_for_rendering();
    m_render_queue->render_all();
    m_render_queue->clear();

    post_render();
}

void scenegraph::update_all(float delta_time)
{
    for(sg_object_ptr obj: m_objects)
        obj->update(delta_time);
}

sg_object_ptr scenegraph::object_depth_pick(int32_t x, int32_t y, int32_t w, int32_t h)
{
    float winz=0.0f;
    glReadPixels( x, h-y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winz);
    glm::vec3 objPt = glm::unProject(glm::vec3(x,h-y,winz), this->get_active_camera()->get_absolute_transform(), this->get_active_camera()->get_projection(), glm::vec4(0,0,w, h));


    std::vector<sg_object_ptr> aabb_test_pass(m_objects);
    /*for(sg_object_ptr obj: this->m_objects)
    {
        if(obj->get_aabb().is_point_inside(objPt))
            aabb_test_pass.push_back(obj);
    }*/

    std::sort(aabb_test_pass.begin(), aabb_test_pass.end(), [&objPt] (sg_object_ptr o1, sg_object_ptr o2)
              {
                  return glm::distance(o1->get_position(),objPt) < glm::distance(o2->get_position(),objPt);
              });


    if(aabb_test_pass.size()&&glm::distance(aabb_test_pass[0]->get_position(),objPt)<2.0f)
        return aabb_test_pass[0];
    else
        return sg_object_ptr();
}

glm::vec3 scenegraph::window_coords_to_world(float depth, int32_t x, int32_t y, int32_t w, int32_t h)
{
    glm::vec4 viewport = glm::vec4(0, 0, w, h);
    glm::vec3 wincoord = glm::vec3(x, h - y - 1, depth);
    glm::vec3 objcoord = glm::unProject(wincoord, this->get_active_camera()->get_absolute_transform(), this->get_active_camera()->get_projection(), viewport);

    return objcoord;
}

sg_graphics_manager_ptr scenegraph::get_graphics_manager()
{
    return m_graphics_manager;
}

sg_render_queue_ptr scenegraph::get_render_queue()
{
    return m_render_queue;
}

sg_camera_object_ptr scenegraph::get_active_camera()
{
    return m_active_camera;
}

void scenegraph::set_active_camera(sg_camera_object_ptr cam)
{
    m_active_camera = cam;
}

void scenegraph::post_render()
{

}


/// LOADING
sg::sg_mesh_object_ptr scenegraph::load_mesh_object(std::string file, bool load_textures)
{
    sg::sg_mesh_object_ptr ret;
    mesh_ptr pmesh;

    pmesh = m_graphics_manager->get_mesh_loader()->load(file);

    if(pmesh)
    {
        ret = share(new sg::sg_mesh_object(this,pmesh));

        std::string texture_path = file.substr(0,file.rfind("/"));
        m_logger->log(LOG_LOG, "Texture path =%s", texture_path.c_str());

        for(uint32_t i=0; i<ret->get_material_count(); i++)
        {
            std::string mat_name, image_path;
            uint32_t pos = pmesh->sub_meshes[i].material_name.find("|");
            mat_name = pmesh->sub_meshes[i].material_name.substr(0,pos);

            if(pos+1<pmesh->sub_meshes[i].material_name.length())
                image_path = pmesh->sub_meshes[i].material_name.substr(pos+1);

            m_logger->log(LOG_LOG, "full mat_name =%s", pmesh->sub_meshes[i].material_name.c_str());
            m_logger->log(LOG_LOG, "mat_name =%s", mat_name.c_str());
            m_logger->log(LOG_LOG, "image_path =%s", image_path.c_str());

            if(load_textures)
            {
                sg::sg_material_ptr mat = ret->get_material(i);
                if(mat->mat_type == SGM_VSM_FINAL_PASS)
                {
                    sg_material_vsm_final_pass * sm_mat = static_cast<sg_material_vsm_final_pass*>(mat.get());

                    if(image_path.length()!=0)
                    {
                        auto tex = m_graphics_manager->load_texture(texture_path + "/" + image_path);

                        if(tex)
                            sm_mat->texture1 = tex;
                    }
                }
                else if(mat->mat_type == SGM_STATIC_MESH)
                {
                    sg_material_static_mesh * sm_mat = static_cast<sg_material_static_mesh*>(mat.get());

                    if(image_path.length()!=0)
                    {
                        auto tex = m_graphics_manager->load_texture(texture_path + "/" + image_path);

                        if(tex)
                            sm_mat->mat_texture = tex;
                    }
                }
            }
        }
    }

    return ret;
}

/// LOADING
sg::sg_skybox_object_ptr scenegraph::load_skybox(std::string file, bool load_textures)
{
    sg::sg_skybox_object_ptr ret;
    mesh_ptr pmesh;

    pmesh = m_graphics_manager->get_mesh_loader()->load(file);

    if(pmesh)
    {
        ret = share(new sg::sg_skybox_object(this,pmesh));

        std::string texture_path = file.substr(0,file.rfind("/"));
        m_logger->log(LOG_LOG, "Texture path =%s", texture_path.c_str());

        for(uint32_t i=0; i<ret->get_material_count(); i++)
        {
            std::string mat_name, image_path;
            uint32_t pos = pmesh->sub_meshes[i].material_name.find("|");
            mat_name = pmesh->sub_meshes[i].material_name.substr(0,pos);

            if(pos+1<pmesh->sub_meshes[i].material_name.length())
                image_path = pmesh->sub_meshes[i].material_name.substr(pos+1);

            m_logger->log(LOG_LOG, "full mat_name =%s", pmesh->sub_meshes[i].material_name.c_str());
            m_logger->log(LOG_LOG, "mat_name =%s", mat_name.c_str());
            m_logger->log(LOG_LOG, "image_path =%s", image_path.c_str());

            if(load_textures)
            {
                sg::sg_material_ptr mat = ret->get_material(i);
                if(mat->mat_type == SGM_ABSTRACT_MATERIAL)
                {
                    sg_abstract_material * am = static_cast<sg_abstract_material*>(mat.get());

                    if(image_path.length()!=0)
                    {
                        auto tex = m_graphics_manager->load_texture(texture_path + "/" + image_path);

                        if(tex)
                            am->set_texture(0,tex);
                    }
                }
            }
        }
    }

    return ret;
}

const sg_shared_mat_vars & scenegraph::get_shared_mat_vars() const
{
    return m_shared_mat_vars;
}

sg_material_ptr scenegraph::get_override_material()
{
    return m_override_material;
}

void scenegraph::set_override_material( sg_material_ptr material)
{
    m_override_material = material;
}

}
