#include <sdl3/SDL.h>
#include <sdl3/SDL_main.h>
#include <string>

#pragma comment(lib, "SDL3.lib")

#include <filesystem>

/* ��� */
//ȭ�� ũ�� ���
constexpr int kScreenWidth{ 640 };
constexpr int kScreenHeight{ 480 };

//SDL�� �����ϰ� â�� �����մϴ�.
bool init();

//�̵� �ε��մϴ�
bool loadMedia();

//�̵� �����ϰ� SDL�� �����մϴ�.
void close();

/* ���� ���� */
//�������� â
SDL_Window* gWindow{ nullptr };

//â�� ���Ե� ǥ��
SDL_Surface* gScreenSurface{ nullptr };

// ȭ�鿡 �ε��Ͽ� ǥ���� �̹���
SDL_Surface* gHelloWorld{ nullptr };


bool init()
{
    //Initialization flag
    bool success{ true };

    //Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Create window
        if (gWindow = SDL_CreateWindow("SDL3 Tutorial: Hello SDL3", kScreenWidth, kScreenHeight, 0); gWindow == nullptr)
        {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }

    return success;
}

bool loadMedia()
{
    //���� �ε� �÷��� 
    bool success{ true };

    //���÷��� �̹��� �ε� 
    std::string imagePath{ "../../resources/hello-sdl3.bmp" };
    if (gHelloWorld = SDL_LoadBMP(imagePath.c_str()); gHelloWorld == nullptr)
    {
        SDL_Log("Unable to load image %s! SDL Error: %s\n", imagePath.c_str(), SDL_GetError());
        success = false;
    }

    return success;
}

void close()
{
    //ǥ�� ���� 
    SDL_DestroySurface(gHelloWorld);
    gHelloWorld = nullptr;

    //â �ı� 
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gScreenSurface = nullptr;

    //SDL ���� �ý��� ���� 
    SDL_Quit();
}

int main(int argc, char* args[])
{
    //���� ���� �ڵ� 
    int exitCode{ 0 };

    //�ʱ�ȭ 
    if (!init())
    {
        SDL_Log("Unable to initialize program!\n");
        exitCode = 1;
    }
    else
    {
        //�̵�� �ε� 
        if (!loadMedia())
        {
            SDL_Log("Unable to load media!\n");
            exitCode = 2;
        }
        else
        {
            //���� �÷��� 
            bool quit{ false };

            //�̺�Ʈ ������ 
            SDL_Event e;
            SDL_zero(e);
            //���� ���� 
            while (quit == false)
            {
                //�̺�Ʈ ������ �������� 
                while (SDL_PollEvent(&e))
                {
                    //�̺�Ʈ�� ���� ������ ��� 
                    if (e.type == SDL_EVENT_QUIT)
                    {
                        //���� ���� ���� 
                        quit = true;
                    }
                }
                //ǥ���� ������� ä��ϴ�. 
                SDL_FillSurfaceRect(gScreenSurface, nullptr, SDL_MapSurfaceRGB(gScreenSurface, 0xFF, 0xFF, 0xFF));

                //ȭ�鿡 �̹����� �������մϴ� .
                SDL_BlitSurface(gHelloWorld, nullptr, gScreenSurface, nullptr);

                //ǥ���� ������Ʈ�մϴ�. 
                SDL_UpdateWindowSurface(gWindow);
            }
        }
    }

    //���� 
    close();

    return exitCode;
}