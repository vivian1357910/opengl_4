#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_draw.H>
#include <FL/fl_Box.H>
#include <FL/Fl_Button.H>
#include <iostream>
#include "BMP_IO.h"
#include <math.h>

using namespace std;

Fl_Image* p1 = nullptr;
Fl_Image* p2 = nullptr;
Fl_Image* p3 = nullptr;
Fl_Image* p4 = nullptr;
Fl_Box* pbox1 = new Fl_Box(10, 10, 200, 200);
Fl_Box* pbox2 = new Fl_Box(210, 10, 200, 200);
Fl_Box* pbox3 = new Fl_Box(410, 10, 200, 200);
Fl_Box* pbox4 = new Fl_Box(610, 10, 200, 200);
Fl_Box* pbox_main = new Fl_Box(10, 210, 800, 800);
int t = 0;
int n = 1;
int width = 0;
int height = 0;
int overlap = 300; // pixel
int vh[1002];
const char* outname = "output.bmp";
static BYTE* bp1[1550 * 1010 * 3] = { 0 };
static BYTE* bp2[1550 * 1010 * 3] = { 0 };
static BYTE* bp3[1550 * 1010 * 3] = { 0 };
static BYTE* bp4[1550 * 1010 * 3] = { 0 };
static BYTE* mp[5000 * 1010 * 3] = { 0 };
static BYTE* pixels[1550 * 1010 * 3] = { 0 };
void mergeFind(BYTE** left, BYTE** right, int ov, int h)
{
    //cout << "merge!!" << endl;
    int w = width - overlap;
    float minD = 1000000;
    //cout << "left: " << (float)(*bp1)[0] << endl;
    if (h == 0)
    {
        for (int i = 0; i < ov; i++)
        {
            float rl = (*left)[(h * height + w + i) * 3 + 0];
            float gl = (*left)[(h * height + w + i) * 3 + 1];
            float bl = (*left)[(h * height + w + i) * 3 + 2];
            float rr = (*right)[(h * height + i) * 3 + 0];
            float gr = (*right)[(h * height + i) * 3 + 1];
            float br = (*right)[(h * height + i) * 3 + 2];
            float D = pow(rl - rr, 2) + pow(gl - gr, 2) + pow(bl - br, 2);
            if (D < minD)
            {
                minD = D;
                vh[h] = i;
            }
        }
        h++;
        //cout << "merge in !! " << vh[h - 1] << endl;
        return mergeFind(left, right, vh[h - 1], h);
    }
    else if (h == height - 1)
    {
        for (int i = ov - 1; i < ov + 1; i++)
        {
            float rl = (*left)[(h * height + w + i) * 3 + 0];
            float gl = (*left)[(h * height + w + i) * 3 + 1];
            float bl = (*left)[(h * height + w + i) * 3 + 2];
            float rr = (*right)[(h * height + i) * 3 + 0];
            float gr = (*right)[(h * height + i) * 3 + 1];
            float br = (*right)[(h * height + i) * 3 + 2];
            float D = pow(rl - rr, 2) + pow(gl - gr, 2) + pow(bl - br, 2);
            if (D < minD)
            {
                minD = D;
                vh[h] = i;
            }
        }
        cout << "merge out !!" << endl;
        cout << "find a path !!" << endl;
        return;
    }
    else
    {
        for (int i = ov - 1; i <= ov + 1; i++)
        {
            if (i < 0 || i >= overlap)
                continue;

            float rl = (*left)[(h * height + w + i) * 3 + 0];
            float gl = (*left)[(h * height + w + i) * 3 + 1];
            float bl = (*left)[(h * height + w + i) * 3 + 2];
            float rr = (*right)[(h * height + i) * 3 + 0];
            float gr = (*right)[(h * height + i) * 3 + 1];
            float br = (*right)[(h * height + i) * 3 + 2];
            float D = pow(rl - rr, 2) + pow(gl - gr, 2) + pow(bl - br, 2);
            if (D < minD)
            {
                minD = D;
                vh[h] = i;

            }
        }
        //cout << minD << " " << vh[h] << endl;
        h++;
        cout << "add path: " << vh[h - 1] << endl;
        return mergeFind(left, right, vh[h - 1], h);
    }
}

void mergeDisplay()
{
    if (p1 != nullptr && p2 != nullptr)
    {
        mergeFind(bp1, bp2, overlap, 0);
        (*mp) = (*bp1);

        for (int i = 0; i < height; i++)
            for (int j = 0; j < width * 2 - overlap; j++)
            {

                if (j < width - overlap + vh[i])
                {

                    (*mp)[(i * height + j) * 3 + 0] = (*bp1)[(i * height + j) * 3 + 0];
                    (*mp)[(i * height + j) * 3 + 1] = (*bp1)[(i * height + j) * 3 + 1];
                    (*mp)[(i * height + j) * 3 + 2] = (*bp1)[(i * height + j) * 3 + 2];

                }
                else
                {
                    (*mp)[(i * height + j) * 3 + 0] = (*bp2)[(i * height + vh[i]) * 3 + 0];
                    (*mp)[(i * height + j) * 3 + 1] = (*bp2)[(i * height + vh[i]) * 3 + 1];
                    (*mp)[(i * height + j) * 3 + 2] = (*bp2)[(i * height + vh[i]) * 3 + 2];
                }
            }
        //BMP_Write(outname, mp, width * 2 - overlap, height);
        cout << "Write!!" << endl;
    }


}

