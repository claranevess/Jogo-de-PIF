<<<<<<< HEAD
# Raylib-Quickstart
A simple cross platform template for setting up a project with the bleeding edge raylib code.
Works with C or C++.

## Supported Platforms
Quickstart supports the main 3 desktop platforms
* Windows
* Linux
* MacOS

# VSCode Users (all platforms)
* Download the quickstart
* Rename the folder to your game name
* Open the folder in VSCode.
* Press F5 to build
* You are good to go.

# Windows Users
There are two compiler toolchains avialble for windows, MinGW-W64 (a free compiler using GCC), and Microsoft Visual Studio
## Using MinGW-W64
* Double click the build-MinGW-W64.bat file.
* cd into the folder in your terminal
* run make
* You are good to go

### Note on MinGW-64 versions
Make sure you have a modern version of MinGW-W64 (not mingw).
The best place to get it is from the W64devkit from
https://github.com/skeeto/w64devkit/releases
or the version installed with the raylib installer
#### If you have installed rayib from the installer
Make sure you have added the path

 C:\raylib\w64devkit\bin 

To your path environment varialbe so that the compiler that came with raylib can be found..

DO NOT INSALL ANOTHER MinGW-W64 from another source such as msys2, you don't need it.

## Microsoft Visual Studio
* Run the build-VisualStudio2022.bat
* double click the .sln file that is geneated.
* develop your game
* you are good to go.

# Linux Users
* CD into the build folder
* run ./premake5 gmake2
* CD back to the root
* run make
* you are good to go

# MacOS Users
* CD into the build folder
* run ./premake5.osx gmake2
* CD back to the root
* run make
* you are good to go

# Output files
The built code will be in the bin dir

# Working directories and the resources folder
The example uses a utility function from path_utils.h that will find the resources dir and set it as the current working directory. This is very useful when starting out. If you wish to manage your own working directory you can simply remove the call to the function and the header.

# Changing to C++
Simply rename src/main.c to src/main.cpp and re-run the steps above and do a clean build.

# Using your own code
Simply remove src/main.c and replace it with your code, and re-run the steps above and do a clean build.

# Building for other OpenGL targets
If you need to build for a different OpenGL version than the default (OpenGL 3.3) you can specify an openGL version in your premake command line. Just modify the bat file or add the following to your command line

## For OpenGL 1.1
--graphics=opengl11

## For OpenGL 2.1
--graphics=opengl21

## For OpenGL 4.3
--graphics=opengl43

## For OpenGLES 2.0
--graphics=opengles2

## For OpenGLES 3.0
--graphics=opengles3

# License
Copyright (c) 2020-2024 Jeffery Myers

This software is provided "as-is", without any express or implied warranty. In no event 
will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you 
  wrote the original software. If you use this software in a product, an acknowledgment 
  in the product documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be misrepresented
  as being the original software.

  3. This notice may not be removed or altered from any source distribution.
=======
## Plataformia

## ⭐️ Descrição

"Plataformia" é um jogo de plataforma envolvente e desafiador, criado na linguagem C, que combina elementos clássicos de jogos como "Donkey Kong (1981)" e "Dino T-Rex". O jogo se desenrola em quatro telas distintas:

### Tela 1 
Início do Jogo: O jogador navega por plataformas enfrentando obstáculos e coletando recompensas espalhadas. Esta fase introduz a mecânica básica do jogo, preparando o jogador para desafios futuros.

### Tela 2 
Enfrentando o Boss: A segunda fase é uma versão intensificada da primeira, com os mesmos obstáculos, mas agora o jogador deve enfrentar um boss no topo da tela. O boss se move de um lado para o outro, enquanto o jogador coleta recompensas e busca espadas nas plataformas para atacá-lo.

### Tela 3
Desafios Mortais: Esta fase apresenta um novo background e plataformas que representam um perigo mortal — se o jogador pisar nelas, morre. Os obstáculos são diferentes, mas o objetivo de coletar recompensas permanece, exigindo precisão e atenção.

### Tela 4 
Fim de Jogo: A fase final exibe a pontuação do jogador, acompanhada de um botão para reiniciar ou sair.

## ⚪️ Objetivo do jogo

O jogador precisa coletar recompensas ao longo das 3 fases para obter a sua pontuação final, sendo o melhor jogador o que obteve mais bolinhas. Ele apenas conseguirá a maior quantidade de bolinhas possível caso passe por essas fases, enfrentando os obstáculos e derrotando o boss.

### ▫ Recompensas

### 🔺 Obstáculos

### Plataformas mortais

### 🟦 Boss não estático

## ⌨️ Como jogar

O jogo possui 2 "personagens" principais:

O jogador: 🟢
O inimigo: 🟦

## ⚙️ Executando o jogo

Para executar o Plataformia, siga essas etapas:

1. Clone esse repositório em sua máquina:
```
git clone https://github.com/claranevess/Plataformia
```
2. Entre na pasta do repositório
```
cd Plataformia
```
3.  Compile os arquivos: 
4. Rode o executável do jogo: 
5. Divirta-se jogando Plataformia com seus amigos!

## Membros

- **Gabriel Araújo** - <a href="mailto:bielaraujo578@gmail.com">📧</a> - <a href="https://br.linkedin.com/in/gabriel-ara%C3%BAjo-bb37792b0"><img src="https://upload.wikimedia.org/wikipedia/commons/c/ca/LinkedIn_logo_initials.png" width="20"></a>
- **Aline Amâncio** - <a href="mailto:afa3@cesar.school">📧</a> - <a href="https://www.linkedin.com/in/aline-amancio-23a6b9247/"><img src="https://upload.wikimedia.org/wikipedia/commons/c/ca/LinkedIn_logo_initials.png" width="20"></a>
- **Clara Neves** - <a href="mailto:mcsan2cesar.school">📧</a> - <a href="https://www.linkedin.com/in/claranevess/"><img src="https://upload.wikimedia.org/wikipedia/commons/c/ca/LinkedIn_logo_initials.png" width="20"></a>

<div style="display: flex; align-items: center; justify-content: center; flex-wrap: wrap; gap: 10px;">
    <a href="https://github.com/GabrielAraujo578">
        <img src="https://avatars.githubusercontent.com/u/183439754?v=4" style="border-radius: 50%; width: 100px; height: 100px;">
    </a>
    <a href="https://github.com/afline">
        <img src="https://avatars.githubusercontent.com/u/167882901?v=4" style="border-radius: 50%; width: 100px; height: 100px;">
    </a>
    <a href="https://github.com/claranevess">
        <img src="https://avatars.githubusercontent.com/u/166565110?v=4" style="border-radius: 50%; width: 100px; height: 100px;">
    </a>
</div>
>>>>>>> 4987487079313bf99cafffc74587ad564bdc8ac8
