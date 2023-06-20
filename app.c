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

#include <stdio.h>
#include <math.h>
#include <string.h>

void init(void)
{
  glClearColor (0, 0, 0, 0);
   glShadeModel (GL_FLAT);
}

double aspect = 1;
int width = 1280, height = 960;
void display(void)
{

   glClear (GL_COLOR_BUFFER_BIT);

  glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   // glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
   gluPerspective(60, aspect, 1.5, 20);

  // default 0,0,0 -> z- (up= y+)
  glTranslatef(-4, 0, 0); // 原点を左に移動する
  // eye -> center, (up)
  gluLookAt(-2, 0, 7, 0, 0, 0, 0, 1, 0);

   glMatrixMode (GL_MODELVIEW);
  
   // 以下はsample-cube.cのコピペ
   // xが奥、zが右手前、yが上
   
   //glLoadIdentity ();             /* clear the matrix */
           /* viewing transformation  */
   //gluLookAt (0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // glTranslatef(0, 0, -5.0);
   
   //gluLookAt(3, 1, 1, 0, 0, 0, 0, 1, 0);
   //gluLookAt(0, 0, 0, -3, -3, -3, 0, 1, 0);
   //glTranslatef(-3, -3, -3);

   //glScalef (1.0, 2.0, 1.0);      /* modeling transformation */

   // 自機
   glLoadIdentity();
   glTranslatef(0, y, 0);
   if(fabs(y) < Y_RANGE){
     glColor3f (0.3, 0.6, 1);
   }else{
     // 範囲外だよ 色を変える
     glColor3f(0.1, 0.1, 0.3);
   }
   
   glutWireCube (1.0);


   for(struct GameObj *g = game_obj; g < game_obj + GAME_OBJ_NUM; g++){
     if(g->score_t > 0){
       // オブジェクトごとの点数表示
       char score_text[20];
       sprintf(score_text, "%d", g->score);
       glLoadIdentity();
       glColor3f(1, 1, 1);
       glRasterPos3f(0, g->y + 0.6, 0);
       glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, score_text);
     }

     if(g->kind == g_none){
       continue;
     }
     
     glLoadIdentity();
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
       glutSolidTorus(0.05, 0.4, 8, 20);
       break;
     }
     
     
   }

   glLoadIdentity();
   glColor3f(0.3, 0.3, 0.3);
   glBegin(GL_LINE_STRIP);
   glVertex3f(0, -3, 0);
   glVertex3f(0, 3, 0);
   glVertex3f(10, 3, 0);
   glVertex3f(10, -3, 0);
   glVertex3f(0, -3, 0);
   glVertex3f(0, 0, 0);
   glVertex3f(10, 0, 0);
   glEnd();
   

   //   glColor3f(1, 1, 1);
   //glutSolidCube(1.0);
   //glColor3f(0.5, 0.3, 0);
   //glutWireCube(1);

   //glColor3f(0, 0, 0);
   //glTranslatef(0, 0, -2); // -3, -3, -5 に描かれる
   //glScalef(0.5, 0.25, 0.5); // 0.5, 0.5, 0.5 にする
   //glutWireDodecahedron();

   //glColor3f(1, 0, 0);
   //glTranslatef(-7, -2, 2); // -10, -5, -3 (←なんか違いそう)
   //glScalef(2, 2, 2); // 1, 1, 1 にする
   //glRotatef(-90, 1, 0, 0);
   //glutWireCone(0.5, 2, 30, 30);
   
   //glFlush ();


   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, width, 0, height);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   glColor3f(1, 1, 1);
   /*   glBegin(GL_POLYGON);
   glVertex3f(0, 0, 0);
   glVertex3f(0, 0.3, 0);
   glVertex3f(0.3, 0.3, 0);
   glVertex3f(0.3, 0, 0);
   glEnd();
   */
   glRasterPos2f(10, 10);
   glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "Score:");
   glRasterPos2f(80, 10);
   char score_text[20];
   sprintf(score_text, "%d", score);
   glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, score_text);

   if(game_over){
     glLoadIdentity();
     glColor3f(1, 0.2, 0.2);
     glTranslatef(width / 2, height / 2, 0);
     glRasterPos2f(-70, -9);
     glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "GAME OVER");
     glBegin(GL_LINE_LOOP);
     glVertex2f(-100, -30);
     glVertex2f(-100, 30);
     glVertex2f(100, 30);
     glVertex2f(100, -30);
     glEnd();
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

void update(){
  if(game_update()){
    glutPostRedisplay();
  }
}

int main(int argc, char** argv)
{
  if(!init_serial()){
    //return 1;
  }

  
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutInitWindowPosition (100, 100);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(update);
   glutMainLoop();
   return 0;
}
