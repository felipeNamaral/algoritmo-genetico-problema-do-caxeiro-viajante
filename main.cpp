#include "raylib.h"
#include <array>
#include "cidades.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>


#define numCidades 50
#define populacao 300
#define maximogeracao 3500
#define taxaMutacao 0.05f
#define tamanhoTorneio 3
#define elitismo 8
#define limiteEstagnacao 400

struct individuo  
{
    std::array<int, numCidades> rota;
    float fitness;

};

struct AnimacaoRota {
    individuo melhor{};
    double inicio = 0.0;
    bool ativa = false;
};

// Desenha o caminho aos poucos, incluindo o retorno para a primeira cidade.
// Chame a cada quadro, antes de desenhar as cidades.
void animaRota(const AnimacaoRota& animacao,
               const std::array<Cidades, numCidades>& cidades, Color cor,float velocidade) {
    if (!animacao.ativa) return;
    float restante = velocidade <= 0.0f
    ? INFINITY
    : static_cast<float>(GetTime() - animacao.inicio) * velocidade;

    for (int i = 0; i < numCidades; i++) {
        const Cidades& origem = cidades[animacao.melhor.rota[i]];
        const Cidades& destino = cidades[animacao.melhor.rota[(i + 1) % numCidades]];
        Vector2 inicio{origem.getX(), origem.getY()};
        Vector2 fim{destino.getX(), destino.getY()};
        float dx = fim.x - inicio.x;
        float dy = fim.y - inicio.y;
        float distancia = std::sqrt(dx * dx + dy * dy);

        if (restante >= distancia) {
            DrawLineEx(inicio, fim, 4.0f, cor);
            restante -= distancia;
        } else {
            float progresso = restante / distancia;
            Vector2 ponta{inicio.x + dx * progresso, inicio.y + dy * progresso};
            DrawLineEx(inicio, ponta, 4.0f, cor);
            DrawCircleV(ponta, 7.0f, cor);
            break;
        }
    }

    const Cidades& primeira = cidades[animacao.melhor.rota[0]];
    DrawCircleLines(static_cast<int>(primeira.getX()),
                    static_cast<int>(primeira.getY()), 25.0f, cor);
}


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

// Realiza um torneio selecionando aleatoriamente 'k' indivíduos e retorna o de menor fitness
individuo torneio(const std::array<individuo, populacao>& rotas, int k = tamanhoTorneio) {
    int melhorIdx = rand() % populacao;
    for (int i = 1; i < k; i++) {
        int sorteado = rand() % populacao;
        if (rotas[sorteado].fitness < rotas[melhorIdx].fitness) {
            melhorIdx = sorteado;
        }
    }
    return rotas[melhorIdx];
}

// Seleção por torneio: seleciona os 2 pais disputando torneios independentes
void selecao(const std::array<individuo, populacao>* rotas, individuo* pai1, individuo* pai2) {
    *pai1 = torneio(*rotas);
    *pai2 = torneio(*rotas);
}

// Retorna o melhor indivíduo da população (menor distância/fitness)
individuo obterMelhor(const std::array<individuo, populacao>& rotas) {
    individuo melhor = rotas[0];
    for (int i = 1; i < populacao; i++) {
        if (rotas[i].fitness < melhor.fitness) {
            melhor = rotas[i];
        }
    }
    return melhor;
}
 

individuo cruzamento(individuo pai1, individuo pai2) {
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

    return filho;
}

void mutacao(individuo& filho) {
    float sorteio = static_cast<float>(rand()) / (RAND_MAX + 1.0);

    if (sorteio < taxaMutacao) {
        int pos1 = rand() % numCidades;
        int pos2;

        do {
            pos2 = rand() % numCidades;
        } while (pos1 == pos2);

        std::swap(filho.rota[pos1], filho.rota[pos2]);
    }
}


void elitismofunc(std::array<individuo, populacao>* novaPopulacao, std::array<individuo, populacao> rotas){


    std::sort(rotas.begin(), rotas.end(), [](const individuo& a, const individuo& b) {
        return a.fitness < b.fitness;
    });

    for(int i = 0; i < elitismo; i++){
        (*novaPopulacao)[i] = rotas[i];
    }
}



