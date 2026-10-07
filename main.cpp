#include "raylib.h"
#include <array>
#include "cidades.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define numCidades 10
#define populacao 100
#define maximogeracao 1000

struct individuo  
{
    std::array<int, numCidades> rota;
    float fitness;

};


void IniciaPopulacao(std::array<int, numCidades>* rota) {
    for (int i = 0; i < numCidades; i++) {
        (*rota)[i] = i;
    }

    for (int i = numCidades - 1; i > 0; i--) {
        int sorteado = rand() % (i + 1);
        std::swap((*rota)[i], (*rota)[sorteado]);
    }
}

void avaliafitiness(std::array<individuo, populacao>* rotas,std::array<Cidades, numCidades> cidades){
    
    for(int i = 0; i < populacao; i++){
        (*rotas)[i].fitness = 0;
        for(int j = 0; j < numCidades; j++){
            float dx = cidades[(*rotas)[i].rota[j]].getX() - cidades[(*rotas)[i].rota[(j + 1) % numCidades]].getX();
            float dy = cidades[(*rotas)[i].rota[j]].getY() - cidades[(*rotas)[i].rota[(j + 1) % numCidades]].getY();
            (*rotas)[i].fitness += std::sqrt(dx * dx + dy * dy);
        }
    }
}

void selecao(std::array<individuo, populacao>* rotas, individuo* pai1, individuo* pai2){
    // Seleciona os dois melhores indivíduos da população
    *pai1 = (*rotas)[0];
    *pai2 = (*rotas)[1];
    if (pai2->fitness < pai1->fitness) {
        std::swap(*pai1, *pai2);
    }
    for (int i = 2; i < populacao; i++) {
        if ((*rotas)[i].fitness < pai1->fitness) {
            *pai2 = *pai1;
            *pai1 = (*rotas)[i];
        } else if ((*rotas)[i].fitness < pai2->fitness) {
            *pai2 = (*rotas)[i];
        }
    }    
}
 

individuo cruzamento(individuo pai1, individuo pai2, std::array<Cidades, numCidades> cidades) {
    individuo filho{};
    std::array<bool, numCidades> usada{};

    // Corte entre 1 e numCidades - 1
    int corte = 1 + rand() % (numCidades - 1);

    // Copia o inicio do pai 1
    for (int i = 0; i < corte; i++) {
        int cidade = pai1.rota[i];

        filho.rota[i] = cidade;
        usada[cidade] = true;
    }

    // Completa na ordem do pai 2, sem repetir cidades
    int posicao = corte;

    for (int i = 0; i < numCidades; i++) {
        int cidade = pai2.rota[i];

        if (!usada[cidade]) {
            filho.rota[posicao] = cidade;
            usada[cidade] = true;
            posicao++;
        }
    }

    for (int i = 0; i < numCidades; i++) {
        float dx = cidades[filho.rota[i]].getX() - cidades[filho.rota[(i + 1) % numCidades]].getX();
        float dy = cidades[filho.rota[i]].getY() - cidades[filho.rota[(i + 1) % numCidades]].getY();
        filho.fitness += std::sqrt(dx * dx + dy * dy);
    }
    return filho;
}


individuo buscaGenetica(std::array<Cidades, numCidades> cidades){
    
    // gera rotas da pupulação 
    std::array<individuo, populacao> rotas; 
    std::array<individuo, populacao> novaPopulacao;

    for(int i = 0; i < populacao; i++){
        IniciaPopulacao(&rotas[i].rota);
    }

    for(int i = 0; i<maximogeracao;i++){
        //avalia as rotas e calcula o fitness
        avaliafitiness(&rotas, cidades);

        individuo pai1,pai2;

        selecao(&rotas, &pai1, &pai2);

        for(int i = 0; i < populacao; i++){
            novaPopulacao[i] = cruzamento(pai1, pai2, cidades);
        }
        rotas = novaPopulacao;
    }

    // Avalia a ultima geracao e retorna seu melhor individuo
    avaliafitiness(&rotas, cidades);

    individuo melhor, segundoMelhor;
    selecao(&rotas, &melhor, &segundoMelhor);

    return melhor;
}

int main()
{
    InitWindow(1920, 1024, "Trabalho 2 - Caixeiro-viajante");
    SetTargetFPS(60);

    std::array<Cidades, numCidades> cidades;
    std::array<Cidades, numCidades> cidadesBenchmark;
    srand(static_cast<unsigned int>(time(NULL)));

    const int larguraPainel = GetScreenWidth() / 2;
    // Centro do painel direito
    const float centroX = GetScreenWidth() * 0.75f;
    const float centroY = GetScreenHeight() * 0.5f;
    const float raio = std::fmin( GetScreenWidth() / 2.0f,static_cast<float>(GetScreenHeight()) ) * 0.25f;



    // Cidades aleatorias no centro do painel esquerdo, com margens de 25%.
    for(int i = 0; i < numCidades; i++) {
        float x = GetRandomValue(larguraPainel / 4, larguraPainel * 3 / 4);
        float y = GetRandomValue(GetScreenHeight() / 4, GetScreenHeight() * 3 / 4);
        cidades[i] = Cidades(x, y, i);
    }
    
    // cidades para o benchmark, no painel direito, com margens de 25%.
    for (int i = 0; i < numCidades; i++){

        float angulo = 2.0f * PI * i / numCidades;
        float x = centroX + raio * std::cos(angulo);
        float y = centroY + raio * std::sin(angulo);

        cidadesBenchmark[i] = Cidades(x, y, i);
    }



    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(Color{20, 20, 30, 255});

        int meio = GetScreenWidth() / 2;

        // Metade direita
        DrawRectangle(
            meio, 0,
            GetScreenWidth() - meio, GetScreenHeight(),
            Color{35, 35, 50, 255}
        );

        // Divisoria e titulos
        DrawLine(meio, 0, meio, GetScreenHeight(), GRAY);
        DrawText("Aleatorio", 20, 20, 30, WHITE);
        DrawText("Benchmark", meio + 20, 20, 30, WHITE);

        for(int i = 0; i < numCidades; i++) {
            // Desenhar as cidades
            DrawCircle(cidades[i].getX(), cidades[i].getY(), 20, cidades[i].getCor());
            DrawText(TextFormat("%d", cidades[i].getId()),cidades[i].getX()-6, cidades[i].getY()-10, 20, BLACK);
        }

        for(int i = 0; i < numCidades; i++) {
            // Desenhar as cidades
            DrawCircle(cidadesBenchmark[i].getX(), cidadesBenchmark[i].getY(), 20, cidadesBenchmark[i].getCor());
            DrawText(TextFormat("%d", cidadesBenchmark[i].getId()),cidadesBenchmark[i].getX()-6, cidadesBenchmark[i].getY()-10, 20, BLACK);
        }        


        if (IsKeyPressed(KEY_A)){
            individuo melhor = buscaGenetica(cidades);
            
        }

        if(IsKeyPressed(KEY_B)){
            individuo melhor = buscaGenetica(cidadesBenchmark);
        }




        EndDrawing();
    }



    CloseWindow();
    return 0;
}
