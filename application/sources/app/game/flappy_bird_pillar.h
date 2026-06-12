#ifndef __FLAPPY_BIRD_PILLAR_H__
#define __FLAPPY_BIRD_PILLAR_H__

class Pillar
{
public:
    static const int PILLAR_WIDTH = 10;

    int x;
    int gap;
    int gapTop;

    int speed;
    int baseSpeed;  
    int type;

    void init();
    void update();
    void draw();

    void nextVariant();
    void applyDifficulty(int score);
};

#endif