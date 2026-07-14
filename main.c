#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define W_WIDTH 600
#define W_HEIGHT 550
#define W_TITLE "Pong Game"
#define START_STRING "Press SPACE to begin..."
#define INITIAL_POSITION 50
#define MAX_BOUNCE_SPEED 450

const float ballRadius = 12.0f;
const float ballSpeed = 400.0f;
const float paddleSpeed = 450.0f;
const float paddleHeight = 60.0f;
const float paddleWidth = 10.0f;

typedef struct
{
    Vector2 position;
    float angle;
    float magnitude;
    float radius;
    int spawnDirection;
} Ball;

typedef struct
{
    Vector2 position;
    Vector2 size;
    struct
    {
        int keyUp;
        int keyDown;
    } inputs;
    float speed;
    int score;
    int aiEnabled;
} Paddle;

typedef enum
{
    MENU,
    LOADING,
    PLAYING,
    GAME_OVER
} GameState;

float clamp(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;

    return value;
}

void drawBall(Ball b)
{
    DrawCircleV(b.position, b.radius, WHITE);
}

void drawPaddle(Paddle p)
{
    DrawRectangleV(p.position, p.size, WHITE);
}

int shouldFollowBallTop(Paddle *p, Ball *b)
{
    int ballY = b->position.y;
    int paddleY = p->position.y + (p->size.y / 2);

    return b->magnitude > 0 && ballY < paddleY;
}

int shouldFollowBallBottom(Paddle *p, Ball *b)
{
    int ballY = b->position.y;
    int paddleY = p->position.y + (p->size.y / 2);

    return b->magnitude > 0 && ballY > paddleY;
}

int shouldGoToCenterFromTop(Paddle *p)
{
    int paddleY = p->position.y + (p->size.y / 2);

    return paddleY < (W_HEIGHT / 2);
}

int shouldGoToCenterFromBottom(Paddle *p)
{
    int paddleY = p->position.y + (p->size.y / 2);

    return paddleY > (W_HEIGHT / 2);
}

void handleMovePaddleUp(Paddle *p, Ball *b, float dt)
{
    if (!IsKeyDown(p->inputs.keyUp) && !p->aiEnabled)
        return;

    if (p->aiEnabled && (!shouldFollowBallTop(p, b) && (shouldGoToCenterFromTop(p) || shouldFollowBallBottom(p, b))))
        return;

    float desiredPosition = (p->position.y - p->speed * dt);

    p->position.y = clamp(desiredPosition, 0.0f, (W_HEIGHT - p->size.y));

    return;
}

void handleMovePaddleDown(Paddle *p, Ball *b, float dt)
{
    if (!IsKeyDown(p->inputs.keyDown) && !p->aiEnabled)
        return;
    if (p->aiEnabled && (!shouldFollowBallBottom(p, b) && (shouldGoToCenterFromBottom(p) || shouldFollowBallTop(p, b))))
        return;

    float desiredPosition = (p->position.y + p->speed * dt);

    p->position.y = clamp(desiredPosition, 0.0f, (W_HEIGHT - p->size.y));

    return;
}

void movePaddle(Paddle *p, Ball *b, float dt)
{
    handleMovePaddleDown(p, b, dt);
    handleMovePaddleUp(p, b, dt);
}

void hitPaddle(Ball *b, Paddle *p, Sound *s)
{
    Vector2 x = (Vector2){p->position.x + (p->size.x / 2), p->position.y + (p->size.y / 2)};
    Vector2 y = b->position;

    float distX = fabsf(x.x - y.x);
    float distY = fabsf(x.y - y.y);

    if ((distX < (p->size.x / 2) + b->radius) && (distY < (p->size.y / 2) + b->radius))
    {
        float relative = (y.y - x.y) / (p->size.y / 2);
        relative = clamp(relative, -1.0f, 1.0f);

        int shouldInvert = 1;

        float incomingAngle = b->angle / DEG2RAD;
        float incomingMagnitude = b->magnitude;

        b->angle = relative * 60;
        b->angle *= DEG2RAD;

        if (b->magnitude < 0)
        {
            b->magnitude *= -1.0;
            b->magnitude = ballSpeed + fabsf(relative) * MAX_BOUNCE_SPEED;
            if (relative < 0)
            {
                b->angle = -fabsf(b->angle);
            }
            else
            {
                b->angle = fabsf(b->angle);
            }
        }
        else
        {
            b->magnitude *= -1.0;
            b->magnitude = -(ballSpeed + fabsf(relative) * MAX_BOUNCE_SPEED);
            if (relative < 0)
            {
                b->angle = fabsf(b->angle);
            }
            else
            {
                b->angle = -fabsf(b->angle);
            }
        }

        PlaySound(*s);
    }
}

