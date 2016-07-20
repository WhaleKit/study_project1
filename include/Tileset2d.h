#ifndef TILESET2D_H
#define TILESET2D_H

#include "Tile.h"

using namespace std;
using namespace sf;

class Tileset2d
{
public:
    Tileset2d (Uint16 width_arg, Uint16 height_arg)
        :   content_m(height_arg*width_arg, nullptr),
            height_m(height_arg),
            width_m(width_arg)
    {    }

    //reference to pointer
    Tile*& at (Vector2u index2d_arg)
    {
        return at(index2d_arg.x, index2d_arg.y);
    }
    Tile*& at (Uint16 x_arg, Uint16 y_arg)
    {
        return content_m[y_arg*width_m + x_arg];
    }
    Tile*& at (Uint32 index_arg)
    {
        return content_m[index_arg];
    }
    Tile* const& at (Vector2u index2d_arg) const
    {
        return at(index2d_arg.x, index2d_arg.y);
    }
    Tile* const & at (Uint16 x_arg, Uint16 y_arg) const
    {
        if (x_arg<width_m /*&& x_arg>=0*/ && y_arg<height_m /*&& y_arg>=0*/)
            return content_m[y_arg*width_m + x_arg];
        else
            return nullptr;
    }
    inline Uint16 getWidth () const
    {
        return width_m;
    }
    Uint16 getHeight() const
    {
        return height_m;
    }
    inline Uint16 XCoordToIndex(float xCoord_arg) const
    {
        return static_cast<float>(xCoord_arg - startOffset_m.x)/tileSize;
    }
    inline Uint16 YCoordToIndex(float yCoord_arg) const
    {
        return static_cast<float>(yCoord_arg - startOffset_m.y)/tileSize;
    }
    inline Vector2u CoordsToIndex(float x_arg, float y_arg) const
    {
        return  Vector2u(XCoordToIndex(x_arg), YCoordToIndex(y_arg)) ;
    }
    inline Vector2u CoordsToIndex(Vector2f coords_arg) const
    {
        return CoordsToIndex(coords_arg.x, coords_arg.y);
    }
    inline Vector2f IndexToCoord( Vector2u index_arg) const
    {
        return IndexToCoord(index_arg.x, index_arg.y);
    }
    inline Vector2f IndexToCoord( Uint16 x_arg, Uint16 y_arg) const
    {
        return Vector2f(XIndexToCoord(x_arg), YIndexToCoord(y_arg));
    }
    inline float XIndexToCoord (Uint16 x_arg) const
    {
        return x_arg*tileSize + startOffset_m.x;
    }
    inline float YIndexToCoord (Uint16 y_arg) const
    {
        return y_arg*tileSize + startOffset_m.y;
    }
    inline FloatRect getTileRectByIndex(Uint16 x, Uint16 y) const
    {
        return getTileRectByIndex(Vector2u(x,y));
    }
    inline FloatRect getTileRectByIndex(Vector2u index_coord) const
    {
        return FloatRect(index_coord.x*tileSize, index_coord.y*tileSize, tileSize, tileSize);
    }

    Uint8 tileSize = 100;
private:
    Vector2f startOffset_m = Vector2f(0,0); //я не могу позволить себе сделать Tileset2d прибитым гвоздями к началу координат
    //динамическая память
    vector<Tile*> content_m;
    Uint16 height_m;
    Uint16 width_m;
};



#endif // TILESET2D_H
