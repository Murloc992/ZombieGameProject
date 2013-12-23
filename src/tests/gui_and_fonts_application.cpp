#include "gui_and_fonts_application.h"

#include "application/window.h"
#include "resources/resource_cache.h"
#include "opengl/texture.h"
#include "opengl/frame_buffer_object.h"
#include "utility/timer.h"
#include "utility/logger.h"
#include "opengl/quad.h"
#include "gui/gui_environment.h"
#include "opengl/opengl_util.h"
#include "gui/gui_element.h"
#include "gui/gui_test_element.h"

gui_and_fonts_application::gui_and_fonts_application(uint32_t argc, const char ** argv): application(argc,argv)
{
    //definitely a comment
}

gui_and_fonts_application::~gui_and_fonts_application()
{

}

bool gui_and_fonts_application::init(const std::string & title, uint32_t width, uint32_t height)
{
    application::init(title,width,height);
    shader_cache = create_resource_cache<shader>();
    tex_cache = create_resource_cache<texture>();
    fbo_cache = create_resource_cache<frame_buffer_object>();

    frame_count = 0;
    last_time = 0;

    ///quad shaders
    char * vsh=NULL;
    char * fsh=NULL;

    binding qtex_binding[]={{"tex",0},{"",-1}};

    if(!read("res/quad.vert",vsh)) return false;
    if(!read("res/quad.frag",fsh)) return false;

    qsh = new shader("quad_shader",vsh,fsh,qtex_binding,0);
	qsh->compile();
	qsh->link();
	shader_cache->push_back(share(qsh));

	delete [] vsh;
	delete [] fsh;

	///prepare fbo, textures
	tex = new texture();
	ztex = new texture();
	auto shared_tex = share(tex);
	auto shared_ztex = share(ztex);
    tex->generate(NULL,GL_TEXTURE_2D,GL_RGBA,GL_RGBA,1024,1024);
    ztex->generate(NULL,GL_TEXTURE_2D,GL_DEPTH_COMPONENT,GL_DEPTH_COMPONENT24,1024,1024,0);
    tex_cache->push_back(shared_tex);
    tex_cache->push_back(shared_ztex);
    this->gl_util->check_and_output_errors();

    fbo = new frame_buffer_object();
    fbo->generate();
    fbo->enable(GL_COLOR_ATTACHMENT0);
    fbo->set(GL_FRAMEBUFFER);

    fbo->attach(GL_COLOR_ATTACHMENT0,shared_tex);
    uint32_t complete = fbo->is_complete();

    if(complete!=GL_FRAMEBUFFER_COMPLETE)
        m_log->log(LOG_ERROR,"GL_FBO_ERROR: %s",gl_util->gl_fbo_error_to_string(complete).c_str());

    fbo->attach(GL_DEPTH_ATTACHMENT,shared_ztex);
    complete = fbo->is_complete();

    if(complete!=GL_FRAMEBUFFER_COMPLETE)
        m_log->log(LOG_ERROR,"GL_FBO_ERROR: %s",gl_util->gl_fbo_error_to_string(complete).c_str());

    fbo_cache->push_back(share(fbo));

    q = new quad();
    q->generate();

    ///set up matrices

    M = glm::mat4(1.0f);
    M = glm::translate(M,glm::vec3(-10,5,-10));
    M = glm::scale(M,glm::vec3(0.01,0.01,0.01));

    V = glm::lookAt(glm::vec3(0,5,20),glm::vec3(0,5,0),glm::vec3(0,1,0));
    V = glm::rotate<float>(V,-90,glm::vec3(0,1,0));

    P = glm::perspective(45.f,4.f/3.f,1.0f,2048.f);

    MVP=P*V*M;

    ///gl setup
    glClearColor(0.0f, 0.0f, 0.568f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    fbo->unset();

    int ww,hh;
    GLFWwindow * _window=wnd->getWindow();
    glfwGetWindowSize(_window,&ww,&hh);

    env=new gui_environment(ww,hh,_window);
    env->set_event_listener(this);
    gui_test_element *elem=new gui_test_element(env,100,0,100,100);
    elem->set_event_listener(this);

    gui_test_element *elem2=new gui_test_element(env,0,0,50,50);
    elem2->set_parent(elem);
    elem2->set_event_listener(this);
    return true;
}

void gui_and_fonts_application::on_event(gui_event e)
{
    logger* _log=this->get_logger();
    switch(e.get_type())
    {
    case element_focused:
        _log->log(LOG_DEBUG,"Element %s got focused.",e.get_caller()->get_name().c_str());
        break;
    case element_focus_lost:
        _log->log(LOG_DEBUG,"Element %s lost focus.",e.get_caller()->get_name().c_str());
        break;
    case element_hovered:
        _log->log(LOG_DEBUG,"Element %s got hovered.",e.get_caller()->get_name().c_str());
        break;
    case element_exitted:
        _log->log(LOG_DEBUG,"Element %s got exitted.",e.get_caller()->get_name().c_str());
        break;
    case mouse_pressed:
        _log->log(LOG_DEBUG,"Mouse pressed on %s.",e.get_caller()->get_name().c_str());
        break;
    case mouse_released:
        _log->log(LOG_DEBUG,"Mouse released on %s.",e.get_caller()->get_name().c_str());
        break;
    case mouse_dragged:
        _log->log(LOG_DEBUG,"Mouse dragged on %s.",e.get_caller()->get_name().c_str());
        break;
    default:
        break;
    }
}

bool gui_and_fonts_application::update()
{
    if(wnd->update() && !wnd->get_key(GLFW_KEY_ESCAPE))
    {
        // Measure speed
        main_timer->tick();

        //env->update(0);

        show_fps();

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        qsh->set();
        tex->set(0);
        q->draw();

        wnd->swap_buffers();

        ///let's just rage quit on gl error
        return !this->gl_util->check_and_output_errors();
    }
    return false;
}

void gui_and_fonts_application::show_fps()
{
    uint32_t currentTime = main_timer->get_time();
    frame_count++;
    if ( currentTime - last_time >= 1000 )  // If last prinf() was more than 1 sec ago
    {
        m_log->log(LOG_LOG,"FPS: %i (%f ms/frame)",frame_count,1000.0/double(frame_count));

        frame_count = 0;
        last_time = currentTime;
    }
}

void gui_and_fonts_application::exit()
{
    delete q;
    delete fbo_cache;
    delete tex_cache;
    delete shader_cache;

    application::exit();
}
