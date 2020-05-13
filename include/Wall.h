#ifndef  WALL_INC
#define  WALL_INC

#include <stdlib.h>
#include <stdint.h>

class Wall
{
    public:
        Wall();

        /* \brief Destructor. Destroy the data */
        virtual ~Wall();



        /* \brief Get the vertices data of the wall
         * \return const array on the vertices data. Use getNbVertices to get how many vertices the array contains (size(array) == 3*nbVertices) */
        const float* getVertices() const {return m_vertices;}

        /* \brief Get the normals data of the wall
         * \return const array on the normals data. Use getNbVertices to get how many vertices the array contains (size(array) == 3*nbVertices) */
        const float* getNormals() const {return m_normals;}

        /* \brief Get the UV mapping data of the wall
         * \return const array on the UV mapping data. Use getNbVertices to get how many vertices the array contains (size(array) == 2*nbVertices) */
        const float* getUVs() const {return m_uvs;}

        /* \brief Get how many vertices this wall contains
         * \return the number of vertices this wall contains*/
        uint32_t getNbVertices() const {return m_nbVertices;}

    protected: 
        uint32_t m_nbVertices = 18;
        float*   m_vertices   = NULL;
        float*   m_normals    = NULL;
        float*   m_uvs        = NULL;
};

#endif
