
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "graphics.h"

extern GLubyte  world[WORLDX][WORLDY][WORLDZ];
extern float tubeData[TUBE_COUNT][6];
extern int screenWidth;
extern int screenHeight;
extern int displayMap;
extern float keyboardPressed;
extern void toggleKeyboardPress();

/* Kevins changes */
typedef int bool;
#define true 1
#define false 0
#define HUMAN_NUMBER 4
#define TUBE_SIZE 3
#define TUBE_LIFE 70
#define SEGMENTS 15
#define MAP_BORDER_SIZE 2


clock_t newTime, oldTime;
int humans[HUMAN_NUMBER][3];
int tubeMovement[TUBE_COUNT];
int tubeHit[TUBE_COUNT];
float acceleration = 1;
float distX=0, distY=0, distZ=0;

void goBack(float x, float y, float z);
void optimizeGround(int groundArray[100][100]);


/* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

/* initialize graphics library */
extern void graphicsInit(int *, char **);

/* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat* getLightPosition();

/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setOldViewPosition(float, float, float);
extern void setViewOrientation(float, float, float);
extern void getViewOrientation(float *, float *, float *);

/* add cube to display list so it will be drawn */
extern void addDisplayList(int, int, int);

/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

/* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

/* tube controls */
extern void createTube(int, float, float, float, float, float, float, int);
extern void hideTube(int);
extern void showTube(int);

/* 2D drawing functions */
extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);


/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
/* flag used to indicate that the test world should be used */
extern int testWorld;
/* flag to print out frames per second */
extern int fps;
/* flag to indicate the space bar has been pressed */
extern int space;
/* flag indicates the program is a client when set = 1 */
extern int netClient;
/* flag indicates the program is a server when set = 1 */
extern int netServer;
/* size of the window in pixels */
extern int screenWidth, screenHeight;
/* flag indicates if map is to be printed */
extern int displayMap;
/* flag indicates use of a fixed viewpoint */
extern int fixedVP;

