#include "lab_m1/lab1/lab1.h"

#include <vector>
#include <iostream>
#include <glm/gtx/matrix_transform_2d.hpp>

#include <windows.h>
#include <shellapi.h>


using namespace std;
using namespace m1;

vector<string> questions = { "Recruitment is based on the manager's capacity to identify the ______  of the product.",
                            "The final decision-making stage in the recruitment process is called _______",
                            "The two types of ________ evaluation are: individual and team-based.",
                            "Which organizational theory states that workers are motivated by a sense of commitment?",
                            "What is the name of the graph that indicates the level of involvement and responsibility of employees?",
                            "What percentage of the time does a manager spend communicating?",
                            "The group in charge of reviewing the technical specifications and authorizing changes is called:_____ _____",
                            "The number of steps necessary to establish a communication plan.",
                            "The extended systematic model of risk management includes _ steps.",
                            "The _____ represents the various risk categories that can trigger negative events." };

vector<string> answers = {
    "REQUIREMENTS",
    "SELECTION",
    "PERFORMANCE",
    "Z THEORY",
    "RESPONSIBLE ACCOUNTABLE CONSULTED INFORMED",
    "805",
    "CONTROL BOARD",
    "8",
    "8",
    "RISK SOURCE" };

constexpr int CAPTURED = 50;

Lab1::Lab1()
{
    player = std::make_unique<Player>(glm::vec3(0, 0, 0), glm::vec3(0.99f, 0, 0));
    meshes["player"] = player->generateModel();

    const auto camera = GetSceneCamera();
    camera->SetPosition(glm::vec3(0, 0, 30));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    corners = { 29, 16 };
    capturedCircles = 0;
    checkedWord = false;
    checkedWord1 = false;
    checkedWord2 = false;
    intrebare_curenta = 0;

    text_renderer = new gfxc::TextRenderer(window->props.selfDir, window->GetResolution().x, window->GetResolution().y);
    text_renderer->Load(RESOURCE_PATH::FONTS + "\\" + "Hack-Bold.ttf", 128);

    magicNum = rand() % 889 + 111;

    q1 = rand() % 10;

    do
    {
        q2 = rand() % 10;
    } while (q1 == q2);

    do
    {
        q3 = rand() % 10;
    } while (q3 == q1 || q3 == q2);
}

Lab1::~Lab1()
{
}

void Lab1::Init()
{
    {
        Mesh* mesh = new Mesh("square");

        std::vector<VertexFormat> vertices;
        glm::vec3 color = { 0, 0, 0 };
        vertices.emplace_back(glm::vec3(-1, 1, 0), color);
        vertices.emplace_back(glm::vec3(1, 1, 0), color);
        vertices.emplace_back(glm::vec3(-1, -1, 0), color);
        vertices.emplace_back(glm::vec3(1, -1, 0), color);

        mesh->InitFromData(vertices, { 0, 1, 3, 2, 0, 2, 1, 3 });
        mesh->SetDrawMode(GL_LINES);

        meshes["square"] = mesh;
    }

    {
        Mesh* mesh = new Mesh("square");

        std::vector<VertexFormat> vertices;
        glm::vec3 color = { 1, 0, 0 };
        vertices.emplace_back(glm::vec3(-1, 1, 0), color);
        vertices.emplace_back(glm::vec3(1, 1, 0), color);
        vertices.emplace_back(glm::vec3(-1, -1, 0), color);
        vertices.emplace_back(glm::vec3(1, -1, 0), color);

        mesh->InitFromData(vertices, { 0, 1, 3, 2, 0, 2, 1, 3 });
        mesh->SetDrawMode(GL_LINES);

        meshes["squareRed"] = mesh;
    }

    {
        Mesh* mesh = new Mesh("circle");

        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;

        for (int i = 0; i < 50; ++i)
        {
            float arg = (2.0 * M_PI * i) / 50;
            vertices.emplace_back(
                glm::vec3(cos(arg) * 0.57f, sin(arg) * 0.57f, 0),
                glm::vec3(0.911f, 0.188f, 0.217f));
            indices.push_back(i);
        }

        mesh->InitFromData(vertices, indices);
        meshes["circle"] = mesh;
        meshes["circle"]->SetDrawMode(GL_TRIANGLE_FAN);
    }

    for (int i = 0; i < 3; i++)
    {
        int rnd = rand() % 2;
        int signX = rnd == 1 ? rnd : -1;

        rnd = rand() % 2;
        int signY = rnd == 1 ? rnd : -1;

        int x = rand() % (corners[0] - 1) + 1;
        x *= signX;

        int y = rand() % (corners[1] - 1) + 1;
        y *= signY;

        circlePositions.emplace_back(x, y);
    }

    for (int i = 0; i < 3; i++)
    {
        int rnd = rand() % 2;
        int signX = rnd == 1 ? rnd : -1;

        rnd = rand() % 2;
        int signY = rnd == 1 ? rnd : -1;

        int x = rand() % (corners[0] - 1) + 1;
        x *= signX;

        int y = rand() % (corners[1] - 1) + 1;
        y *= signY;

        targetPositions.emplace_back(x, y);
    }

    auto str = PATH_JOIN("file:///", window->props.selfDir, "Credits.pdf");
    ShellExecute(0, 0, str.c_str(), 0, 0, SW_SHOW);
}

