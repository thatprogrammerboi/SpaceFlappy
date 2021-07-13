#include <Tinyfont.h>
#include <Arduboy2.h>
#include <ArduboyTones.h>

#include "sprites.h"
#include "sfx.h"

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

const int gravity = 1;

enum gameState {
  MENU_STATE,
  GAME_STATE,
  GAME_OVER_STATE,
  GAME_START_STATE
} gameState;

int shipX = 8;
int shipY = 32;
int yv = 0;
int animTimer = 0;
int delayTimer = 0;
int background1X = 0;
int background2X = 127;
int lives = 3;
bool invincible = false;
int invincibleTimer = 0;
int score = 0;

struct Bullets {
  int x = 0;
  int y = 0;
  bool fired = false;
};

struct Enemies {
  int x = 0;
  int y = 0;
  bool onscreen = false;
};

struct Explosion {
  int x = 0;
  int y = 0;
  int frame = 0;
  bool exploding = false;
};

Bullets bullet[10];
Enemies enemy[10];
Explosion explosion[10];

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(60);
  //Serial.begin(9600);
}

void loop() {
  //Code for display
  if (!(arduboy.nextFrameDEV())) {
    return;
  };

  if (arduboy.everyXFrames(10)) {
    if (animTimer == 0) {
      animTimer++;
    } else {
      animTimer = 0;
    };
  };
  arduboy.clear();

  switch (gameState) {
    case (MENU_STATE):
      arduboy.pollButtons();
      if (background1X > -127 && arduboy.everyXFrames(3)) {
        background1X = background1X - 1;
      } else if (background1X <= -127 && background2X < 0) {
        background1X = 127;
      };
      if (background2X > -127 && arduboy.everyXFrames(3)) {
        background2X = background2X - 1;
      } else if (background2X <= -127 && background1X < 0) {
        background2X = 127;
      };
      if (arduboy.everyXFrames(2)) {
        arduboy.drawBitmap(background1X, 0, stars, 128, 64);
        arduboy.drawBitmap(background2X, 0, stars, 128, 64);
      };

      arduboy.setCursor(30, 15);
      arduboy.print(F("SPACE FLAPPY"));
      arduboy.setCursor(45, 45);
      if (arduboy.everyXFrames(2)) {
        arduboy.print(F("Press "));
        arduboy.drawBitmap(77, 45, aButton, 8, 8);
      }
      if (arduboy.justPressed(A_BUTTON)) {
        arduboy.initRandomSeed();
        lives = 3;
        invincible = false;
        invincibleTimer = 0;
        for (int itemNum = 0; itemNum < 10; itemNum++) {
          bullet[itemNum].x = 0;
          bullet[itemNum].y = 0;
          bullet[itemNum].fired = false;
          enemy[itemNum].x = 0;
          enemy[itemNum].y = 0;
          enemy[itemNum].onscreen = false;
          explosion[itemNum].x = 0;
          explosion[itemNum].y = 0;
          explosion[itemNum].frame = 0;
          explosion[itemNum].exploding = false;
        };
        score = 0;
        yv = 0;
        shipY = 32;
        gameState = GAME_START_STATE;
        sound.tones(gameStart);
      };
      break;

    case (GAME_START_STATE):
      arduboy.pollButtons();
      //Background movement and drawing
      if (background1X > -127) {
        background1X = background1X - 1;
      } else if (background1X <= -127 && background2X < 0) {
        background1X = 127;
      };
      if (background2X > -127) {
        background2X = background2X - 1;
      } else if (background2X <= -127 && background1X < 0) {
        background2X = 127;
      };

      if (arduboy.everyXFrames(2)) {
        arduboy.drawBitmap(background1X, 0, stars, 128, 64);
        arduboy.drawBitmap(background2X, 0, stars, 128, 64);
      };

      if (animTimer == 0) {
        arduboy.drawBitmap(shipX, shipY, ship1, 14, 6);
      } else if (animTimer == 1) {
        arduboy.drawBitmap(shipX, shipY, ship2, 14, 6);
      }

      arduboy.setCursor(50, 30);
      if (arduboy.everyXFrames(2)) {
        arduboy.print(F("Ready?"));
      }

      delayTimer++;

      if (delayTimer > 120) {
        gameState = GAME_STATE;
        delayTimer = 0;
      }

      break;

    case (GAME_STATE):
      arduboy.pollButtons();
      //Player ship control and movement
      if (arduboy.justPressed(UP_BUTTON) && shipY > 1) {
        yv = -2;
      } else {
        if (shipY < 57) {
          if (arduboy.everyXFrames(10) && yv < 3) {
            yv = yv + gravity;
          };
          if (yv < 10) {
            shipY = shipY + yv;
          };
        };
      };

      if (shipY < 1) {
        yv = 0;
        shipY = 1;
      } else if (shipY > 57) {
        shipY = 57;
        yv = 0;
      };

      if (arduboy.everyXFrames(5)) {
        shipY = shipY + yv;
      };

      if (arduboy.everyXFrames(60)) {
        score++;
      };

      //Weapon control and movement
      if (arduboy.justPressed(B_BUTTON)) {
        for (int bulletNum = 0; bulletNum < 10; bulletNum++) {
          if (bullet[bulletNum].fired == false) {
            bullet[bulletNum].x = shipX + 5;
            bullet[bulletNum].y = shipY + 3;
            bullet[bulletNum].fired = true;
            sound.tone(450, 10);
            break;
          };
        };
      };

      for (int bulletNum = 0; bulletNum < 10; bulletNum++) {
        if (bullet[bulletNum].x < 127 && bullet[bulletNum].fired) {
          bullet[bulletNum].x = bullet[bulletNum].x + 3;
        } else {
          bullet[bulletNum].fired = false;
        };
      };

      for (int bulletNum = 0; bulletNum < 10; bulletNum++) {
        for (int enemyNum = 0; enemyNum < 10; enemyNum++) {
          if (arduboy.collide(Rect(bullet[bulletNum].x, bullet[bulletNum].y, 3, 1), Rect(enemy[enemyNum].x - 2, enemy[enemyNum].y - 2, 10, 10)) && bullet[bulletNum].fired) {
            for (int explosionNum = 0; explosionNum < 10; explosionNum++) {
              if (explosion[explosionNum].exploding == false) {
                explosion[explosionNum].exploding = true;
                explosion[explosionNum].x = enemy[enemyNum].x - 2;
                explosion[explosionNum].y = enemy[enemyNum].y - 2;
                explosion[explosionNum].frame = 0;
                break;
              };
            };
            enemy[enemyNum].onscreen = false;
            enemy[enemyNum].x = 0;
            enemy[enemyNum].y = 0;
            bullet[bulletNum].fired = false;
            sound.tones(enemyDeath);
            score = score + 20; //Add to score
          };
        };
      };


      //Background movement and drawing
      if (background1X > -127) {
        background1X = background1X - 1;
      } else if (background1X <= -127 && background2X < 0) {
        background1X = 127;
      };
      if (background2X > -127) {
        background2X = background2X - 1;
      } else if (background2X <= -127 && background1X < 0) {
        background2X = 127;
      };
      if (arduboy.everyXFrames(2)) {
        arduboy.drawBitmap(background1X, 0, stars, 128, 64);
        arduboy.drawBitmap(background2X, 0, stars, 128, 64);
      };


      //Enemy ship spawning and movement
      if (arduboy.everyXFrames(20)) {
        for (int enemyNum = 0; enemyNum < 10; enemyNum++) {
          if (!enemy[enemyNum].onscreen) {
            enemy[enemyNum].x = 127;
            enemy[enemyNum].y = random(1, 55);
            enemy[enemyNum].onscreen = true;
            break;
          }
        }
      }

      for (int enemyNum = 0; enemyNum < 10; enemyNum++) {
        if (enemy[enemyNum].x > -8 && enemy[enemyNum].onscreen) {
          enemy[enemyNum].x = enemy[enemyNum].x - 2;
          if (animTimer == 0) {
            arduboy.drawBitmap(enemy[enemyNum].x, enemy[enemyNum].y, enemy1, 8, 8);
          } else if (animTimer == 1) {
            arduboy.drawBitmap(enemy[enemyNum].x, enemy[enemyNum].y, enemy2, 8, 8);
          }
        } else {
          enemy[enemyNum].onscreen = false;
        }
      }


      //Player ship drawing and collision checking
      for (int enemyNum = 0; enemyNum < 10; enemyNum++) {
        if (arduboy.collide(Rect(shipX, shipY, 14, 6), Rect(enemy[enemyNum].x, enemy[enemyNum].y, 8, 8)) && !invincible) {
          invincibleTimer = 120; //2 seconds @60fps
          invincible = true;
          if (lives > 1) {
            lives = lives - 1;
            sound.tones(playerDeath);
          } else {
            gameState = GAME_OVER_STATE;
            sound.tones(gameOver);
          }
        }
      }

      if (invincibleTimer > 0 && invincible) {
        invincibleTimer = invincibleTimer - 1;
      } else {
        invincible = false;
      }

      if (invincible) {
        if (animTimer == 0 && arduboy.everyXFrames(2)) {
          arduboy.drawBitmap(shipX, shipY, ship1, 14, 6);
        } else if (animTimer == 1 && arduboy.everyXFrames(2)) {
          arduboy.drawBitmap(shipX, shipY, ship2, 14, 6);
        };
      } else {
        if (animTimer == 0) {
          arduboy.drawBitmap(shipX, shipY, ship1, 14, 6);
        } else if (animTimer == 1) {
          arduboy.drawBitmap(shipX, shipY, ship2, 14, 6);
        }
      }

      if (invincibleTimer > 117 && gameState != GAME_OVER_STATE) {
        arduboy.fillScreen(WHITE);
      };


      //Draw the bullets
      for (int bulletNum = 0; bulletNum < 10; bulletNum++) {
        if (bullet[bulletNum].fired == true) {
          arduboy.drawFastHLine(bullet[bulletNum].x, bullet[bulletNum].y, 3);
        };
      };


      //Draw the explosions
      for (int explosionNum = 0; explosionNum < 10; explosionNum++) {
        if (explosion[explosionNum].exploding == true) {
          switch (explosion[explosionNum].frame) {
            case 0:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion1, 12, 12);
              break;

            case 1:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion2, 12, 12);
              break;

            case 2:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion3, 12, 12);
              break;

            case 3:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion4, 12, 12);
              break;

            case 4:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion5, 12, 12);
              break;

            case 5:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion6, 12, 12);
              break;

            case 6:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion7, 12, 12);
              break;

            case 7:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion8, 12, 12);
              break;

            case 8:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion9, 12, 12);
              break;

            case 9:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion10, 12, 12);
              break;

            case 10:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion11, 12, 12);
              break;

            case 11:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion12, 12, 12);
              break;

            case 12:
              arduboy.drawBitmap(explosion[explosionNum].x, explosion[explosionNum].y, explosion13, 12, 12);
              break;

            default:
              arduboy.setCursor(explosion[explosionNum].x, explosion[explosionNum].y);
              arduboy.print(F("ERROR"));
              break;
          };
        };
      };
      if (arduboy.everyXFrames(2)) {
        for (int explosionNum = 0; explosionNum < 10; explosionNum++) {
          if (explosion[explosionNum].frame < 12) {
            explosion[explosionNum].frame++;
          } else {
            explosion[explosionNum].exploding = false;
          };
        };
      };
      tinyfont.setCursor(85, 58);
      tinyfont.print(F("LIFE"));
      tinyfont.setCursor(110, 58);
      switch (lives) {
        case 3:
          tinyfont.print(F("iii"));
          break;
        case 2:
          tinyfont.print(F("ii"));
          break;
        case 1:
          tinyfont.print(F("i"));
          break;
        default:
          tinyfont.print(F("Error"));
          break;
      }
      tinyfont.setCursor(2, 58);
      tinyfont.print(F("SCORE"));
      tinyfont.setCursor(30, 58);
      tinyfont.print(score);
      break;

    case GAME_OVER_STATE:
      arduboy.pollButtons();
      if (arduboy.everyXFrames(3)) {
        arduboy.drawBitmap(background1X, 0, stars, 128, 64);
        arduboy.drawBitmap(background2X, 0, stars, 128, 64);
      };

      arduboy.setCursor(39, 10);
      arduboy.print(F("GAME OVER"));

      arduboy.drawBitmap(20, 50, left, 8, 8);
      arduboy.setCursor(32, 50);
      arduboy.print(F("Retry"));

      arduboy.drawBitmap(71, 50, right, 8, 8);
      arduboy.setCursor(82, 50);
      arduboy.print(F("Menu"));

      if (arduboy.justPressed(LEFT_BUTTON)) {
        lives = 3;
        invincible = false;
        invincibleTimer = 0;
        for (int itemNum = 0; itemNum < 10; itemNum++) {
          bullet[itemNum].x = 0;
          bullet[itemNum].y = 0;
          bullet[itemNum].fired = false;
          enemy[itemNum].x = 0;
          enemy[itemNum].y = 0;
          enemy[itemNum].onscreen = false;
          explosion[itemNum].x = 0;
          explosion[itemNum].y = 0;
          explosion[itemNum].frame = 0;
          explosion[itemNum].exploding = false;
        };
        score = 0;
        yv = 0;
        shipY = 32;
        sound.tones(gameStart);
        gameState = GAME_START_STATE;
      } else if (arduboy.justPressed(RIGHT_BUTTON)) {
        gameState = MENU_STATE;
      };
      break;
  };
  //Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
  arduboy.display();
};