individuo buscaGenetica(std::array<Cidades, numCidades> cidades, const char* tipoBusca = "Cidades Aleatorias", const char* tag = "BUSCA A", int idExecucao = 1){
    
    // gera rotas da pupulação 
    std::array<individuo, populacao> rotas; 
    std::array<individuo, populacao> novaPopulacao;

    individuo melhorGlobal;
    

    int totalMelhorias = 0;
    int geracoesExecutadas = 0;
    bool estagnou = false;

    printf("\n==================================================\n");
    printf("[%s #%d] Iniciando busca: %s\n", tag, idExecucao, tipoBusca);
    printf("[%s #%d][CONFIG] Cidades: %d | Populacao: %d | Taxa Mutacao: %.2f | Elitismo: %d | Max Geracoes: %d | Limite Estagnacao: %d\n",
           tag, idExecucao, numCidades, populacao, taxaMutacao, elitismo, maximogeracao, limiteEstagnacao);
    printf("==================================================\n");
    fflush(stdout);

    // inicializa a população com rotas aleatórias
    for(int i = 0; i < populacao; i++){
        IniciaPopulacao(&rotas[i].rota);
    }
    //avalia as rotas e calcula o fitness
    avaliafitiness(&rotas, cidades);
    melhorGlobal = obterMelhor(rotas);


    // loop de gerações
    for(int i = 0, geracoesSemMelhora = 0; i < maximogeracao && geracoesSemMelhora < limiteEstagnacao; i++, geracoesSemMelhora++){

        //elitismo: copia o melhor individuo para a nova população
        elitismofunc(&novaPopulacao, rotas);

        for(int j = elitismo; j < populacao; j++){
            individuo pai1, pai2;
            selecao(&rotas, &pai1, &pai2);
            novaPopulacao[j] = cruzamento(pai1, pai2);
            mutacao(novaPopulacao[j]);
        }


        rotas = novaPopulacao;

        avaliafitiness(&rotas, cidades);

        individuo melhor = obterMelhor(rotas);
        if(melhor.fitness < melhorGlobal.fitness){
            if(totalMelhorias == 0){
                printf("[%s][MELHORIA] Geracao %d | Inicializacao do melhor global: %.2f\n",
                       tag, i + 1, melhor.fitness);
            } else {
                float distanciaAnterior = melhorGlobal.fitness;
                float progresso = distanciaAnterior - melhor.fitness;
                printf("[%s][MELHORIA] Geracao %d | Anterior: %.2f -> Nova: %.2f | Progresso: %.2f\n",
                       tag, i + 1, distanciaAnterior, melhor.fitness, progresso);
            }
            totalMelhorias++;
            melhorGlobal = melhor;
            geracoesSemMelhora = -1; 
        }

        geracoesExecutadas = i + 1;
        int semMelhoraAtual = (geracoesSemMelhora == -1) ? 0 : (geracoesSemMelhora + 1);

        if((i + 1) % 10 == 0){
            printf("[%s][GERACAO] Geracao %d | Melhor Atual: %.2f | Melhor Global: %.2f | Sem Melhora: %d\n",
                   tag, i + 1, melhor.fitness, melhorGlobal.fitness, semMelhoraAtual);
        }

        if(geracoesSemMelhora + 1 >= limiteEstagnacao){
            estagnou = true;
        }

        AnimacaoRota animacao;
        animacao.melhor = melhor;
        animacao.inicio = GetTime(); 
        animacao.ativa = true;

        BeginDrawing();
        ClearBackground(Color{20, 20, 30, 255});

        for (const Cidades& cidade : cidades) {
            DrawCircle(cidade.getX(), cidade.getY(), 20, cidade.getCor());

            DrawText(TextFormat("%d", cidade.getId()),
                    cidade.getX() - 6, cidade.getY() - 10,
                    20, BLACK);
        }

        DrawText(TextFormat("Geracao: %d | Distancia: %.2f",
                            i + 1, melhor.fitness),
                20, 20, 20, WHITE);
        animaRota(animacao, cidades, GREEN, 0.0f);

        EndDrawing();
    }

    if(estagnou){
        printf("[%s][ESTAGNACAO] Limite de %d geracoes sem melhoria atingido na geracao %d. Melhor fitness: %.2f\n",
               tag, limiteEstagnacao, geracoesExecutadas, melhorGlobal.fitness);
    }

    printf("[%s][FIM] Motivo: %s | Total geracoes: %d | Melhor distancia global: %.2f | Total melhorias: %d\n",
           tag, (estagnou ? "Estagnacao" : "Limite maximo de geracoes atingido"),
           geracoesExecutadas, melhorGlobal.fitness, totalMelhorias);
    printf("==================================================\n\n");
    fflush(stdout);

    return melhorGlobal;
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



    AnimacaoRota animacaoAleatoria;
    AnimacaoRota animacaoBenchmark;

    int execucaoA = 0;
    int execucaoB = 0;

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_A)) {
            execucaoA++;
            animacaoAleatoria.melhor = buscaGenetica(cidades, "Cidades Aleatorias", "BUSCA A", execucaoA);
            animacaoAleatoria.inicio = GetTime();
            animacaoAleatoria.ativa = true;
        }
        if (IsKeyPressed(KEY_B)) {
            execucaoB++;
            animacaoBenchmark.melhor = buscaGenetica(cidadesBenchmark, "Circulo (Benchmark)", "BUSCA B", execucaoB);
            animacaoBenchmark.inicio = GetTime();
            animacaoBenchmark.ativa = true;
        }

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
        DrawText("A: buscar e animar rota", 20, 60, 20, LIGHTGRAY);
        DrawText("B: buscar e animar rota", meio + 20, 60, 20, LIGHTGRAY);

        animaRota(animacaoAleatoria, cidades, GREEN, 350.0f);
        animaRota(animacaoBenchmark, cidadesBenchmark, SKYBLUE, 350.0f);
        if (animacaoAleatoria.ativa) {
            DrawText(TextFormat("Distancia: %.2f", animacaoAleatoria.melhor.fitness),
                     20, 90, 20, GREEN);
        }
        if (animacaoBenchmark.ativa) {
            DrawText(TextFormat("Distancia: %.2f", animacaoBenchmark.melhor.fitness),
                     meio + 20, 90, 20, SKYBLUE);
        }

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

        EndDrawing();
    }



    CloseWindow();
    return 0;
}