void Lab1::FrameStart()
{
    glm::ivec2 resolution = window->props.resolution;
    glClearColor(0.5, 0.5, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, resolution.x, resolution.y);
}

void Lab1::Update(float deltaTimeSeconds)
{
    glm::mat3 model = glm::mat3(1);
    model = glm::translate(model, player->position);
    RenderMesh2D(meshes["player"], shaders["VertexColor"], model);

    glm::mat3 circlePos = glm::mat3(1);
    auto playerPos = player->position + glm::vec2(1.0, 1.0);
    auto playerNeg = player->position - glm::vec2(1.0, 1.0);

    for (auto& pos : circlePositions)
    {

        if (playerNeg.x < pos.x && pos.x < playerPos.x)
        {
            if (playerNeg.y < pos.y && pos.y < playerPos.y)
            {
                if (!player->isInDeliveryZone(deliveryPos1, deliveryPos2, deliveryNeg1, deliveryNeg2))
                {
                    pos.x = CAPTURED;
                    capturedCircles++;
                }
            }
        }

        if (pos.x != CAPTURED)
        {
            RenderMesh2D(meshes["circle"], shaders["VertexNormal"], glm::translate(circlePos, pos));
        }
    }

    glm::mat3 deliveryZone = glm::mat3(1);
    deliveryZone = glm::translate(deliveryZone, { -26, 11 });
    deliveryZone = glm::scale(deliveryZone, { 3, 5 });

    char* text = new char[30];

    /*if (key == GLFW_KEY_C)
    {
        auto str = PATH_JOIN("file:///", window->props.selfDir, "Credits.pdf");
        ShellExecute(0, 0, str.c_str(), 0, 0, SW_SHOW);
    }*/

    if (capturedCircles == 6)
    {
        if (checkedWord2)
        {
            sprintf(text, "Secret code: %d", magicNum % 1000);
            text_renderer->RenderText("Congratulations! Now you have to exit!", 200, 90, 0.2f, { 0.99, 0.99, 0.99 });
        }
        else if (checkedWord1)
        {
            sprintf(text, "Secret code: %d", magicNum % 1000);
            text_renderer->RenderText("You guessed it! Last one, we promise!", 200, 90, 0.2f, { 0.99, 0.99, 0.99 });
            RenderMesh2D(meshes["player"], glm::translate(glm::mat3(1), { 5, 5 }), glm::vec3(0.4f, 0, 0.99f));
            text_renderer->RenderText(questions[q3], 200, 130, 0.1f, { 0.99, 0.99, 0.99 });
            intrebare_curenta = 3;
        }
        else if (checkedWord)
        {
            sprintf(text, "Secret code: %d", magicNum % 100);
            text_renderer->RenderText("You guessed it! Time for part 2", 200, 90, 0.2f, { 0.99, 0.99, 0.99 });
            text_renderer->RenderText(questions[q2], 200, 130, 0.1f, { 0.99, 0.99, 0.99 });
            intrebare_curenta = 2;
        }
        else
        {
            sprintf(text, "Secret code: %d", magicNum % 100);
            text_renderer->RenderText(questions[q1], 200, 90, 0.1f, { 0.99, 0.99, 0.99 });
            intrebare_curenta = 1;
        }
    }
    else
    {
        sprintf(text, "Secret code: %d", magicNum % 10);
    }
    text_renderer->RenderText(text, 200, 40, 0.3f, { 0.99, 0.99, 0.99 });
    delete text;

    RenderMesh2D(meshes["squareRed"], shaders["VertexColor"], deliveryZone);
    RenderMesh2D(meshes["square"], shaders["VertexColor"], glm::scale(glm::mat3(1), { corners[0] + 0.7f, corners[1] + 0.7f }));
}

