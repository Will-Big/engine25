#include <sdl3/SDL.h>
#include <sdl3/SDL_main.h>
#include <string>

#pragma comment(lib, "SDL3.lib")

#include <filesystem>

/* 상수 */
//화면 크기 상수
constexpr int kScreenWidth{ 640 };
constexpr int kScreenHeight{ 480 };

//SDL을 시작하고 창을 생성합니다.
bool init();

//미디어를 로드합니다
bool loadMedia();

//미디어를 해제하고 SDL을 종료합니다.
void close();

/* 전역 변수 */
//렌더링할 창
SDL_Window* gWindow{ nullptr };

//창에 포함된 표면
SDL_Surface* gScreenSurface{ nullptr };

// 화면에 로드하여 표시할 이미지
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
    //파일 로딩 플래그 
    bool success{ true };

    //스플래시 이미지 로드 
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
    //표면 정리 
    SDL_DestroySurface(gHelloWorld);
    gHelloWorld = nullptr;

    //창 파괴 
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gScreenSurface = nullptr;

    //SDL 하위 시스템 종료 
    SDL_Quit();
}

int main(int argc, char* args[])
{
    //최종 종료 코드 
    int exitCode{ 0 };

    //초기화 
    if (!init())
    {
        SDL_Log("Unable to initialize program!\n");
        exitCode = 1;
    }
    else
    {
        //미디어 로드 
        if (!loadMedia())
        {
            SDL_Log("Unable to load media!\n");
            exitCode = 2;
        }
        else
        {
            //종료 플래그 
            bool quit{ false };

            //이벤트 데이터 
            SDL_Event e;
            SDL_zero(e);
            //메인 루프 
            while (quit == false)
            {
                //이벤트 데이터 가져오기 
                while (SDL_PollEvent(&e))
                {
                    //이벤트가 종료 유형인 경우 
                    if (e.type == SDL_EVENT_QUIT)
                    {
                        //메인 루프 종료 
                        quit = true;
                    }
                }
                //표면을 흰색으로 채웁니다. 
                SDL_FillSurfaceRect(gScreenSurface, nullptr, SDL_MapSurfaceRGB(gScreenSurface, 0xFF, 0xFF, 0xFF));

                //화면에 이미지를 렌더링합니다 .
                SDL_BlitSurface(gHelloWorld, nullptr, gScreenSurface, nullptr);

                //표면을 업데이트합니다. 
                SDL_UpdateWindowSurface(gWindow);
            }
        }
    }

    //정리 
    close();

    return exitCode;
}