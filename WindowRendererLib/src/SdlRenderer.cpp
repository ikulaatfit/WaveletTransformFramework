#ifdef SDL_ENABLED
#include "SdlRenderer.h"

SdlRenderer::SdlRenderer()
{
	this->init();
}

SdlRenderer::SdlRenderer(const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type, int screen, bool fullscreen, s_color font_color, std::ostream *error)
{
	this->init();
  this->open(window_name, width, height, dev_type, screen, fullscreen, font_color, error);
}
 
SdlRenderer::~SdlRenderer(void)
{
	this->close();
}

void SdlRenderer::init()
{
	this->opened = false;
}

bool SdlRenderer::isOpened()
	{
		return this->opened;
	}

bool SdlRenderer::open(const std::string &window_name, unsigned int width, unsigned int height, input_buffer_type dev_type, int screen, bool fullscreen, s_color font_color, std::ostream *error)
{
  this->close();
  if((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) || (TTF_Init() != 0))
  {
    if(error != NULL) *error << "Error: SDL_Init() Failed: " << SDL_GetError() << std::endl;
    return false;
  }
  int x;
  int y;
  if(screen == DEF_WINDOW_NUM)
  {
    x = SDL_WINDOWPOS_CENTERED;
    y = SDL_WINDOWPOS_CENTERED;
  }
  else
  {
    x = SDL_WINDOWPOS_CENTERED_DISPLAY(screen);
    y = SDL_WINDOWPOS_CENTERED_DISPLAY(screen);
  }
  this->dev_type = dev_type;
  this->window_name = window_name;
  Uint32 flags = 0;
  if(fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
  if((this->frame_window = SDL_CreateWindow(this->window_name.c_str(), x, y, width, height, flags)) == NULL)
  {
    if(error != NULL) *error << "Error: SDL_CreateWindow Failed: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
  }

  if((this->frame_renderer = SDL_CreateRenderer(this->frame_window, -1, SDL_RENDERER_ACCELERATED)) == NULL)
  {
    SDL_DestroyWindow(this->frame_window);
    if(error != NULL) *error << "Error: SDL_CreateRenderer Failed: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
  }
  
  //this->frame_image = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 4);
  this->width = width;
  this->height = height;
  if((this->font = TTF_OpenFont("cour.ttf", 14)) == NULL)
  {
    if(error != NULL) *error << "Error: function TTF_OpenFont.";
    return false;
  }
  TTF_SetFontStyle(this->font, TTF_STYLE_BOLD);
  this->opened = true;
  this->font_color = font_color;
  return true;
}

bool SdlRenderer::showFrame(void *bgra_frame, std::string &output_text, unsigned int text_start_x, unsigned int text_start_y, std::ostream *error)
	{
    if(!this->isOpened())
    {
      return false;
    }
    SDL_Color color = {font_color.r, font_color.g, font_color.b, font_color.a};
    SDL_Surface *text_surface;
    SDL_Surface *frame_surface;
    SDL_Texture *frame_texture;
    if((output_text.size() != 0) && ((text_surface = TTF_RenderText_Blended_Wrapped(this->font, output_text.c_str(), color, width)) == NULL))
    {
      if(error != NULL) *error << "Error: TTF_RenderUTF8_Solid Failed: " << SDL_GetError() << std::endl;
      return false;
    }

    if((frame_surface = SDL_CreateRGBSurfaceFrom(bgra_frame, width, height, 4*8, width * 4, 0, 0, 0, 0)) == NULL)
    {
      SDL_DestroyRenderer(this->frame_renderer);
      SDL_DestroyWindow(this->frame_window);
      if(error != NULL) *error << "Error: SDL_CreateRGBSurfaceFrom Failed: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return false;
    }
    SDL_Rect rect = {(int)text_start_x,(int)text_start_y,0,0};
    if(output_text.size() != 0) SDL_BlitSurface(text_surface, NULL, frame_surface, &rect);
    if((frame_texture = SDL_CreateTextureFromSurface(this->frame_renderer, frame_surface)) == NULL)
    {
      SDL_FreeSurface(frame_surface);
      SDL_DestroyRenderer(this->frame_renderer);
      SDL_DestroyWindow(this->frame_window);
      if(error != NULL) *error << "Error: SDL_CreateTextureFromSurface Failed: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return false;
    }
    SDL_RenderCopy(this->frame_renderer, frame_texture, NULL, NULL);
    SDL_RenderPresent(this->frame_renderer);

	// TODO this should work but not working
    if(output_text.size() != 0) SDL_FreeSurface(text_surface);

    SDL_DestroyTexture(frame_texture);
    SDL_FreeSurface(frame_surface);

    return true;
	}

void SdlRenderer::close()
{
  if(this->isOpened())
  {
    //TTF_CloseFont(this->font);
    //TTF_Quit();
    SDL_DestroyRenderer(this->frame_renderer);
    SDL_DestroyWindow(this->frame_window);
    //SDL_Quit();
    this->opened = false;
  }
}

unsigned char SdlRenderer::getKey()
  {
    if(!this->isOpened()) return 0;
    
    while(SDL_PollEvent(&event))
    {
      switch(event.type)
      {
      case SDL_KEYUP:
        return (unsigned char)event.key.keysym.sym;
      case SDL_WINDOWEVENT:
        switch(event.window.event)
        {
        case SDL_WINDOWEVENT_CLOSE:
          return (unsigned char)SDLK_ESCAPE;
        }
      }
    }
    return 0;
  }


#endif