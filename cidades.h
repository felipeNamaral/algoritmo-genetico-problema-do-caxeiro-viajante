#include "raylib.h"

class Cidades {



    public:
        float getX() const { return x; };
        float getY() const { return y; };
        Color getCor() const { return cor; };
        int getId() const { return id; };

        Cidades(float x = 0, float y = 0, int id = 0): id(id), x(x), y(y) {}


    private:
        int id;
        float x,y;
        Color cor = {255, 255, 255, 255};


};
