#include "precomp.h"

#include "iqmesh.h"
#include "math/util.h"
#include "resource_cache.h"
#include "utility/logger.h"

// TODO (serengeor#1#): Separate skeletal info from mesh


iqmesh::iqmesh(logger *logger)
{
    m_logger=logger;
    data_buff = NULL;
    frames = NULL;
	base_frame = NULL;
	inverse_base_frame = NULL;
	current_frame = NULL;
	texts = NULL;
	glmesh = share(new mesh());
	glmesh->buffers.resize(IQM_BUFFER_COUNT);
}

iqmesh::~iqmesh()
{
    free();
}

bool iqmesh::generate()
{
	//set the buffers to invalid values
	if(data_header.num_vertexarrays==0)
        return false;

	//create the VAO
	glGenVertexArrays(1,&glmesh->vao);
	glBindVertexArray(glmesh->vao);

	//keeping track on enabling attrib ids
	uint32_t attribid=0;
	//determine how many buffers we got and check their formats
	for(uint32_t i=0; i<data_header.num_vertexarrays; i++)
	{
		iqmvertexarray va=vertexarrays[i];
		switch(va.type)
		{
			case IQM_POSITION:
			m_logger->log(LOG_DEBUG,"Got a position buffer.");

			if(va.format!=IQM_FLOAT)
			{
				m_logger->log(LOG_DEBUG,"Bad format. Cannot continue.");
				return false;
			}

			glGenBuffers(1,&glmesh->buffers[IQM_POSITION]);
			glBindBuffer(GL_ARRAY_BUFFER,glmesh->buffers[IQM_POSITION]);
			glBufferData(GL_ARRAY_BUFFER,data_header.num_vertexes*sizeof(positions[0]),positions,GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribid);
			glVertexAttribPointer(attribid,3,GL_FLOAT,GL_FALSE,0,0);

			attribid++;
			break;

			case IQM_TEXCOORD:
			m_logger->log(LOG_DEBUG,"Got a texcoord buffer.");

			if(va.format!=IQM_FLOAT)
			{
				m_logger->log(LOG_DEBUG,"Bad format. Cannot continue.");
				return false;
			}

			glGenBuffers(1,&glmesh->buffers[IQM_TEXCOORD]);
			glBindBuffer(GL_ARRAY_BUFFER,glmesh->buffers[IQM_TEXCOORD]);
			glBufferData(GL_ARRAY_BUFFER, data_header.num_vertexes*sizeof(texcoords[0]),&texcoords[0],GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribid);
			glVertexAttribPointer(attribid,2,GL_FLOAT,GL_FALSE,0,0);
			attribid++;
			break;

			case IQM_NORMAL:
			m_logger->log(LOG_DEBUG,"Got a normal buffer.");
			if(va.format!=IQM_FLOAT)
			{
				m_logger->log(LOG_DEBUG,"Bad format. Cannot continue.");
				return false;
			}
			glGenBuffers(1,&glmesh->buffers[IQM_NORMAL]);
			glBindBuffer(GL_ARRAY_BUFFER,glmesh->buffers[IQM_NORMAL]);
			glBufferData(GL_ARRAY_BUFFER,data_header.num_vertexes*sizeof(normals[0]),&normals[0],GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribid);
			glVertexAttribPointer(attribid,3,GL_FLOAT,GL_FALSE,0,0);
			attribid++;
			break;

			case IQM_TANGENT:
			m_logger->log(LOG_DEBUG,"Got a tangent buffer.");
			if(va.format!=IQM_FLOAT)
			{
				m_logger->log(LOG_DEBUG,"Bad format. Cannot continue.");
				return false;
			}
			glGenBuffers(1,&glmesh->buffers[IQM_TANGENT]);
			glBindBuffer(GL_ARRAY_BUFFER,glmesh->buffers[IQM_TANGENT]);
			glBufferData(GL_ARRAY_BUFFER,data_header.num_vertexes*sizeof(tangents[0]),&tangents[0],GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribid);
			glVertexAttribPointer(attribid,4,GL_FLOAT,GL_FALSE,0,0);
			attribid++;
			break;

			case IQM_BLENDINDEXES:
			m_logger->log(LOG_DEBUG,"Got a blendindex buffer.");
			if(va.format!=IQM_UBYTE)
			{
				m_logger->log(LOG_DEBUG,"Bad format. Cannot continue.");
				return false;
			}
			glGenBuffers(1,&glmesh->buffers[IQM_BLENDINDEXES]);
			glBindBuffer(GL_ARRAY_BUFFER,glmesh->buffers[IQM_BLENDINDEXES]);
			glBufferData(GL_ARRAY_BUFFER,data_header.num_vertexes*sizeof(bindexes[0]),&bindexes[0],GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribid);
			glVertexAttribPointer(attribid,4,GL_UNSIGNED_BYTE,GL_FALSE,0,0);
			attribid++;
			break;

			case IQM_BLENDWEIGHTS:
			m_logger->log(LOG_DEBUG,"Got a blendweight buffer.");
			if(va.format!=IQM_UBYTE)
			{
				m_logger->log(LOG_DEBUG,"Bad format. Cannot continue.");
				return false;
			}
			glGenBuffers(1,&glmesh->buffers[IQM_BLENDWEIGHTS]);
			glBindBuffer(GL_ARRAY_BUFFER,glmesh->buffers[IQM_BLENDWEIGHTS]);
			glBufferData(GL_ARRAY_BUFFER,data_header.num_vertexes*sizeof(bweights[0]),&bweights[0],GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribid);
			glVertexAttribPointer(attribid,4,GL_UNSIGNED_BYTE,GL_FALSE,0,0);
			attribid++;
			break;

			//whoever uses that.. :D
			case IQM_COLOR:
			m_logger->log(LOG_DEBUG,"Got a color buffer. Lol.");
			break;
		}
	}
	if(data_header.num_triangles>0)
	{
		glGenBuffers(1,&glmesh->buffers[IQM_INDICES]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,glmesh->buffers[IQM_INDICES]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,data_header.num_triangles*3*sizeof(uint32_t),&triangles[0],GL_STATIC_DRAW);
	}
	else
    {
        m_logger->log(LOG_DEBUG,"Mesh generation failed.");
		return false;
    }

	glBindVertexArray(0);
	return true;
}

