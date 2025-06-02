#include <sdl3/SDL.h>
#include <sdl3/SDL_main.h>
#include <sdl3_image/SDL_image.h>
#include <string>

#pragma comment(lib, "SDL3.lib")
#pragma comment(lib, "SDL3_image.lib")


/* 전역 변수 */
//렌더링할 창
SDL_Window* gWindow{ nullptr };

// 창에 그리는 데 사용되는 렌더러 
SDL_Renderer* gRenderer{ nullptr };

//창에 포함된 표면
SDL_Surface* gScreenSurface{ nullptr };

// 화면에 로드하여 표시할 이미지
SDL_Surface* gHelloWorld{ nullptr };


/* 클래스 프로토타입 */
class LTexture
{
public:
    //텍스처 변수를 초기화합니다 
    LTexture();

    //텍스처 변수를 정리합니다 
    ~LTexture();

    //디스크에서 텍스처를 로드합니다 
    bool loadFromFile(std::string path);

    //텍스처를 정리합니다 
    void destroy();

    //텍스처를 그립니다 
    void render(float x, float y);

    //텍스처 크기를 가져옵니다 
    int getWidth();
    int getHeight();

private:
    //텍스처 데이터를 포함합니다 
    SDL_Texture* mTexture;

    //텍스처 크기 
    int mWidth;
    int mHeight;
};

LTexture::LTexture() :
    //텍스처 변수 초기화 
    mTexture{ nullptr },
    mWidth{ 0 },
    mHeight{ 0 }
{

}

LTexture::~LTexture()
{
    //텍스처 정리 
    destroy();
}

bool LTexture::loadFromFile(std::string path)
{
    //텍스처가 이미 있는 경우 정리 
    destroy();

    //표면 로드 
    if (SDL_Surface* loadedSurface = IMG_Load(path.c_str()); loadedSurface == nullptr)
    {
        SDL_Log("Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError());
    }
    else
    {
        //표면에서 텍스처 생성 
        if (mTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface); mTexture == nullptr)
        {
            SDL_Log("Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError());
        }
        else
        {
            //이미지 크기 가져오기 
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        //로드된 표면 정리 
        SDL_DestroySurface(loadedSurface);
    }

    //텍스처가 로드되면 성공을 반환 합니다.
    return mTexture != nullptr;
}

void LTexture::destroy()
{
    //텍스처 정리 
    SDL_DestroyTexture(mTexture);
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

void LTexture::render(float x, float y)
{
    //텍스처 위치 설정 
    SDL_FRect dstRect = { x, y, static_cast<float>(mWidth), static_cast<float>(mHeight) };

    //텍스처 렌더링 
    SDL_RenderTexture(gRenderer, mTexture, nullptr, &dstRect);
}

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}

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

// 렌더링할 PNG 이미지 
LTexture gPngTexture;


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
        if( !SDL_CreateWindowAndRenderer( "SDL3 Tutorial: Textures and Extension Libraries", kScreenWidth, kScreenHeight, 0, &gWindow, &gRenderer ) )
        {
            SDL_Log( "Window could not be created! SDL error: %s\n", SDL_GetError() );
            success = false;
        }
    }

    return success;
}

bool loadMedia()
{
    //파일 로딩 플래그 
    bool success{ true };

    //스플래시 이미지 로드 
    if (success = gPngTexture.loadFromFile("../../resources/mario.png"); !success)
    {
        SDL_Log("Unable to load png image!\n");
    }

    return success;

    return success;
}

void close()
{
    //Clean up texture
    gPngTexture.destroy();

    //Destroy window
    SDL_DestroyRenderer(gRenderer);
    gRenderer = nullptr;
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;

    //Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* args[])
{
    //최종 종료 코드 
    int exitCode{ 0 };

    //초기화 
    if (!init())
    {
        SDL_Log("프로그램을 초기화할 수 없습니다!\n");
        exitCode = 1;
    }
    else
    {
        //미디어 로드 
        if (!loadMedia())
        {
            SDL_Log("미디어를 로드할 수 없습니다!\n");
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

                //배경을 흰색으로 채우기 
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(gRenderer);

                //화면에 이미지 렌더링 
                gPngTexture.render(0.f, 0.f);

                //화면 업데이트 
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    //정리 
    close();

    return exitCode;
}