#include "engine.h"
#include <iostream>

const color skyBlue(77/255.0, 213/255.0, 240/255.0);
const color grassGreen(26/255.0, 176/255.0, 56/255.0);
const color darkGreen(27/255.0, 81/255.0, 45/255.0);
const color white(1, 1, 1);
const color brickRed(201/255.0, 20/255.0, 20/255.0);
const color darkBlue(1/255.0, 110/255.0, 214/255.0);
const color purple(119/255.0, 11/255.0, 224/255.0);
const color black(0, 0, 0);
const color magenta(1, 0, 1);
const color orange(1, 163/255.0, 22/255.0);
const color cyan (0, 1, 1);

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Set uniforms that never change
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    // TODO: Initialize the user to be a 10x10 white block
    //       centered at (0, 0)
   user = make_unique<Rect>(shapeShader, vec2(0, 0), vec2(10, 10), white); // placeholder for compilation

    // Init grass
    grass = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height / 3), grassGreen);

    // Init mountains
    mountains.push_back(make_unique<Triangle>(shapeShader, vec2(width/4, 300), vec2(width, 400), darkGreen));
    mountains.push_back(make_unique<Triangle>(shapeShader, vec2(2*width/3, 300), vec2(width, 500), darkGreen));

    // Init Cloud
    clouds.push_back(Cloud(shapeShader, vec2(200, 500)));
    clouds.push_back(Cloud(shapeShader, vec2(400, 520)));
    clouds.push_back(Cloud(shapeShader, vec2(325, 480)));

    // Init buildings from closest to furthest
    int totalBuildingWidth = 0;
    vec2 buildingSize;
    while (totalBuildingWidth < width + 50) {
        // Building height between 50-100
        buildingSize.y = rand() % 51 + 50;
        // Building width between 30-50
        buildingSize.x = rand() % 21 + 30;
        buildings1.push_back(make_unique<Rect>(shapeShader,
                                               vec2(totalBuildingWidth + (buildingSize.x / 2.0) + 5,
                                                    ((buildingSize.y / 2.0) + 50)),
                                               buildingSize, brickRed));
        totalBuildingWidth += buildingSize.x + 5;
    }
    // Populate second set of buildings
    totalBuildingWidth = 0;
    while (totalBuildingWidth < width + 100) {
        // TODO: Populate this vector of darkBlue buildings
        // Building height between 100-200
        buildingSize.y = rand() % 101 + 100;
        // Building width between 50-100
        buildingSize.x = rand() % 51 + 50;
        buildings2.push_back(make_unique<Rect>(shapeShader, vec2(totalBuildingWidth + (buildingSize.x / 2.0) + 5,
            (buildingSize.y + 50)), buildingSize, darkBlue));
            totalBuildingWidth += buildingSize.x + 5;
    }
    // Populate third set of buildings
    totalBuildingWidth = 0;
    while (totalBuildingWidth < width + 200) {
        // TODO: Populate this vector of purple buildings
        // Building height between 200-400
        buildingSize.y = rand() % 201 + 200;
        // Building width between 100-200
        buildingSize.x = rand() % 101 + 100;
        buildings3.push_back(make_unique<Rect>(shapeShader, vec2(totalBuildingWidth + (buildingSize.x / 2.0) + 5,
            (buildingSize.y + 50)), buildingSize, purple));
        totalBuildingWidth += buildingSize.x + 5;
    }
}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped

    // TODO: make the user move with the mouse
    user->setPos(vec2(MouseX, MouseY));

    for (const unique_ptr<Rect>& r : buildings1) {
        if (r->isOverlapping(*user)) {
            r->setColor(orange);
        } else {
            r->setColor(brickRed);
        }
    }

    // TODO: Update the colors of buildings2 and buildings3.
    //  Note that darkBlue buildings turn cyan when overlapping
    //  with the user, and purple buildings turn magenta.
    for (unique_ptr<Rect>& r : buildings2) {
        if (r->isOverlapping(*user)) {
            r->setColor(cyan);
        }
        //to make sure that they dont stay the altered color
        else {
            r->setColor(darkBlue);
        }
    }

    for (unique_ptr<Rect>& r : buildings3) {
        if (r->isOverlapping(*user)) {
            r->setColor(magenta);
        }
        //to make sure that they dont stay the altered color
        else {
            r->setColor(purple);
        }
    }

    // TODO: Once you are confident your isOverlapping method
    //  works, uncomment this code to have the program exit
    //  when the user overlaps with the clouds.
    for (const Cloud& c : clouds) {
        if (c.isOverlapping(*user)) {
            glfwSetWindowShouldClose(window, true);
        }
    }
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Update clouds
    for (Cloud& c : clouds) {
        c.moveXWithinBounds(-1, width);
    }

    // Update buildings
    for (int i = 0; i < buildings1.size(); ++i) {
        // Move all the red buildings to the left
        buildings1[i]->moveX(-1.5);
        // If a building has moved off the screen
        if (buildings1[i]->getPosX() < -(buildings1[i]->getSize().x/2)) {
            // Set it to the right of the screen so that it passes through again
            int buildingOnLeft = (buildings1[i] == buildings1[0]) ? buildings1.size()-1 : i - 1;
            buildings1[i]->setPosX(buildings1[buildingOnLeft]->getPosX() + buildings1[buildingOnLeft]->getSize().x/2 + buildings1[i]->getSize().x/2 + 5);
        }
    }

    // TODO: Make the other two vectors of buildings move.
    //  The larger the buildings, the slower they should move.

    // Update buildings
    for (int i = 0; i < buildings2.size(); ++i) {
        // Move all the red buildings to the left
        buildings2[i]->moveX(-0.9);
        // If a building has moved off the screen
        if (buildings2[i]->getPosX() < -(buildings2[i]->getSize().x/2)) {
            // Set it to the right of the screen so that it passes through again
            int buildingOnLeft = (buildings2[i] == buildings2[0]) ? buildings2.size()-1 : i - 1;
            buildings2[i]->setPosX(buildings2[buildingOnLeft]->getPosX() + buildings2[buildingOnLeft]->getSize().x/2 + buildings2[i]->getSize().x/2 + 5);
        }
    }

    // Update buildings
    for (int i = 0; i < buildings3.size(); ++i) {
        // Move all the red buildings to the left
        buildings3[i]->moveX(-0.4);
        // If a building has moved off the screen
        if (buildings3[i]->getPosX() < -(buildings3[i]->getSize().x/2)) {
            // Set it to the right of the screen so that it passes through again
            int buildingOnLeft = (buildings3[i] == buildings3[0]) ? buildings3.size()-1 : i - 1;
            buildings3[i]->setPosX(buildings3[buildingOnLeft]->getPosX() + buildings3[buildingOnLeft]->getSize().x/2 + buildings3[i]->getSize().x/2 + 5);
        }
    }

}

void Engine::render() {
    glClearColor(skyBlue.red,skyBlue.green, skyBlue.blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const unique_ptr<Triangle>& m : mountains) {
        m->setUniforms();
        m->draw();
    }

    for (Cloud& c : clouds) {
        c.setUniformsAndDraw();
    }

    grass->setUniforms();
    grass->draw();

    // TODO: Add logic to draw the the user and the buildings.
    //  Note that the order of drawing matters because whatever
    //  is drawn last appears on top.
    //building3
    for (unique_ptr<Rect>& b : buildings3) {
        b->setUniforms();
        b->draw();
    }
    //buildings2
    for (unique_ptr<Rect>& b : buildings2) {
        b->setUniforms();
        b->draw();
    }
    //buidlings1
    for (unique_ptr<Rect>& b : buildings1) {
        b->setUniforms();
        b->draw();
    }

    user->setUniforms();
    user->draw();


    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}