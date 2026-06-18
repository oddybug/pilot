#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glad/gl.h> // You generally only need gl.h if SDL manages the context
#include <iostream>

int main(int argc, char *argv[]) {
  SDL_Window *window = nullptr;
  SDL_GLContext gl_context = nullptr;
  bool done = false;

  // 1. Initialize SDL3 Video first
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  // Optional: Set your desired OpenGL versions/profile before creating the window
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // 2. Create the window
  window = SDL_CreateWindow("An SDL3 window", 640, 480, SDL_WINDOW_OPENGL);
  if (window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  // 3. Create the OpenGL context (Crucial: Must happen BEFORE loading GLAD)
  gl_context = SDL_GL_CreateContext(window);
  if (gl_context == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create GL context: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // 4. NOW initialize GLAD loader
  // SDL_GL_GetProcAddress is cast to GLADloadfunc so GLAD knows how to fetch functions
  int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
  if (version == 0) {
    std::cerr << "Failed to initialize GLAD OpenGL context" << std::endl;
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  // Successfully loaded! Print version (e.g., 30033 for GL 3.3)
  std::cout << "GLAD Loaded OpenGL Version: " << GLAD_VERSION_MAJOR(version) 
            << "." << GLAD_VERSION_MINOR(version) << std::endl;

  // 5. Main Game Loop
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      }
    }

    // Render loop
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
  }

  // Cleanup
  SDL_GL_DestroyContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
