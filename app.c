/*
 * Copyright (c) 1993-1997, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
 */

/*
 *  cube.c
 *  This program demonstrates a single modeling transformation,
 *  glScalef() and a single viewing transformation, gluLookAt().
 *  A wireframe cube is rendered.
 */
#include <GL/glut.h>
#include <stdlib.h>

#include "serial.h"
#include "game.h"
#include "audio.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

// #define SHOW_GRID

int width = 1280, height = 960;
double aspect = 16.0 / 9;
// int width = 800, height = 450;

void display_text(double r, double g, double b, int x, const char *str, double s){
  glPushMatrix();
  {
    glColor3f(r, g, b);
    glTranslatef(width / 2, height / 2, 0);

    glRasterPos2f(x, -9);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, str);
    glBegin(GL_LINE_LOOP);
    {
      glVertex2f(-100, -30);
      glVertex2f(-100, 30);
      glVertex2f(100, 30);
      glVertex2f(100, -30);
    }
    glEnd();
    glBegin(GL_LINES);
    {
      glVertex2f(-90, -20);
      glVertex2f(-90 + 180 * s, -20);
    }
    glEnd();
  }
  glPopMatrix();
}
void display_title(double r, double g, double b){
  glPushMatrix();
  {
    glColor3f(r, g, b);
    glTranslatef(width / 2, height / 2, 0);

    glRasterPos2f(-70, 20);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "Virtual Plane");
    glBegin(GL_LINES);
    {
      glVertex2f(-200, 0);
      glVertex2f(200, 0);
    }
    glEnd();

    glRasterPos2f(-120, -40);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "< Press Button to Start >");
    
/*    glRasterPos2f(-105, -120);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "--- How to Play ---");
    glRasterPos2f(-260, -180);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "1. Hold the circuit board with the sensor facing down,");
    glRasterPos2f(-240, -210);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "and move it to control the plane.");
    glRasterPos2f(-260, -250);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "2. Press the button to shoot.");
*/

  }
  glPopMatrix();
}

void display(void)
{

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  // glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
  gluPerspective(60, aspect, 1.5, 50);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
  // default 0,0,0 -> z- (up= y+)
  // eye -> center, (up)
  gluLookAt(2.5, -0.5, 7, 4, 0, 0, 0, 1, 0);
  
  // https://www.oit.ac.jp/is/L231/~whashimo/Article/OpenGL/Chapter3/index.html
  // https://www.oit.ac.jp/is/L231/~whashimo/Article/OpenGL/Chapter5/index.html
  // https://stackoverflow.com/questions/55338066/how-do-i-make-opengl-specular-light-work
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  static float light0[4][4]={{1, 10, 1, 0}, //position
                      {0.2, 0.2, 0.2, 1}, //ambient
                      {0.8, 0.8, 0.8, 1}, //diffuse
                      {0.4, 0.4, 0.4, 1}}; //specular
  glLightfv(GL_LIGHT0, GL_POSITION, light0[0]);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0[1]);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0[2]);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light0[3]);

  glEnable(GL_DEPTH_TEST);

  // 背景
  static double scroll = 0;
  srand(0);
  for(int d = 1; d <= 3; d++){
    double z = -5 * d;
    double xr = (-z + 7) / 7 * 10 * 1.5;
    double yr = (-z + 7) / 7 * 6 * 2;
    for(int n = 0; n < 20; n++){
      double x = xr * rand1() - xr * 0.2 - scroll;
      while(x < - xr * 0.2){
	x += xr;
      }
      double y = yr * rand1() - yr * 0.5;
      glPushMatrix();
      glColor3f(1, 1, 0.9);
      glTranslatef(x, y, z);
      glutSolidSphere(0.05, 5, 5);
      glPopMatrix();
    }
  }
  srand((unsigned int)time(NULL));
  scroll += 0.1;
  
  // xが右奥、zが右前、yが上
  glPushMatrix();
  {
    // 自機
    glTranslatef(0, y, 0);
    glRotatef(-my_angle() * 180 / 3.14, 0, 0, -1);

    if(fabs(y) < Y_RANGE){
      glColor3f (0.5, 0.8, 1);
    }else{
      // 範囲外だよ 色を変える
      glColor3f(0.1, 0.2, 0.3);
    }
    // glutSolidSphere(0.5, 50, 50);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glBegin(GL_TRIANGLES);
    {
      glNormal3f(0, 1, 0);
      glVertex3f(0.5, 0.2, 0);
      glVertex3f(-0.5, 0.2, 0.1);
      glVertex3f(-0.5, 0.15, 0.5);

      glNormal3f(0, 1, 0);
      glVertex3f(0.5, 0.2, 0);
      glVertex3f(-0.5, 0.15, -0.5);
      glVertex3f(-0.5, 0.2, -0.1);

      glNormal3f(0, 0, -1);
      glVertex3f(0.5, 0.2, 0);
      glVertex3f(-0.5, -0.2, 0);
      glVertex3f(-0.5, 0.2, 0.1);

      glNormal3f(0, 0, 1);
      glVertex3f(0.5, 0.2, 0);
      glVertex3f(-0.5, 0.2, -0.1);
      glVertex3f(-0.5, -0.2, 0);
    }
    glEnd();

  }
  glPopMatrix();

  // オブジェクト
  // 奥のものから順に描画する
  for(int i = game_obj_current; i > game_obj_current - GAME_OBJ_NUM; i--){
    struct GameObj *g = &game_obj[(i + GAME_OBJ_NUM) % GAME_OBJ_NUM];
    if(g->score_t > 0){
      // オブジェクトごとの点数表示
      char text[20];
      sprintf(text, "%d", g->score);
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST); // これは常に手前に表示する
      glColor3f(1, 1, 1);
      glRasterPos3f(g->score_x, g->score_y + 0.5, 0);
      glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, text);
      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
    }

    if(g->kind == g_none){
      continue;
    }
     
    glPushMatrix();
    {
      glTranslatef(g->x, g->y, 0);
      glRotatef(g->t * 90, 1, 0.7, 0);

      switch(g->kind){
      case g_block:
        glColor3f(1, 0.8, 0.7);
        glutSolidCube(0.5);
        break;
      case g_coin:
        glColor3f(1, 1, 0.3);
        // innerR, outerR, nsides, rings
        glutSolidTorus(0.05, 0.3, 8, 20);
        break;
      case g_yakan:
        glColor3f(1, 1, 1);
        glutSolidTeapot(0.5);
        break;
      case g_bullet:
        glColor3f(1, 0.5, 0.3);
        // r, slices, stacks
        glutSolidSphere(0.1, 10, 10);
        break;
      case g_none:
	break;
      }
    }
    glPopMatrix();
  }

  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_COLOR_MATERIAL);