void Lab1::FrameEnd()
{
}

void Lab1::OnInputUpdate(float deltaTime, int mods)
{
    if (player->isInWindow(corners[0], corners[1]))
    {
        if (window->KeyHold(GLFW_KEY_A))
        {
            player->moveLeft(deltaTime * 15);
        }
        if (window->KeyHold(GLFW_KEY_D))
        {
            player->moveRight(deltaTime * 15);
        }
        if (window->KeyHold(GLFW_KEY_W))
        {
            player->moveUp(deltaTime * 15);
        }
        if (window->KeyHold(GLFW_KEY_S))
        {
            player->moveDown(deltaTime * 15);
        }
    }
    else
    {
        player->position = { 0, 0 };
    }
}

void Lab1::OnKeyPress(int key, int mods)
{
    if (player->isInDeliveryZone(deliveryPos1, deliveryPos2, deliveryNeg1, deliveryNeg2))
    {
        if (key == GLFW_KEY_SPACE)
        {
            for (auto& p : circlePositions)
            {
                if (p.x == CAPTURED)
                {
                    p = player->position;
                    capturedCircles++;
                    break;
                }
            }
        }
    }

    if (capturedCircles == 6)
    {
        static int pressedChars = 0;
        if ((GLFW_KEY_A <= key && key <= GLFW_KEY_Z) || key == 32 || (key >= 48 && key <= 57))
        {
            chars_pressed.push_back(key);
            
        }
        for (int i = 0; i < chars_pressed.size(); i++)
        {
            cout << chars_pressed[i];
        }
        cout << "\n";

        string raspuns = "";

        switch (intrebare_curenta)
        {
        case 1:

            for (int i = chars_pressed.size() - answers[q1].size(); i < chars_pressed.size(); i++)
            {
                raspuns = raspuns + chars_pressed[i];
            }
            cout << raspuns << "\n";
            if (raspuns == answers[q1])
            {
                checkedWord = true;
            }

        case 2:

            for (int i = chars_pressed.size() - answers[q2].size(); i < chars_pressed.size(); i++)
            {
                raspuns = raspuns + chars_pressed[i];
            }
            cout << raspuns << "\n";
            if (raspuns == answers[q2])
            {
                checkedWord1 = true;
            }

        case 3:

            for (int i = chars_pressed.size() - answers[q3].size(); i < chars_pressed.size(); i++)
            {
                raspuns = raspuns + chars_pressed[i];
            }

            if (raspuns == answers[q3])
            {
                checkedWord2 = true;
            }
        }

        if (checkedWord2)
        {
            std::vector<int> secretCode = { magicNum % 10, (magicNum / 10) % 10, (magicNum / 100) % 10 };
            std::reverse(secretCode.begin(), secretCode.end());

            int numeros = 0;

            if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
            {
                nums_pressed.push_back(key - 48);
            }

            if (nums_pressed.size() >= 3)
                for (int i = nums_pressed.size() - 3; i < nums_pressed.size(); i++)
                {
                    numeros = numeros * 10 + nums_pressed[i];
                }

            if (numeros == magicNum)
            {
                exit(0);
            }
        }
    };
}

void Lab1::OnKeyRelease(int key, int mods)
{
}

void Lab1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}

void Lab1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}

void Lab1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void Lab1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Lab1::OnWindowResize(int width, int height)
{
}