/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/* allows users to define colours */
extern int setUserColour(int, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
void unsetUserColour(int);
extern void getUserColour(int, GLfloat *, GLfloat *, GLfloat *, GLfloat *, GLfloat *, GLfloat *, GLfloat *, GLfloat *);

    /********* end of extern variable declarations **************/

    /*** collisionResponse() ***/
    /* -performs collision detection and response */
    /*  sets new xyz  to position of the viewpoint after collision */
    /* -can also be used to implement gravity by updating y position of vp*/
    /* note that the world coordinates returned from getViewPosition()
    will be the negative value of the array indices */
    void collisionResponse() {
      float x, y, z;
      int xVal, yVal, zVal;
      float minClose = 0.16;
      float maxClose = 1 - minClose;
      getViewPosition(&x, &y, &z);

      x = (x * -1);
      y = (y * -1);
      z = (z * -1);

      xVal = floor(x);
      yVal = floor(y);
      zVal = floor(z);

      // printf("Position [%d, %d] = %d\n", xVal, zVal, yVal);

      if (xVal < 0 || xVal >= WORLDX || yVal < 0 || yVal >= WORLDY || zVal < 0 || zVal >= WORLDZ) {
        // printf("Trying to leave the world\n");
        goBack(x, y, z);
      }

      //  Checks if the view point is inside of a cube and goes back
      if (world[xVal][yVal][zVal] > 0) {
        // printf("Position: [%d, %d] = %d\n", xVal, zVal, yVal);
        // printf("Floats: [%f, %f] = %f\n", x, z, y);
        goBack(x, y, z);
      }

      /*  To avoid seing into cubes the program checks any position the player is extremely close to.
      I have set the cube cushions to 0.1. The player must always keep a distance from the cubes */
      if ((x-xVal > maxClose)) {
        if (world[xVal+1][yVal][zVal] > 0) {
          goBack(x, y, z);
        }
      }
      if ((x-xVal < minClose)) {
        if (world[xVal-1][yVal][zVal] > 0) {

          goBack(x, y, z);
        }
      }
      if ((y-yVal > maxClose)) {
        if (world[xVal][yVal+1][zVal] > 0) {
          goBack(x, y, z);
        }
      }
      if ((y-yVal < minClose)) {
        if (world[xVal][yVal-1][zVal] > 0) {
          goBack(x, y, z);
        }
      }
      if ((z-zVal > maxClose)) {
        if (world[xVal][yVal][zVal+1] > 0) {
          goBack(x, y, z);
        }
      }
      if ((z-zVal < minClose)) {
        if (world[xVal][yVal][zVal-1] > 0) {
          goBack(x, y, z);
        }
      }

    }

    void goBack(float x, float y, float z) {
      // printf("Position [%f, %f] = %f\n", (x* -1), (z* -1), (y* -1));
      getOldViewPosition(&x, &y, &z);
      setViewPosition(x, y, z);
      distX=distY=distZ=0;
      // printf("Position [%f, %f] = %f\n", (x* -1), (z* -1), (y* -1));
    }

    /*  optimizeGround()
        Description: Takes ground array which contains the y values on the x and z map and fills in all the holes for the world.
    */
    void optimizeGround(int groundArray[100][100]) {
      int xVal, zVal, yVal, minY;


      for(xVal=0; xVal<WORLDX; xVal++) {
         for(zVal=0; zVal<WORLDZ; zVal++) {
           yVal = groundArray[xVal][zVal];

           // anything smaller than the minimum y and the program will draw further down
           minY = yVal;

           // ignores any cube that is at position 1 or 0 as they cannot cause any holes in the world.
           if (yVal > 1) {

             // checks all of the neighboring cubes for their y value to see if it is smaller than the

             if (xVal != 0) {
               if (groundArray[xVal-1][zVal] < minY) {
                 minY = groundArray[xVal-1][zVal];
               }
             }
             if (xVal != 99) {
               if (groundArray[xVal+1][zVal] < minY) {
                 minY = groundArray[xVal+1][zVal];
               }
             }
             if (zVal != 0) {
               if (groundArray[xVal][zVal-1] < minY) {
                 minY = groundArray[xVal][zVal-1];
               }

               if (xVal != 0) {
                 if (groundArray[xVal-1][zVal-1] < minY) {
                   minY = groundArray[xVal-1][zVal-1];
                 }
               }
               if (xVal != 99) {
                 if (groundArray[xVal+1][zVal-1] < minY) {
                   minY = groundArray[xVal+1][zVal-1];
                 }
               }
             }
             if (zVal != 99) {
               if (groundArray[xVal][zVal+1] < minY) {
                 minY = groundArray[xVal][zVal+1];
               }
               if (xVal != 0) {
                 if (groundArray[xVal-1][zVal+1] < minY) {
                   minY = groundArray[xVal-1][zVal+1];
                 }
               }
               if (xVal != 99) {
                 if (groundArray[xVal+1][zVal+1] < minY) {
                   minY = groundArray[xVal+1][zVal+1];
                 }
               }
             }
           }
           // printf("[%d, %d] = %d\tmin= %d\n", xVal, zVal, yVal, minY);

           world[xVal][yVal][zVal] = 3;

           // keeps drawing cubes below as long as the minY val is smaller than the yVal-1
           while (minY < (yVal - 1)) {
             yVal--;
             world[xVal][yVal][zVal] = 3;
           }
         }
      }
    }


    void initializeHumans() {
        time_t t;
        srand((unsigned) time(&t));
        int xVal;
        int yVal;
        int zVal;

        for (int i = 0; i < HUMAN_NUMBER; i++) {
          xVal = ((rand() % (95 - 5 + 1)) + 5);
          yVal = ((rand() % (47 - 30 + 1)) + 30);
          zVal = ((rand() % (95 - 5 + 1)) + 5);
          humans[i][0] = xVal;
          humans[i][1] = yVal;
          humans[i][2] = zVal;

          // printf("Val %d [%d, %d] = %d\n", i, xVal, zVal, yVal);

          world[xVal][yVal][zVal] = 7;
          world[xVal][yVal+1][zVal] = 6;
          world[xVal][yVal+2][zVal] = 1;

        }
    }



    void drawPlayer(int mapWidth, int mapHeight, int mapPadding) {
      float xVal, yVal, zVal, xOrn, yOrn, zOrn;
      float xVector, zVector, rotx, roty;
      GLfloat blue[] = {0.0, 0.0, 0.5, 0.5};
      set2Dcolour(blue);

      int playerSize = (6 * displayMap);
      int dirMultiplier = 6;

      getViewPosition(&xVal, &yVal, &zVal);
      getViewOrientation(&xOrn, &yOrn, &zOrn);

      rotx = (xOrn / 180.0 * 3.141592);
      roty = (yOrn / 180.0 * 3.141592);

      xVector = xVal = fabs(xVal);
      zVector = zVal = fabs(zVal);

      // xVector += sin(roty) * dirMultiplier;
      // zVector -= cos(roty) * dirMultiplier;

      // xVector = xVector - xVal;
      // zVector = zVector - zVal;

      xVal = round((xVal / 100) * mapWidth) + mapPadding;
      zVal = round((zVal / 100) * mapHeight) + mapPadding;

      // xVector = round((xVector / 100) * mapWidth) + mapPadding;
      // zVector = round((zVector / 100) * mapHeight) + mapPadding;


      draw2Dtriangle((xVal+(playerSize)), (zVal-(playerSize/2)), (xVal), (zVal+playerSize), (xVal-(playerSize)), (zVal-(playerSize/2)));
    }

    void drawHumans(int mapWidth, int mapHeight, int mapPadding) {
      float xVal;
      float zVal;
      int humanSize = (3 * displayMap);
      GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
      set2Dcolour(green);

      for (int i = 0; i < HUMAN_NUMBER; i++) {
        xVal = humans[i][0];
        zVal = humans[i][2];

        xVal = round((xVal / 100) * mapWidth) + mapPadding;
        zVal = round((zVal / 100) * mapHeight) + mapPadding;

        draw2Dbox((xVal-humanSize), (zVal+humanSize), (xVal+humanSize), (zVal-humanSize));

      }

    }

    void drawTubes(int mapWidth, int mapHeight, int mapPadding) {
      float xStart;
      float yStart;
      float zStart;
      float xEnd;
      float yEnd;
      float zEnd;
      GLfloat purple[] = {0.5, 0.0, 0.5, 0.5};
      set2Dcolour(purple);

      for (int i = 0; i < TUBE_COUNT; i++) {
        if (tubeMovement[i] <=0) {
          /* hide the tube */
        }
        else{
          /* move the tube forward */
          xStart = tubeData[i][0]; // sx;
          zStart = tubeData[i][2]; // sz;
          xEnd = tubeData[i][3]; // ex;
          zEnd = tubeData[i][5]; // ez;

          xStart = round((xStart / 100) * mapWidth) + mapPadding;
          zStart = round((zStart / 100) * mapHeight) + mapPadding;
          xEnd = round((xEnd / 100) * mapWidth) + mapPadding;
          zEnd = round((zEnd / 100) * mapHeight) + mapPadding;


          draw2Dline(xStart, zStart, xEnd, zEnd, (2 * displayMap));
        }
      }
      // printf("\n");
    }

    void drawMap(int mapWidth, int mapHeight, int mapPadding) {
      GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
      GLfloat yellow[] = {0.5, 0.7, 0.0, 0.5};

      set2Dcolour(black);
      draw2Dbox(mapPadding, (mapHeight+mapPadding), (mapWidth+mapPadding), mapPadding);

      set2Dcolour(yellow);

      draw2Dline((mapPadding-1), (mapPadding-1), (mapWidth+mapPadding+1), (mapPadding-1), MAP_BORDER_SIZE);
      draw2Dline((mapWidth+mapPadding+1), (mapPadding-1), (mapWidth+mapPadding+1), (mapHeight+mapPadding+1), MAP_BORDER_SIZE);
      draw2Dline((mapWidth+mapPadding+1), (mapHeight+mapPadding+1), (mapPadding-1), (mapHeight+mapPadding+1), MAP_BORDER_SIZE);
      draw2Dline((mapPadding-1), (mapHeight+mapPadding+1), (mapPadding-1), (mapPadding-1), MAP_BORDER_SIZE);
    }


    /******* draw2D() *******/
    /* draws 2D shapes on screen */
    /* use the following functions: 			*/
    /*	draw2Dline(int, int, int, int, int);		*/
    /*	draw2Dbox(int, int, int, int);			*/
    /*	draw2Dtriangle(int, int, int, int, int, int);	*/
    /*	set2Dcolour(float []); 				*/
    /* colour must be set before other functions are called	*/
    void draw2D() {

      int mapWidth = 200;
      int mapHeight = 200;
      int reductionFactor = 5;
      int mapPadding = 1;

      if (testWorld) {
        /* draw some sample 2d shapes */
        if (displayMap == 1) {
          GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
          set2Dcolour(green);
          draw2Dline(0, 0, 500, 500, 15);
          draw2Dtriangle(0, 0, 200, 200, 0, 200);

          GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
          set2Dcolour(black);
          draw2Dbox(500, 380, 524, 388);
        }
      } else {

        GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
        GLfloat red[] = {0.5, 0.0, 0.0, 0.5};
        GLfloat black[] = {0.0, 0.0, 0.0, 0.5};

        // set2Dcolour(green);
        // draw2Dline(0, 0, 500, 500, 15);

        if (displayMap == 0) {
          /* no map */
        }
        else {
          if (displayMap == 1) {
            mapPadding = 2;
            mapWidth = (screenWidth / reductionFactor) - (mapPadding * 2);
            mapHeight = (screenHeight / reductionFactor) - (mapPadding * 2);
          }
          else if (displayMap == 2) {
            mapPadding = 25;
            mapWidth = screenWidth - (mapPadding * 2);
            mapHeight = screenHeight - (mapPadding * 2);

          }
          else{
            printf("Error: The display map integer is unrecognized %d\n", displayMap);
          }

          // draw player
          drawPlayer(mapWidth, mapHeight, mapPadding);
          // draw beam
          drawTubes(mapWidth, mapHeight, mapPadding);
          drawHumans(mapWidth, mapHeight, mapPadding);
          drawMap(mapWidth, mapHeight, mapPadding);
        }

      }
    }

    /*  updateHumans()
        Description: Updates the Humans y position
    */
    void updateHumans() {
      int xVal;
      int yVal;
      int zVal;

      for (int i = 0; i < HUMAN_NUMBER; i++) {
        xVal = humans[i][0];
        yVal = humans[i][1];
        zVal = humans[i][2];

        if (world[xVal][yVal-1][zVal] == 0) {
          world[xVal][yVal+2][zVal] = 0;

          // decrements Y value
          yVal--;

          world[xVal][yVal][zVal] = 7;
          world[xVal][yVal+1][zVal] = 6;
          world[xVal][yVal+2][zVal] = 1;

          // update humans Y location
          humans[i][1] = yVal;
        }
      }
    }

    bool tubeCollision(float xStart, float yStart, float zStart, float xEnd, float yEnd, float zEnd, int segments) {
      int x, y, z;

      xEnd = fabs(xEnd);
      yEnd = fabs(yEnd);
      zEnd = fabs(zEnd);

      float xDelta = (xEnd - xStart) / segments;
      float yDelta = (yEnd - yStart) / segments;
      float zDelta = (zEnd - zStart) / segments;

      // printf("Start: %f, %f, %f\n", xStart, yStart, zStart);
      // printf("End: %f, %f, %f\n", xEnd, yEnd, zEnd);
      // printf("Delta: %f, %f, %f\n", xDelta, yDelta, zDelta);


      for (int i = 0; i < segments; i++) {

        x = (int)floor(xStart);
        y = (int)floor(yStart);
        z = (int)floor(zStart);

        // printf("checking: %d %d %d\n", x, y, z);
        // printf("value: %d\n", world[x][y][z]);

        if (world[x][y][z] > 0) {
          if (world[x][y][z] == 1 || world[x][y][z] == 6 || world[x][y][z] == 7) {
            printf("Lazer hit human\n");
          }
          else{
            /* collision with the ground */
          }

          return true;
        }

        xStart = xStart + xDelta;
        yStart = yStart + yDelta;
        zStart = zStart + zDelta;

      }

      return false;
    }

    void updateTubes() {
      float xStart, xEnd;
      float yStart, yEnd;
      float zStart, zEnd;
      float xVector;
      float yVector;
      float zVector;
      float vectorMultiplier = 0.1;

      for (int i = 0; i < TUBE_COUNT; i++) {
        // printf("%d ", tubeMovement[i]);
        if (tubeMovement[i] <=0) {
          /* hide the tube */
          hideTube(i);
          tubeHit[i] = 0;
        }
        else{
          /* move the tube forward */
          xStart = tubeData[i][0]; // sx;
          yStart = tubeData[i][1]; // sy;
          zStart = tubeData[i][2]; // sz;
          xVector = tubeData[i][3] - xStart; // ex;
          yVector = tubeData[i][4] - yStart; // ey;
          zVector = tubeData[i][5] - zStart; // ez;

          xStart = xStart + (xVector * vectorMultiplier);
          yStart = yStart + (yVector * vectorMultiplier);
          zStart = zStart + (zVector * vectorMultiplier);

          xEnd = xStart + xVector;
          yEnd = yStart + yVector;
          zEnd = zStart + zVector;

          // do collision checking
          if (tubeHit[i] == 0) {
            if (tubeCollision((xEnd-(xVector * vectorMultiplier)), (yEnd-(yVector * vectorMultiplier)), (zEnd-(zVector * vectorMultiplier)), xEnd, yEnd, zEnd, SEGMENTS)) {
              // printf("TUBE COLLISION: %d\n", i);
              tubeHit[i] = 1;
            }
            else{
              // updates the new tube position
              tubeData[i][0] = xStart;
              tubeData[i][1] = yStart;
              tubeData[i][2] = zStart;
              tubeData[i][3] = xStart + xVector;
              tubeData[i][4] = yStart + yVector;
              tubeData[i][5] = zStart + zVector;
            }
          }

          tubeMovement[i]--;
        }
      }
      // printf("\n");
    }

    /*** update() ***/
    /* background process, it is called when there are no other events */
    /* -used to control animations and perform calculations while the  */
    /*  system is running */
    /* -gravity must also implemented here, duplicate collisionResponse */
    void update() {
      int i, j, k;
      float *la;
      float x, y, z;

      /* sample animation for the testworld, don't remove this code */
      /* demo of animating mobs */
      if (testWorld) {

        /* update old position so it contains the correct value */
        /* -otherwise view position is only correct after a key is */
        /*  pressed and keyboard() executes. */
        getViewPosition(&x, &y, &z);
        setOldViewPosition(x,y,z);

        /* sample of rotation and positioning of mob */
        /* coordinates for mob 0 */
        static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
        static float mob0ry = 0.0;
        static int increasingmob0 = 1;
        /* coordinates for mob 1 */
        static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
        static float mob1ry = 0.0;
        static int increasingmob1 = 1;
        /* counter for user defined colour changes */
        static int colourCount = 0;
        static GLfloat offset = 0.0;

        /* move mob 0 and rotate */
        /* set mob 0 position */
        setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

        /* move mob 0 in the x axis */
        if (increasingmob0 == 1)
        mob0x += 0.2;
        else
        mob0x -= 0.2;
        if (mob0x > 50) increasingmob0 = 0;
        if (mob0x < 30) increasingmob0 = 1;

        /* rotate mob 0 around the y axis */
        mob0ry += 1.0;
        if (mob0ry > 360.0) mob0ry -= 360.0;

        /* move mob 1 and rotate */
        setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

        /* move mob 1 in the z axis */
        /* when mob is moving away it is visible, when moving back it */
        /* is hidden */
        if (increasingmob1 == 1) {
          mob1z += 0.2;
          showMob(1);
        } else {
          mob1z -= 0.2;
          hideMob(1);
        }
        if (mob1z > 72) increasingmob1 = 0;
        if (mob1z < 52) increasingmob1 = 1;

        /* rotate mob 1 around the y axis */
        mob1ry += 1.0;
        if (mob1ry > 360.0) mob1ry -= 360.0;

        /* change user defined colour over time */
        if (colourCount == 1) offset += 0.05;
        else offset -= 0.01;
        if (offset >= 0.5) colourCount = 0;
        if (offset <= 0.0) colourCount = 1;
        setUserColour(9, 0.7, 0.3 + offset, 0.7, 1.0, 0.3, 0.15 + offset, 0.3, 1.0);

        /* sample tube creation  */
        /* draws a purple tube above the other sample objects */
        createTube(1, 45.0, 30.0, 45.0, 50.0, 30.0, 50.0, 6);

        /* end testworld animation */


      } else {

        static int gravityTick = 0;
        static bool isAccelerating = false;
        double updateTick = 0.08;
        float newX, newY, newZ;
        float oldX, oldY, oldZ;
        float accelerationMax = 2;
        float accelerationFactor = 0.01;
        float decelerationFactor = 0.97;
        float stoppingFactor = 0.003;

        newTime = clock();

        double time_taken = ((double)(newTime-oldTime))/CLOCKS_PER_SEC;
        updateTubes();

        getViewPosition(&newX, &newY, &newZ);
        getOldViewPosition(&oldX, &oldY, &oldZ);

        if (time_taken > updateTick) {
          // every couple ticks the system will update the humans position.
          if (gravityTick == 2) {
            gravityTick = 0;
            updateHumans();
          }
          gravityTick++;
          oldTime = newTime;

          if (keyboardPressed) {
            // printf("KeyPress\n");

            toggleKeyboardPress();
            isAccelerating = true;

          }
          else {
            // printf("No Key\n");
            isAccelerating = false;
            if (acceleration > 1) {
              distX = (newX - oldX) * decelerationFactor;
              distY = (newY - oldY) * decelerationFactor;
              distZ = (newZ - oldZ) * decelerationFactor;
              acceleration = 1;
            }
          }
        }

        /* outside the update tick */

        if (isAccelerating) {
          if (acceleration < accelerationMax) {
            acceleration = acceleration + accelerationFactor;
          }
          // printf("Moving\n");
          distX = (newX - oldX);
          distY = (newY - oldY);
          distZ = (newZ - oldZ);

          oldX = newX;
          oldY = newY;
          oldZ = newZ;

          newX = newX + distX;
          newY = newY + distY;
          newZ = newZ + distZ;

          setOldViewPosition(oldX, oldY, oldZ);
          setViewPosition(newX, newY, newZ);
          collisionResponse();

        }
        else {
          // if it is still moving in a significatly viewable distance then it will update the view position
          if (fabs(distX) > stoppingFactor || fabs(distY) > stoppingFactor || fabs(distZ) > stoppingFactor) {
            // printf("Deccelerating\n");
            distX = (newX - oldX) * decelerationFactor;
            distY = (newY - oldY) * decelerationFactor;
            distZ = (newZ - oldZ) * decelerationFactor;

            oldX = newX;
            oldY = newY;
            oldZ = newZ;

            newX = newX + distX;
            newY = newY + distY;
            newZ = newZ + distZ;

            setOldViewPosition(oldX, oldY, oldZ);
            setViewPosition(newX, newY, newZ);
            collisionResponse();
          }
          else {
            // printf("Stopped\n");
            distX = 0;
            distY = 0;
            distZ = 0;
          }
        }
        // printf("Accel: %f\tDist: %f %f %f\n", acceleration, distX, distY, distZ);
      }
    }


    /* called by GLUT when a mouse button is pressed or released */
    /* -button indicates which button was pressed or released */
    /* -state indicates a button down or button up event */
    /* -x,y are the screen coordinates when the mouse is pressed or */
    /*  released */
    void mouse(int button, int state, int x, int y) {
      static int beamNumber = 0;
      float xStart, yStart, zStart;
      float xEnd, yEnd, zEnd;
      float xOrn, yOrn, zOrn;
      float rotx, roty;

      if (button == GLUT_LEFT_BUTTON){
        // printf("left button - ");
        if (state == GLUT_UP){
          // printf("up - ");
          getViewPosition(&xStart, &yStart, &zStart);
          getViewOrientation(&xOrn, &yOrn, &zOrn);

          rotx = (xOrn / 180.0 * 3.141592);
          roty = (yOrn / 180.0 * 3.141592);

          xEnd = xStart = fabs(xStart);
          yEnd = yStart = fabs(yStart);
          zEnd = zStart = fabs(zStart);
          xEnd += sin(roty) * TUBE_SIZE;
          yEnd -= sin(rotx) * TUBE_SIZE;
          zEnd -= cos(roty) * TUBE_SIZE;

          // printf("Start: %f, %f, %f\n", xStart, yStart, zStart);
          // printf("End: %f, %f, %f\n", xEnd, yEnd, zEnd);
          // printf("Orn: %f, %f\n", xOrn, yOrn);

          if (tubeCollision(xStart, yStart, zStart, xEnd, yEnd, zEnd, (SEGMENTS*7))) {
            printf("Can't fire lazer. Too close to block.\n");
            /* tube collision has occured so no ray is set */
          }
          else{
            // once it gets to the tube maximum it begins dranwing at the beginning of the array
            if (beamNumber >= TUBE_COUNT) {
              beamNumber = 0;
            }

            createTube(beamNumber, xStart, yStart, zStart, xEnd, yEnd, zEnd, 6);
            tubeMovement[beamNumber] = TUBE_LIFE; //sets moves the beam can make
            beamNumber = beamNumber + 1;
          }
        }
      }
    }



    int main(int argc, char** argv)
    {
      int i, j, k;
      /* initialize the graphics system */
      graphicsInit(&argc, argv);

      /* the first part of this if statement builds a sample */
      /* world which will be used for testing */
      /* DO NOT remove this code. */
      /* Put your code in the else statment below */
      /* The testworld is only guaranteed to work with a world of
      with dimensions of 100,50,100. */
      if (testWorld == 1) {
        /* initialize world to empty */
        for(i=0; i<WORLDX; i++)
        for(j=0; j<WORLDY; j++)
        for(k=0; k<WORLDZ; k++)
        world[i][j][k] = 0;

        /* some sample objects */
        /* build a red platform */
        for(i=0; i<WORLDX; i++) {
          for(j=0; j<WORLDZ; j++) {
            world[i][24][j] = 3;
          }
        }
        /* create some green and blue cubes */
        world[50][25][50] = 1;
        world[49][25][50] = 1;
        world[49][26][50] = 1;
        world[52][25][52] = 2;
        world[52][26][52] = 2;

        /* create user defined colour and draw cube */
        setUserColour(9, 0.7, 0.3, 0.7, 1.0, 0.3, 0.15, 0.3, 1.0);
        world[54][25][50] = 9;


        /* blue box shows xy bounds of the world */
        for(i=0; i<WORLDX-1; i++) {
          world[i][25][0] = 2;
          world[i][25][WORLDZ-1] = 2;
        }
        for(i=0; i<WORLDZ-1; i++) {
          world[0][25][i] = 2;
          world[WORLDX-1][25][i] = 2;
        }

        /* create two sample mobs */
        /* these are animated in the update() function */
        createMob(0, 50.0, 25.0, 52.0, 0.0);
        createMob(1, 50.0, 25.0, 52.0, 0.0);

        /* create sample player */
        createPlayer(0, 52.0, 27.0, 52.0, 0.0);
      } else {

        /* initialize world to empty */
        for(i=0; i<WORLDX; i++)
        for(j=0; j<WORLDY; j++)
        for(k=0; k<WORLDZ; k++)
        world[i][j][k] = 0;

        FILE *groundFile;
        char buf[1000];
        char newNumber[5];

        int groundArray[100][100];

        int xVal = 0;
        int zVal = 0;
        int yVal = 0;
        int maxY = 0;
        int maxX = 0;
        int maxZ = 0;
        int reductionFactor = 45;
        bool addingWord = false;
        int charCount= 0;
        int numberCount= 0;


        groundFile =fopen("ground.pgm","r");
        if (!groundFile) return 1;

        while (fgets(buf,1000, groundFile)!=NULL) {
          if (buf[0] == '#' || buf[0] == 'P') {
            /* ignore */
          }
          else {
            strcpy(newNumber, "");
            for (i = 0; i < strlen(buf); i++) {
              if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == '\r') {
                if (addingWord) {
                  // finishes the adding to the word
                  newNumber[charCount] = '\0';
                  addingWord = false;
                  // counts a number
                  numberCount++;

                  if (numberCount > 3) {

                    yVal = atoi(newNumber) / reductionFactor;

                    // printf("[%d, %d] = %d\n", xVal, zVal, yVal);

                    groundArray[xVal][zVal] = yVal;


                    xVal++;
                    /* code */
                    if (xVal == maxX) {
                      xVal = 0;
                      zVal++;
                    }
                  }
                  else { // first three numbers are the max values
                    if (numberCount == 1) {
                      maxX = atoi(newNumber);
                    } else if (numberCount == 2) {
                      maxZ = atoi(newNumber);
                    } else {
                      maxY = atoi(newNumber);
                    }
                  }

                  // if (numberCount == 350) {
                  //   maxY = maxY;
                  //   printf("\nMAx Y: %d\n\n", maxY);
                  //   return 0;
                  // }

                  // resets the newNumber string
                  strcpy(newNumber, "");
                  charCount = 0;
                }
              }
              else{
                addingWord = true;
                newNumber[charCount] = buf[i];
                charCount = charCount + 1;
              }
            }
          }
        }
        createPlayer(0, 52.0, 27.0, 52.0, 0.0);

        fclose(groundFile);

        optimizeGround(groundArray);

        initializeHumans();
      }

      setViewPosition(-50,-20,-50);
      // setOldViewPosition(-51,-20,-52);

      /* starts the graphics processing loop */
      /* code after this will not run until the program exits */
      glutMainLoop();
      return 0;
    }
