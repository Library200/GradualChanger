#include"GradualChanger.h"

//---------------- �÷�ʾ�� ----------------//
#ifdef GRADUAL_CHANGER_DEMO
#include <iostream>
int main() {
    int   clock = 0;
    float x = 0.f;
    int   y = 10;

    // ����������������
    new GradualChanger<float>(x, 100.f, clock, 120); // 2 �루���� 60 fps��
    new GradualChanger<int>(y, 0, clock, 60);        // 1 ��

    // ģ����ѭ��
    for (clock = 0; clock < 200; ++clock) {
        GradualChangerBase::update();
        if (clock % 20 == 0)
            std::cout << "t=" << clock << "  x=" << x << "  y=" << y << '\n';
    }
}
#endif