void iqmesh::set_frame(uint32_t frame)
{
    const uint32_t offset = (frame%data_header.num_frames)*data_header.num_joints;

    for(uint32_t i = 0; i < data_header.num_joints; i++)
    {
        if(joints[i].parent >= 0) current_frame[i] = mul(current_frame[joints[i].parent],frames[offset+i]);
        else current_frame[i] = frames[offset+i];
    }
}

void iqmesh::set_interp_frame(float frame)
{
    if(data_header.num_frames <= 0) return;

    int frame1 = (int)floor(frame),
        frame2 = frame1 + 1;

    float frameoffset = frame - frame1;
    frame1 %= data_header.num_frames;
    frame2 %= data_header.num_frames;
    glm::mat3x4 *mat1 = &frames[frame1 * data_header.num_joints],
              *mat2 = &frames[frame2 * data_header.num_joints];
    // Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
    // Concatenate the result with the inverse of the base pose.
    // You would normally do animation blending and inter-frame blending here in a 3D engine.

    for(uint32_t i = 0; i < data_header.num_joints; i++)
    {
        glm::mat3x4 mat = mat1[i]*(1 - frameoffset) + mat2[i]*frameoffset;
        if(joints[i].parent >= 0) current_frame[i] = mul(current_frame[joints[i].parent],mat);
        else current_frame[i] = mat;
    }
}

void iqmesh::draw(bool whole)
{
	//bind the meshe's vertex array with its bound buffers
	glBindVertexArray(glmesh->vao);

	if(whole)
	{
		//draw mesh as one unit, saves draw calls.
		glDrawElements(GL_TRIANGLES,data_header.num_triangles*3,GL_UNSIGNED_INT,0);
	}
	else
	{
		//draw each submesh separately, allows more customisation.
		for(uint32_t i=0; i<data_header.num_meshes; i++)
		{
            //glBindTexture(GL_TEXTURE_2D,submeshes[i].mat.texid);
            //draw all sub meshes using index offset
            glDrawElements(GL_TRIANGLES,submeshes[i].num_triangles*3,GL_UNSIGNED_INT,(void*)(sizeof(uint32_t)*submeshes[i].first_triangle*3));
		}
	}

	//make sure to unbind the vertex array after using it
	glBindVertexArray(0);
}

void iqmesh::free()
{
    if(data_buff) delete [] data_buff;
    if(frames) delete [] frames;
    if(base_frame) delete [] base_frame;
    if(inverse_base_frame) delete [] inverse_base_frame;
    if(current_frame) delete [] current_frame;

    if(texts) delete [] texts;
}