#ifdef SHOW_GRID
  glColor3f(0.3, 0.3, 0.3);
  glBegin(GL_LINES);
  {
    glVertex3f(0, -Y_RANGE, 0);
    glVertex3f(0, Y_RANGE, 0);
    glVertex3f(X_RANGE / 3, -Y_RANGE, 0);
    glVertex3f(X_RANGE / 3, Y_RANGE, 0);
    glVertex3f(X_RANGE * 2 / 3, -Y_RANGE, 0);
    glVertex3f(X_RANGE * 2 / 3, Y_RANGE, 0);
    glVertex3f(X_RANGE, -Y_RANGE, 0);
    glVertex3f(X_RANGE, Y_RANGE, 0);
    for(double y = -Y_RANGE; y <= Y_RANGE + .1; y +=0.5){
      glVertex3f(0, y, 0);
      glVertex3f(X_RANGE, y, 0);
    }
    for(double x = -1; x <= X_RANGE + .1; x +=0.5){
      glVertex3f(x, 0, -5);
      glVertex3f(x, 0, 5);
    }
  }
  glEnd();
#endif

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  char text[20];

  glColor3f(1, 1, 1);
  glRasterPos2f(10, 35);
  glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "Score:");
  glRasterPos2f(80, 35);
  sprintf(text, "%d", (int)(score));
  glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, text);
  glRasterPos2f(10, 10);
  glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "HiScore:");
  glRasterPos2f(100, 10);
  sprintf(text, "%d", hiscore);
  glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, text);

  switch(game_state){
  case g_over:
    display_text(1, 0.2, 0.2, -60, "Game Over", 0);
    break;
  case g_title:
    display_title(0.2, 0.7, 1);
    break;
  case g_ready:
    display_text(0.2, 0.7, 1, -35, "Ready?", game_main_t / READY_T);
    break;
  case g_main:
    if(game_main_t < READY_T / 2){
      display_text(1, 0.7, 0.2, -30, "Start!", 0);
    }
    break;
  }

  glFlush();

  glutSwapBuffers();
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   aspect = (double)w / h;
   width = w;
   height = h;
}

/* ARGSUSED1 */
void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
         exit(0);
         break;
   }
}
void mouse_click(int b, int s, int x, int y){
  if(b == 2 && s == GLUT_DOWN){ //右クリックのみ反応
    mouse_clicked = 1;
  }
}
void mouse_motion(int x, int y){
  mouse_x_rat = (double)x / width;
  mouse_y_rat = (double)y / height;
}

void update(){
  if(game_update()){
    glutPostRedisplay();
  }
}

int main(int argc, char** argv)
{
  if(!init_serial()){
    //return 1;
    printf("using mouse instead\n");
    use_mouse = 1;
  }
  init_wav();
  init_pa();
  init_game();
  
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(width, height);
  glutInitWindowPosition (100, 100);
  glutCreateWindow (argv[0]);
  glClearColor (0, 0, 0, 0);
  glShadeModel (GL_FLAT);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(update);
  if(use_mouse){
    glutMouseFunc(mouse_click);
    glutMotionFunc(mouse_motion);
  }
  glutMainLoop();
  return 0;
}