int hitTop(Ball *b)
{
    return b->position.y - b->radius <= 0;
}

int hitBottom(Ball *b)
{
    return b->position.y + b->radius >= W_HEIGHT;
}

void drawScores(Paddle *lp, Paddle *rp)
{
    DrawText(TextFormat("%d | %d", lp->score, rp->score), W_WIDTH / 2, W_HEIGHT / 2, 16, WHITE);
}

void spawnBall(Ball *b)
{
    float randomAngle = (float)GetRandomValue(-30, 30);
    float randomPosition = (float)GetRandomValue(60, W_HEIGHT - 60);

    b->angle = randomAngle * DEG2RAD;
    b->position.y = randomPosition;
    b->position.x = W_WIDTH / 2;

    if (b->spawnDirection == 0)
    {
        b->magnitude = ballSpeed;
        b->spawnDirection = 1;
    }
    else
    {
        b->magnitude = -ballSpeed;
        b->spawnDirection = 0;
    }
}

void initializeGame(Paddle *lp, Paddle *rp, Ball *b, Vector2 ballSpeed)
{
    spawnBall(b);

    lp->position.x = INITIAL_POSITION;
    lp->position.y = (W_HEIGHT / 2) - (lp->size.y / 2);
    lp->inputs.keyUp = KEY_W;
    lp->inputs.keyDown = KEY_S;
    lp->score = 0;

    rp->position.x = W_WIDTH - INITIAL_POSITION - rp->size.x;
    rp->position.y = (W_HEIGHT / 2) - (rp->size.y / 2);
    rp->inputs.keyUp = KEY_UP;
    rp->inputs.keyDown = KEY_DOWN;
    rp->score = 0;
}

int isBallOutOfBoundsLeft(Ball *b)
{
    return (b->position.x - b->radius <= 0);
}

int isBallOutOfBoundsRight(Ball *b)
{
    return (b->position.x + b->radius >= W_WIDTH);
}

void moveBall(Ball *b, float dt)
{
    b->position.x += (b->magnitude * cosf(b->angle)) * dt;
    b->position.y += (b->magnitude * sinf(b->angle)) * dt;
}

