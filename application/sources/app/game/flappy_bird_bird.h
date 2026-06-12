#ifndef __FLAPPY_BIRD_BIRD_H__
#define __FLAPPY_BIRD_BIRD_H__

class Bird
{
public:
    static const int BIRD_WIDTH = 15;
    static const int BIRD_HEIGHT = 15;

    int x;
    int y;
    int vy;

    void init();
    void update();
    void flap();
    void draw();
};

#endif