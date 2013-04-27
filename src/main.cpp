#include "precomp.h"
#include "tgaloader.h"
#include "texture.h"
#include "shader.h"
#include "iqmesh.h"
#include "iqmloader.h"

/**
static const char* vs = R"(
#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;
uniform mat4 MVP;
out vec2 UV;
void main()
{
    UV=tex_coords;
    gl_Position = MVP*vec4(position.x, position.y, position.z, 1.0);
})";
**/

static const char* gvs =
"#version 330\n"
"uniform mat4 MVP;\n"
"uniform mat4 bonemats[80];\n"
"layout (location=0) in vec3 pos;\n"
"layout (location=1) in vec2 tex;\n"
"layout (location=2) in vec3 normal;\n"
"layout (location=3) in vec4 vtangent;\n"
"layout (location=4) in vec4 vbones;\n"
"layout (location=5) in vec4 vweights;\n"
"out vec2 UV;\n"
"void main(void)\n"
"{\n"
"   mat4 m = bonemats[int(vbones.x)] * vweights.x;\n"
"   m += bonemats[int(vbones.y)] * vweights.y;\n"
"   m += bonemats[int(vbones.z)] * vweights.z;\n"
"   m += bonemats[int(vbones.w)] * vweights.w;\n"
"   vec4 mpos= vec4(pos,0);\n"
"   gl_Position = MVP * mpos;\n"
"   //mat3 madjtrans = mat3(cross(m[1].xyz, m[2].xyz), cross(m[2].xyz, m[0].xyz), cross(m[0].xyz, m[1].xyz));\n"
"   //vec3 mnormal = normal * madjtrans;\n"
"   //vec3 mtangent = vtangent.xyz * madjtrans; // tangent not used, just here as an example\n"
"   //vec3 mbitangent = cross(mnormal, mtangent) * vtangent.w; // bitangent not used, just here as an example\n"
"}\n";

static const char* fs = R"(
#version 330
uniform sampler2D tex;
in vec2 UV;
out vec4 FragColor;

void main()
{
    FragColor = texture2D(tex,UV);
})";

static bool is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

texture * load_tex(std::string file)
{
    std::ifstream rfile(file);

    if(!rfile.is_open())
    {
        printf("Could not open file\n");
        return NULL;
    }

    rfile.seekg(0, std::ios::end);
    uint32_t size = rfile.tellg();
    rfile.seekg(0);


    tgaloader l;
    char * buf = new char[size];
    rfile.read(buf,size);
    return l.generate(buf,size);
}

void glerr()
{
    uint32_t err = GL_NO_ERROR;

    while((err=glGetError())!=GL_NO_ERROR)
    {
        std::cout<<"GL_ERROR: " << err << std::endl;
    }
}

int main()
{
    printf("sizeof(iqmanim)=%lu\n", sizeof(iqmanim));
    printf("Is this bitch big endian? (%s)\n",is_big_endian()==true?"true":"false");

    if (!glfwInit())
    {
        printf("glfw init failed!\n");
        return -1;
    }

    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4); // 4x antialiasing
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    /* Create a windowed mode window and its OpenGL context */
    if (!glfwOpenWindow(800, 600, 8, 8, 8, 0, 24, 0, GLFW_WINDOW))
    {
        printf("glfw open window failed!\n");
        return -1;
    }

    if(glxwInit()!=0)
    {
        printf("GLXW failed to initialize.\n");
        return -1;
    }

    printf("OpenGL context version: %s\n",glGetString(GL_VERSION));

    /* Development going wild. or is it? */
    glClearColor(0.2f,0.2f,0.4f,1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    ///-------------------------------------
    FILE* io=fopen("../../ZombieGameProject/res/mrfixit.iqm","rb");

    //get the size
    fseek(io,0,SEEK_END);
    int32_t fsize=ftell(io);
    fseek(io,SEEK_SET,0);

    printf("Filesize ftell:%i\n",fsize);

    char* buffer=new char[fsize];
    fread(buffer,fsize,1,io);
    fclose(io);

    iqmloader *loader=new iqmloader();
    iqmesh * mesh=loader->load(buffer);
    loader->loadiqmanims(mesh);
    ///-------------------------------------



    glm::mat4 M=glm::mat4(1.0f);
    glm::mat4 V=glm::lookAt(glm::vec3(0,5,20),glm::vec3(0,5,0),glm::vec3(0,1,0));
    glm::mat4 P=glm::perspective(45.f,4.f/3.f,0.1f,1000.f);
    M=glm::translate(M,glm::vec3(-10,5,-10));
    M=glm::scale(M,glm::vec3(0.01,0.01,0.01));
    V = glm::rotate<float>(V,-90,glm::vec3(0,1,0));
    glm::mat4 MVP;

    binding tex_binding[]={{"",-1}};
    binding attrib_binding[]={{"position",0},{"tex_coords",1},{"",-1}};
    shader sh("default",gvs,fs,0,0);
	sh.compile();
	sh.link();
	sh.set();

    uint32_t mpl = sh.getparam("MVP");
    uint32_t bonemats = sh.getparam("bonemats");

    //texture * tex = load_tex("../../ZombieGameProject/res/Body.tga");
    //texture * tex2 = load_tex("../../ZombieGameProject/res/Head.tga");

    //mesh->submeshes[0].mat.texid=tex->obj;
    //mesh->submeshes[1].mat.texid=tex2->obj;

    //printf("Animations: %i %i\n",mesh->anims[0].first_frame,mesh->anims[0].num_frames);
    /* Loop until the user closes the window */

    double lastTime = glfwGetTime();
    int32_t nbFrames = 0;
    float fr=0.f,efr=101.f;
    while (glfwGetWindowParam(GLFW_OPENED)&&!glfwGetKey(GLFW_KEY_ESC))
    {
        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 )  // If last prinf() was more than 1 sec ago
        {
            // printf and reset timer
            printf("FPS: %i (%f ms/frame)\n",nbFrames,1000.0/double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        ///animate
        if(fr>=efr)
            fr=0;
        mesh->set_frame(fr);
        fr+=0.1;

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        MVP=P*V*M;
        glUniformMatrix4fv(sh.getparam("MVP"),1,GL_FALSE,glm::value_ptr(MVP));
        glUniformMatrix4fv(sh.getparam("bonemats"),mesh->data_header.num_joints,GL_FALSE,&mesh->current_frame[0][0].x);
        mesh->draw(false);

        //glerr();

        /* Swap front and back buffers and process events */
        glfwSwapBuffers();
    }
    delete mesh;
    delete loader;
    return 0;
}