int main(void)
{
    InitWindow(W_WIDTH, W_HEIGHT, W_TITLE);
    InitAudioDevice();
    SetTargetFPS(60);

    Sound sfxHitPaddle = LoadSound("hit_paddle.ogg");
    Sound sfxHitBarrier = LoadSound("hit_wall.ogg");
    Sound sfxScorePoint = LoadSound("score_point.ogg");

    const int textSize = MeasureText(W_TITLE, 32);
    const int textSize2 = MeasureText(START_STRING, 18);
    const int textSize3 = MeasureText("Game over, Left Player won. Press space to play again.", 18);
    const int textSize4 = MeasureText("3", 18);

    Vector2 ballSpeed = {300.0f, 300.0f};

    Ball ball;
    ball.radius = ballRadius;
    ball.spawnDirection = 0;

    Paddle leftPaddle;
    leftPaddle.size.x = paddleWidth;
    leftPaddle.size.y = paddleHeight;
    leftPaddle.speed = paddleSpeed;
    leftPaddle.aiEnabled = 0;

    Paddle rightPaddle;
    rightPaddle.size.x = paddleWidth;
    rightPaddle.size.y = paddleHeight;
    rightPaddle.speed = paddleSpeed;
    rightPaddle.aiEnabled = 1;

    double timerStart = 0.0f;
    double remaining = 0.0f;
    int isSlowMotion = 0;

    GameState state = MENU;

    while (!WindowShouldClose())
    {
        float deltatime = GetFrameTime();
        float ballDt = deltatime;
        switch (state)
        {
        case MENU:
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                state = LOADING;
                timerStart = GetTime();
            }
            break;
        }
        case LOADING:
        {
            remaining = 0.0f - (GetTime() - timerStart);

            if (remaining < 0.0f)
            {
                initializeGame(&leftPaddle, &rightPaddle, &ball, ballSpeed);
                state = PLAYING;
            }
            break;
        }
        case PLAYING:
        {
            if (leftPaddle.score >= 10 || rightPaddle.score >= 10)
                state = GAME_OVER;

            if (IsKeyPressed(KEY_Q))
            {
                isSlowMotion = isSlowMotion ? 0 : 1;
            }

            if (isSlowMotion)
            {
                ballDt *= 0.05f;
            }

            moveBall(&ball, ballDt);
            movePaddle(&leftPaddle, &ball, deltatime);
            movePaddle(&rightPaddle, &ball, deltatime);

            hitPaddle(&ball, &leftPaddle, &sfxHitPaddle);
            hitPaddle(&ball, &rightPaddle, &sfxHitPaddle);

            if (hitTop(&ball) || hitBottom(&ball))
            {
                ball.angle *= -1.0f;
                PlaySound(sfxHitBarrier);
            }

            if (isBallOutOfBoundsLeft(&ball))
            {
                rightPaddle.score++;
                PlaySound(sfxScorePoint);
                spawnBall(&ball);
            }

            if (isBallOutOfBoundsRight(&ball))
            {
                leftPaddle.score++;
                PlaySound(sfxScorePoint);
                spawnBall(&ball);
            }

            break;
        }
        case GAME_OVER:
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                state = LOADING;
                timerStart = GetTime();
            }
            break;
        }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(W_TITLE, (W_WIDTH - textSize) / 2, 10, 32, WHITE);

        switch (state)
        {
        case MENU:
        {
            DrawText(START_STRING, (W_WIDTH - textSize2) / 2, W_HEIGHT / 2, 18, WHITE);
            break;
        }
        case LOADING:
        {
            DrawText(TextFormat("%d", (int)remaining), (W_WIDTH - textSize4) / 2, W_HEIGHT / 2, 32, WHITE);
            break;
        }
        case PLAYING:
        {
            drawBall(ball);
            drawPaddle(leftPaddle);
            drawPaddle(rightPaddle);

            drawScores(&leftPaddle, &rightPaddle);
            break;
        }
        case GAME_OVER:
            DrawText(TextFormat("Game over, %s won. Press space to play again.", leftPaddle.score > rightPaddle.score ? "Left Player" : "Right Player"), (W_WIDTH - textSize3) / 2, W_HEIGHT / 2, 18, WHITE);
            break;
        }

        // DrawLineEx((Vector2){INITIAL_POSITION - (paddleWidth + 5.0f), 0}, (Vector2){INITIAL_POSITION - (paddleWidth + 5.0f), W_HEIGHT}, 1, RED);
        // DrawLineEx((Vector2){(W_WIDTH - INITIAL_POSITION + (paddleWidth + 5.0f)), 0}, (Vector2){(W_WIDTH - INITIAL_POSITION + (paddleWidth + 5.0f)), W_HEIGHT}, 1, RED);
        // DrawLineEx((Vector2){0, W_HEIGHT / 2}, (Vector2){W_WIDTH, W_HEIGHT / 2}, 1, RED);
        // DrawLineEx((Vector2){W_WIDTH / 2, 0}, (Vector2){W_WIDTH / 2, W_HEIGHT}, 1, RED);

        EndDrawing();
    }

    UnloadSound(sfxHitPaddle);
    UnloadSound(sfxHitBarrier);
    UnloadSound(sfxScorePoint);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}