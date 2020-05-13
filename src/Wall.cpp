#include "Wall.h"

Wall::Wall()
{
    m_vertices = (float*)malloc(sizeof(float) * 3 * 18);
    m_normals  = (float*)malloc(sizeof(float) * 3 * 18);
    m_uvs      = (float*)malloc(sizeof(float) * 2 * 18);

    float vertices[3 * 18] = {	//Back
								-0.5, -0.5,  0.5,
								 0.5,  0.5,  0.5,
								 0.5, -0.5,  0.5,
								-0.5, -0.5,  0.5,
								-0.5,  0.5,  0.5,
								 0.5,  0.5,  0.5,

								 //Left
								 -0.5, -0.5,  0.5,
								 -0.5, -0.5, -0.5,
								 -0.5,  0.5, -0.5,
								 -0.5, -0.5,  0.5,
								 -0.5,  0.5, -0.5,
								 -0.5,  0.5,  0.5,

								 //Right
								  0.5, -0.5,  0.5,
								  0.5,  0.5, -0.5,
								  0.5, -0.5, -0.5,
								  0.5, -0.5,  0.5,
								  0.5,  0.5,  0.5,
								  0.5,  0.5, -0.5 };
/*
    float uvs[2*36] = {		//Back
                            0.0, 0.0,
                             1.0,  1.0,
                             1.0, 0.0,
                            0.0, 0.0,
                            0.0,  1.0,
                             1.0,  1.0,

                            //Left
                            0.0,  1.0,
                            0.0, 0.0,
                             1.0, 0.0,
                            0.0,  1.0,
                             1.0, 0.0,
                             1.0,  1.0,

                            //Right
                            0.0,  1.0,
                             1.0, 0.0,
                            0.0, 0.0,
                            0.0,  1.0,
                             1.0,  1.0,
                             1.0, 0.0	};
*/

    float normals[3 * 18] = {	//Back
							   0.0, 0.0, 1.0,
							   0.0, 0.0, 1.0,
							   0.0, 0.0, 1.0,
							   0.0, 0.0, 1.0,
							   0.0, 0.0, 1.0,
							   0.0, 0.0, 1.0,

							   //Left
							   -1.0, 0.0, 0.0,
							   -1.0, 0.0, 0.0,
							   -1.0, 0.0, 0.0,
							   -1.0, 0.0, 0.0,
							   -1.0, 0.0, 0.0,
							   -1.0, 0.0, 0.0,

							   //Right
								1.0, 0.0, 0.0,
								1.0, 0.0, 0.0,
								1.0, 0.0, 0.0,
								1.0, 0.0, 0.0,
								1.0, 0.0, 0.0,
								1.0, 0.0, 0.0 };
	
    for(uint32_t i = 0; i < 3*18; i++)
    {
        m_normals[i] = vertices[i];
		m_vertices[i] = normals[i];
    }

}


Wall::~Wall()
{
    if(m_vertices)
        free(m_vertices);
    if(m_normals)
        free(m_normals);
    if(m_uvs)
        free(m_uvs);
}