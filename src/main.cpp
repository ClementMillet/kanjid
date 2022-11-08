#include <iostream>
#include "kanjiinfo.hpp"

int main(int argc, char* argv[])
{
    KanjiInfo *ki = new KanjiInfo();
    Pix* img = ki->open_img(argv[1]);
    ki->img2string(img);
    ki->get_jisho();
    std::cout << pixGetWidth(img) << "x" << pixGetHeight(img) << std::endl;
    delete(ki);
    delete(img);
    return 0;
}