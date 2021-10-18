//=============================================================================
class CNoLista
{
public:
    int dados;
    CNoLista *proximo;
}; // fim de class CNoLista
//=============================================================================
class ListaFrames
{
    CNoLista *cabeca;
public:
    ListaFrames();
    ~ListaFrames();
    explicit ListaFrames(char *str);
    int numElems() const;
    int operator[](int) const;
    bool isEmpty() const;
    ListaFrames(const ListaFrames &input);
    void insereItem(int);
};



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
void ListaFrames::insereItem(int item)
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
// exe 8.7
//=============================================================================
ListaFrames::ListaFrames(char *str)
{
  cabeca = nullptr;
  int size = strlen(str);

  //cout << endl << "Tamanho: " << size << endl;

  for (int i = 0; i < size; ++i)
  {
    //cout << "Char: " << str[i] << " Valor: " <<(int) str[i] << endl;
    insereItem((int) str[i]);
  }
}
//=============================================================================

//=============================================================================
// exe 8.8 Aux
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

//=============================================================================
// exe 8.5
//=============================================================================
int ListaFrames::operator[](int index) const
{
  CNoLista *atual = cabeca;

  //A melhorar
  if (index > this->numElems())
  {
    return -1;
  }
  else
  {
    for (int i = 0; i < index; ++i)
    {
      atual = atual->proximo;
    }
    return atual->dados;
  }
}
//=============================================================================

//=============================================================================
// exe 8.9 Aux
//=============================================================================
bool ListaFrames::isEmpty() const
{
  return (this->cabeca == nullptr);
}
//=============================================================================

//=============================================================================
// exe 8.11 Aux
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