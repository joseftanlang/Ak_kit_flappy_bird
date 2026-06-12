#ifndef __FLAPPY_BIRD_BACKGROUND_H__
#define __FLAPPY_BIRD_BACKGROUND_H__

class Background
{
public:
    void init();
    void update(int score);
    void draw(int score, int best);

private:
    int cloudX1;
    int cloudX2;
    int cloudX3;

    int skyPhase; // 0 = day, 1 = sunset, 2 = night
};

#endif