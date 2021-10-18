#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <chrono>
#include <thread>

#include <vector>

//=============================================================================
class CNoLista
{
public:
    rs2::frameset dados;
    CNoLista *proximo;
}; // fim de class CNoLista
//=============================================================================
class ListaFrames
{
    CNoLista *cabeca;
public:
    ListaFrames();
    ~ListaFrames();
    int numElems() const;
    rs2::frameset operator[](int) const;
    bool isEmpty() const;
    ListaFrames(const ListaFrames &input);
    void insereItem(rs2::frameset);
};

// 3rd party header for writing png files
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

std::vector<rs2::frameset> frameBuffer;

int main(int argc, char *argv[]) try
{
  ListaFrames lista;

  rs2::config config;
  config.enable_stream(rs2_stream::RS2_STREAM_DEPTH, 640, 480, rs2_format::RS2_FORMAT_Z16, 15);
  config.enable_stream(rs2_stream::RS2_STREAM_COLOR, 640, 480, rs2_format::RS2_FORMAT_RGB8, 15);

  rs2::colorizer color_map(2);
  rs2::pipeline pipe;
  pipe.start(config);

  // Wait for the next set of frames from the camera. Now that autoexposure, etc.
  // has settled, we will write these to disk
  for (int i = 0; i < 500; ++i)
  {
    std::cout << "Iteration: " << i << std::endl;
    rs2::frameset newFrame;

    newFrame = pipe.wait_for_frames(1000);

  }


  return EXIT_SUCCESS;
}
catch (const rs2::error &e)
{
  std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    "
            << e.what() << std::endl;
  return EXIT_FAILURE;
}
catch (const std::exception &e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}

//=============================================================================
ListaFrames::ListaFrames()
{ // Construtor por omissao
  cabeca = nullptr;
}
//=============================================================================
ListaFrames::~ListaFrames()
{
  CNoLista *atual, *proximo;

  atual = cabeca;
  while (atual != nullptr)
  {
    proximo = atual->proximo;
    delete atual;
    atual = proximo;
  }
}
//=============================================================================
void ListaFrames::insereItem(rs2::frameset item)
{
  auto *novo = new CNoLista;
  CNoLista *atual;

  novo->dados = item;
  novo->proximo = nullptr;

  if (cabeca == nullptr)
  {
    cabeca = novo;
  }
  else
  {
    atual = cabeca;
    while (atual->proximo != nullptr)
      atual = atual->proximo;
    atual->proximo = novo;
  }
}
//=============================================================================
int ListaFrames::numElems() const
{
  CNoLista *atual = cabeca;
  int counter = 0;


  if (cabeca == nullptr)
  {
    return -1;
  }
  else
  {
    while (atual->proximo != nullptr)
    {
      atual = atual->proximo;
      counter++;
    }
    return counter + 1;
  }
}
//=============================================================================
rs2::frameset ListaFrames::operator[](int index) const
{
  CNoLista *atual = cabeca;

  for (int i = 0; i < index; ++i)
  {
    atual = atual->proximo;
  }
  return atual->dados;
}
//=============================================================================
bool ListaFrames::isEmpty() const
{
  return (this->cabeca == nullptr);
}
//=============================================================================
ListaFrames::ListaFrames(const ListaFrames &input)
{
  this->cabeca = nullptr;

  for (int i = 0; i < input.numElems(); ++i)
  {
    this->insereItem(input[i]);
  }
}
//=============================================================================