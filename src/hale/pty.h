#ifndef PTY_H
#define PTY_H

class PtyTerminal;

class Pty
{
public:
    Pty();

    PtyTerminal *open(int cols = 80, int rows = 24);
    void resize();
    void startProcess();
    void kill();
};

class PtyTerminal
{

};

#endif // PTY_H
