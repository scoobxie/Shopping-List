#include <raylib.h>
#include <cstring>
#include <iostream>
#include <fstream>

#define MAX_LINES 200
#define MAX_CHARS 76 * MAX_LINES  // plenty of space

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Shopping List");
    SetTargetFPS(60);

    // Load font and textures
    Font font = LoadFont("font/OriginalSurfer-Regular.ttf");
    Image icon = LoadImage("Slices1.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    Texture2D strawberry = LoadTexture("Strawberry1.png");
    Texture2D leaves = LoadTexture("Leaves1.png");
    Texture2D heartEmpty = LoadTexture("heart1.png");
    Texture2D heartFilled = LoadTexture("heart2.png");

    // UI rectangle
    Rectangle WritingSpace = { 50, 100, 700, 500 };

    // Text buffer
    char myText[MAX_CHARS] = { 0 };
    Vector2 checkboxPositions[MAX_LINES];
    bool checkboxStates[MAX_LINES] = { false };
    int checkboxCount = 0;
    int lineLengths[MAX_LINES] = { 0 };
    int rowCount = 0;

    bool writingMode = false;
    int framesCounter = 0;
    float backspaceTimer = 0.0f;
    bool holdingBackspace = false;

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        bool mouseOnText = CheckCollisionPointRec(mouse, WritingSpace);

        if (mouseOnText) {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                writingMode = !writingMode;
            }
        }
        else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        if (writingMode) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && strlen(myText) + 1 < MAX_CHARS) {
                    int len = strlen(myText);
                    myText[len] = (char)key;
                    myText[len + 1] = '\0';
                    lineLengths[rowCount]++;

                    // Auto wrap line if too long
                    if (lineLengths[rowCount] >= 61) {
                        strcat(myText, "\n");
                        rowCount++;
                        lineLengths[rowCount] = 0;
                    }
                }
                key = GetCharPressed();
            }

            // Enter key = new line
            if (IsKeyPressed(KEY_ENTER)) {
                // Measure position of this finished line
                int completedLine = rowCount;
                int y = 100 + completedLine * 25;  // assumes vertical line spacing = font size
                checkboxPositions[checkboxCount] = { 50, (float)y };  // X is near left border
                checkboxCount++;

                strcat(myText, "\n");
                rowCount++;
                lineLengths[rowCount] = 0;
            }

            // Handle backspace
            if (IsKeyPressed(KEY_BACKSPACE)) holdingBackspace = true;
            if (IsKeyReleased(KEY_BACKSPACE)) {
                holdingBackspace = false;
                backspaceTimer = 0;
            }

            if (holdingBackspace) backspaceTimer += GetFrameTime();
            else backspaceTimer = 0.0f;

            if ((IsKeyPressed(KEY_BACKSPACE) || (IsKeyDown(KEY_BACKSPACE) && backspaceTimer > 0.3f)) && strlen(myText) > 0) {
                int len = strlen(myText);
                if (myText[len - 1] == '\n') {
                    if (rowCount > 0) rowCount--;
                }
                else if (lineLengths[rowCount] > 0) {
                    lineLengths[rowCount]--;
                }
                myText[len - 1] = '\0';
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(Color{ 255, 199, 207, 255 });

        DrawText("SHOPPING LIST", 210, 30, 50, Color{ 237, 28, 36, 255 });
        DrawRectangleRec(WritingSpace, Color{ 202, 226, 150, 255 });
        DrawRectangleLinesEx(WritingSpace, 5, Color{ 184, 188, 120, 255 });
        DrawTexture(strawberry, 135, 15, WHITE);
        DrawTexture(leaves, 610, 65, WHITE);

        // Blinking cursor
        if (writingMode && ((framesCounter / 20) % 2 == 0)) {
            Vector2 cursorPos = { 75, 100 };
            int currentLine = 0;
            int lastLineStart = 0;
            int len = strlen(myText);

            for (int i = 0; i < len; i++) {
                if (myText[i] == '\n') {
                    currentLine++;
                    lastLineStart = i + 1;
                }
            }

            // Substring for current line
            const char* linePtr = &myText[lastLineStart];
            int lineLen = strlen(linePtr);

            // Find end of line (or next newline)
            for (int i = 0; linePtr[i] && linePtr[i] != '\n'; i++) {
                lineLen = i + 1;
            }

            char currentLineText[256] = { 0 };
            strncpy(currentLineText, linePtr, lineLen);
            currentLineText[lineLen] = '\0';

            Vector2 textSize = MeasureTextEx(font, currentLineText, 25, 0);
            cursorPos.y = 100 + currentLine * 25;
            cursorPos.x += textSize.x;

            if (writingMode && ((framesCounter / 20) % 2 == 0)) {
                DrawText("|", cursorPos.x, cursorPos.y, 25, RED);
            }
        }

        DrawTextEx(font, myText, { 80, 100 }, 25, 0, RED);

        for (int i = 0; i < checkboxCount; i++) {
            Rectangle box = { checkboxPositions[i].x, checkboxPositions[i].y, (float)heartEmpty.width, (float)heartEmpty.height };

            // Draw heart based on checkbox state
            if (checkboxStates[i]) {
                DrawTexture(heartFilled, (int)box.x, (int)box.y, WHITE);
            }
            else {
                DrawTexture(heartEmpty, (int)box.x, (int)box.y, WHITE);
            }

            // Toggle heart checkbox on click
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                CheckCollisionPointRec(GetMousePosition(), box)) {
                checkboxStates[i] = !checkboxStates[i];
            }
        }

        EndDrawing();

        framesCounter++;
    }

    // Save the text to file on exit
    std::ofstream outFile("shopping_list.txt");
    if (outFile.is_open()) {
        outFile << myText;
        outFile.close();
    }
    else {
        std::cerr << "Failed to save shopping list to file!" << std::endl;
    }


    UnloadFont(font);
    UnloadTexture(strawberry);
    UnloadTexture(leaves);
    UnloadTexture(heartEmpty);
    UnloadTexture(heartFilled);
    CloseWindow();

    return 0;
}