void FileChooser_cb(Fl_File_Chooser* fc, void* data)
{
    // output
    const char* current = fc->value(1);
    cout << "Load: " << current << endl;

    // ¶}±Ò
    Fl_BMP_Image tmp(current);
    width = tmp.w();
    height = tmp.h();

    BMP_Read(current, pixels, width, height);
    //cout << "pixels: " << (float)(*pixels)[1] << endl;
    if (p1 == nullptr && t == 1)
    {
        p1 = tmp.copy(200, 200);
        (*bp1) = (*pixels);
        //cout << "bp1: " << (float)(*bp1)[0] << endl;
        //cout << "p1" << endl;
    }
    else if (p2 == nullptr && t == 1)
    {
        p2 = tmp.copy(200, 200);
        (*bp2) = (*pixels);
        //cout << "bp2: " << (float)(*bp2)[0] << endl;
        //cout << "p2" << endl;
    }
    else if (p3 == nullptr && t == 1)
    {
        p3 = tmp.copy(200, 200);
        (*bp3) = (*pixels);
        //cout << "p3" << endl;
    }
    else if (p4 == nullptr && t == 1)
    {
        p4 = tmp.copy(200, 200);
        (*bp4) = (*pixels);
        //cout << "p4" << endl;
    }
    t++;

}

void chooserBtn_cb(Fl_Widget* w, void* box)
{
    Fl_File_Chooser fc(".", "*", Fl_File_Chooser::SINGLE, "File Chooser");
    fc.callback(FileChooser_cb);
    fc.show();
    while (fc.visible())
        Fl::wait();
    t = 0;
    if (n == 1)
    {
        pbox1->image(p1);
        //cout << "b1" << endl;
        n++;
        pbox1->redraw();
    }
    else if (n == 2)
    {
        pbox2->image(p2);
        //cout << "b2" << endl;
        n++;
        pbox2->redraw();
    }
    else if (n == 3)
    {
        pbox3->image(p3);
        //cout << "b3" << endl;
        n++;
        pbox3->redraw();
    }
    else if (n == 4)
    {
        pbox4->image(p4);
        //cout << "b4" << endl;
        pbox4->redraw();
    }
}

void resetBtn_cb(Fl_Widget* w, void* box)
{
    p1 = nullptr;
    p2 = nullptr;
    p3 = nullptr;
    p4 = nullptr;
    (*bp1) = { 0 };
    (*bp2) = { 0 };
    (*bp3) = { 0 };
    (*bp4) = { 0 };
    t = 0;
    n = 1;
    pbox1->image(p1);
    pbox1->redraw();
    pbox2->image(p2);
    pbox2->redraw();
    pbox3->image(p3);
    pbox3->redraw();
    pbox4->image(p4);
    pbox4->redraw();
    pbox_main->image(p1);
    pbox_main->redraw();
}

void mergeBtn_cb(Fl_Widget* w, void* data)
{
    cout << "merge!!" << endl;
    mergeDisplay();
    pbox_main->image(p1);
    pbox_main->redraw();
}

void quitBtn_cb(Fl_Widget* w, void* data)
{
    cout << "finish !!" << endl;
    exit(0);
}

int  main(int argc, char* argv[])
{
    fl_register_images();
    Fl_Double_Window* window = new Fl_Double_Window(1000, 1000, "Test FLTK");

    pbox1 = new Fl_Box(10, 10, 200, 200);
    pbox2 = new Fl_Box(210, 10, 200, 200);
    pbox3 = new Fl_Box(410, 10, 200, 200);
    pbox4 = new Fl_Box(610, 10, 200, 200);
    pbox_main = new Fl_Box(10, 210, 800, 800);
    pbox1->box(FL_UP_BOX);
    pbox2->box(FL_UP_BOX);
    pbox3->box(FL_UP_BOX);
    pbox4->box(FL_UP_BOX);
    pbox_main->box(FL_UP_BOX);
    Fl_Button* chooserBtn = new Fl_Button(850, 50, 100, 50, "Load");
    chooserBtn->callback((Fl_Callback*)chooserBtn_cb, pbox1);
    Fl_Button* resetBtn = new Fl_Button(850, 110, 100, 50, "Reset");
    resetBtn->callback((Fl_Callback*)resetBtn_cb, pbox1);
    Fl_Button* mergeBtn = new Fl_Button(850, 850, 100, 50, "Merge");
    mergeBtn->callback((Fl_Callback*)mergeBtn_cb, pbox_main);
    Fl_Button* quitBtn = new Fl_Button(850, 910, 100, 50, "Quit");
    quitBtn->callback((Fl_Callback*)quitBtn_cb, pbox_main);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}