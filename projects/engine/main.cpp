#include <sdl3/SDL.h>
#include <sdl3/SDL_main.h>
#include <sdl3_image/SDL_image.h>
#include <string>

#pragma comment(lib, "SDL3.lib")
#pragma comment(lib, "SDL3_image.lib")


/* ���� ���� */
//�������� â
SDL_Window* gWindow{ nullptr };

// â�� �׸��� �� ���Ǵ� ������ 
SDL_Renderer* gRenderer{ nullptr };

//â�� ���Ե� ǥ��
SDL_Surface* gScreenSurface{ nullptr };

// ȭ�鿡 �ε��Ͽ� ǥ���� �̹���
SDL_Surface* gHelloWorld{ nullptr };


/* Ŭ���� ������Ÿ�� */
class LTexture
{
public:
    //�ؽ�ó ������ �ʱ�ȭ�մϴ� 
    LTexture();

    //�ؽ�ó ������ �����մϴ� 
    ~LTexture();

    //��ũ���� �ؽ�ó�� �ε��մϴ� 
    bool loadFromFile(std::string path);

    //�ؽ�ó�� �����մϴ� 
    void destroy();

    //�ؽ�ó�� �׸��ϴ� 
    void render(float x, float y);

    //�ؽ�ó ũ�⸦ �����ɴϴ� 
    int getWidth();
    int getHeight();

private:
    //�ؽ�ó �����͸� �����մϴ� 
    SDL_Texture* mTexture;

    //�ؽ�ó ũ�� 
    int mWidth;
    int mHeight;
};

LTexture::LTexture() :
    //�ؽ�ó ���� �ʱ�ȭ 
    mTexture{ nullptr },
    mWidth{ 0 },
    mHeight{ 0 }
{

}

LTexture::~LTexture()
{
    //�ؽ�ó ���� 
    destroy();
}

bool LTexture::loadFromFile(std::string path)
{
    //�ؽ�ó�� �̹� �ִ� ��� ���� 
    destroy();

    //ǥ�� �ε� 
    if (SDL_Surface* loadedSurface = IMG_Load(path.c_str()); loadedSurface == nullptr)
    {
        SDL_Log("Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError());
    }
    else
    {
        //ǥ�鿡�� �ؽ�ó ���� 
        if (mTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface); mTexture == nullptr)
        {
            SDL_Log("Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError());
        }
        else
        {
            //�̹��� ũ�� �������� 
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        //�ε�� ǥ�� ���� 
        SDL_DestroySurface(loadedSurface);
    }

    //�ؽ�ó�� �ε�Ǹ� ������ ��ȯ �մϴ�.
    return mTexture != nullptr;
}

void LTexture::destroy()
{
    //�ؽ�ó ���� 
    SDL_DestroyTexture(mTexture);
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

void LTexture::render(float x, float y)
{
    //�ؽ�ó ��ġ ���� 
    SDL_FRect dstRect = { x, y, static_cast<float>(mWidth), static_cast<float>(mHeight) };

    //�ؽ�ó ������ 
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

// �������� PNG �̹��� 
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
    //���� �ε� �÷��� 
    bool success{ true };

    //���÷��� �̹��� �ε� 
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
    //���� ���� �ڵ� 
    int exitCode{ 0 };

    //�ʱ�ȭ 
    if (!init())
    {
        SDL_Log("���α׷��� �ʱ�ȭ�� �� �����ϴ�!\n");
        exitCode = 1;
    }
    else
    {
        //�̵�� �ε� 
        if (!loadMedia())
        {
            SDL_Log("�̵� �ε��� �� �����ϴ�!\n");
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

                //����� ������� ä��� 
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(gRenderer);

                //ȭ�鿡 �̹��� ������ 
                gPngTexture.render(0.f, 0.f);

                //ȭ�� ������Ʈ 
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    //���� 
    close();

    return exitCode;
}