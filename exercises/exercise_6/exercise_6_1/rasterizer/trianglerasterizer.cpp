#include "trianglerasterizer.h"

/*
 * \class triangle_rasterizer
 * A class which scanconverts a triangle. It computes the pixels such that they are inside the triangle.
 */
triangle_rasterizer::triangle_rasterizer(int x1, int y1, int x2, int y2, int x3, int y3) : valid(false)
{
    this->initialize_triangle(x1, y1, x2, y2, x3, y3);
}

/*
 * Destroys the current instance of the triangle rasterizer
 */
triangle_rasterizer::~triangle_rasterizer()
{}

/*
 * Returns a vector which contains alle the pixels inside the triangle
 */
std::vector<glm::ivec2> triangle_rasterizer::all_pixels()
{
    std::vector<glm::ivec2> points;

    while (this->more_fragments()) {
        points.push_back(glm::ivec2(x_current, y_current));
        this->next_fragment();
    }

    return points;
}

/*
 * Checks if there are fragments/pixels inside the triangle ready for use
 * \return true if there are more fragments in the triangle, else false is returned
 */
bool triangle_rasterizer::more_fragments() const
{
    return this->valid;
}

/*
 * Computes the next fragment inside the triangle
 */
void triangle_rasterizer::next_fragment()
{
    if(x_current < x_stop)
        x_current += 1;
    else { // scan line finished, go to next non-empty scan line
        leftedge.next_fragment();
        rightedge.next_fragment();
        while(leftedge.more_fragments() && leftedge.x() >= rightedge.x()) {
            leftedge.next_fragment();
            rightedge.next_fragment();
        }
        valid = leftedge.more_fragments();
        if(valid){
            x_start = leftedge.x();
            x_stop = rightedge.x() - 1;
            x_current = x_start;
            y_current = leftedge.y();
        }
    }
}

/*
 * Returns the current x-coordinate of the current fragment/pixel inside the triangle
 * It is only valid to call this function if "more_fragments()" returns true,
 * else a "runtime_error" exception is thrown
 * \return The x-coordinate of the current triangle fragment/pixel
 */
int triangle_rasterizer::x() const
{
    if (!this->valid) {
        throw std::runtime_error("triangle_rasterizer::x(): Invalid State/Not Initialized");
    }
    return this->x_current;
}

/*
 * Returns the current y-coordinate of the current fragment/pixel inside the triangle
 * It is only valid to call this function if "more_fragments()" returns true,
 * else a "runtime_error" exception is thrown
 * \return The y-coordinate of the current triangle fragment/pixel
 */
int triangle_rasterizer::y() const
{
    if (!this->valid) {
        throw std::runtime_error("triangle_rasterizer::y(): Invalid State/Not Initialized");
    }
    return this->y_current;
}

/*
 * Initializes the TriangleRasterizer with the three vertices
 * \param x1 - the x-coordinate of the first vertex
 * \param y1 - the y-coordinate of the first vertex
 * \param x2 - the x-coordinate of the second vertex
 * \param y2 - the y-coordinate of the second vertex
 * \param x3 - the x-coordinate of the third vertex
 * \param y3 - the y-coordinate of the third vertex
 */
void triangle_rasterizer::initialize_triangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
    ivertex[0] = glm::ivec2(x1,y1);
    ivertex[1] = glm::ivec2(x2,y2);
    ivertex[2] = glm::ivec2(x3,y3);

    lower_left = LowerLeft();
    upper_left = UpperLeft();
    the_other = 3 - lower_left - upper_left;

    glm::ivec2 e1(ivertex[upper_left] - ivertex[lower_left]);
    glm::ivec2 e2(ivertex[the_other] - ivertex[lower_left]);

    int z_e1xe2 = e1.x * e2.y - e1.y * e2.x;
    if(z_e1xe2 > 0) { // Red triangle case
        leftedge.init(ivertex[lower_left].x, ivertex[lower_left].y,
                      ivertex[the_other].x, ivertex[the_other].y,
                      ivertex[upper_left].x, ivertex[upper_left].y);
        rightedge.init(ivertex[lower_left].x, ivertex[lower_left].y,
                      ivertex[upper_left].x, ivertex[upper_left].y);
    }
    else if(z_e1xe2 < 0) { // Blue triangle case
        leftedge.init(ivertex[lower_left].x, ivertex[lower_left].y,
                      ivertex[upper_left].x, ivertex[upper_left].y);
        rightedge.init(ivertex[lower_left].x, ivertex[lower_left].y,
                      ivertex[the_other].x, ivertex[the_other].y,
                      ivertex[upper_left].x, ivertex[upper_left].y);
    }

    x_start = leftedge.x();
    y_start = leftedge.y();
    x_stop = rightedge.x() - 1;
    y_stop = ivertex[upper_left].y;
    x_current = x_start;
    y_current = y_start;

    valid = x_start <= x_stop;
    if(!valid)
        next_fragment();
}

/*
 * Computes the index of the lower left vertex in the array ivertex
 * \return the index in the vertex table of the lower left vertex
 */
int triangle_rasterizer::LowerLeft()
{
    int ll = 0;
    for(int i=ll+1; i<3; i++) {
        if(ivertex[i].y < ivertex[ll].y || (ivertex[i].y == ivertex[ll].y && ivertex[i].x < ivertex[ll].x))
            ll = i;
    }
    return ll;
}

/*
 * Computes the index of the upper left vertex in the array ivertex
 * \return the index in the vertex table of the upper left vertex
 */
int triangle_rasterizer::UpperLeft()
{
    int ul = 0;
    for(int i=ul+1; i<3; i++) {
        if(ivertex[i].y > ivertex[ul].y || (ivertex[i].y == ivertex[ul].y && ivertex[i].x < ivertex[ul].x))
            ul = i;
    }
    return ul;
}