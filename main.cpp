#include "raylib.h"
#include <array>
#include <vector>
#include "cidades.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

#define numCidades 20
#define populacao 200
#define maximogeracao 1000
#define taxaMutacao 0.1f
#define tamanhoTorneio 3
#define elitismo 6
#define limiteEstagnacao 200
#define raio 200

struct individuo
{
    std::array<int, numCidades> rota;
    float fitness;
};

struct AnimacaoRota
{
    individuo melhor{};
    double inicio = 0.0;
    bool ativa = false;
};

// Desenha o caminho aos poucos, incluindo o retorno para a primeira cidade.
// Chame a cada quadro, antes de desenhar as cidades.
void animaRota(const AnimacaoRota &animacao,
               const std::array<Cidades, numCidades> &cidades, Color cor, float velocidade)
{
    if (!animacao.ativa)
        return;
    float restante = velocidade <= 0.0f
                         ? INFINITY
                         : static_cast<float>(GetTime() - animacao.inicio) * velocidade;

    for (int i = 0; i < numCidades; i++)
    {
        const Cidades &origem = cidades[animacao.melhor.rota[i]];
        const Cidades &destino = cidades[animacao.melhor.rota[(i + 1) % numCidades]];
        Vector2 inicio{origem.getX(), origem.getY()};
        Vector2 fim{destino.getX(), destino.getY()};
        float dx = fim.x - inicio.x;
        float dy = fim.y - inicio.y;
        float distancia = std::sqrt(dx * dx + dy * dy);

        if (restante >= distancia)
        {
            DrawLineEx(inicio, fim, 4.0f, cor);
            restante -= distancia;
        }
        else
        {
            float progresso = restante / distancia;
            Vector2 ponta{inicio.x + dx * progresso, inicio.y + dy * progresso};
            DrawLineEx(inicio, ponta, 4.0f, cor);
            DrawCircleV(ponta, 7.0f, cor);
            break;
        }
    }

    const Cidades &primeira = cidades[animacao.melhor.rota[0]];
    DrawCircleLines(static_cast<int>(primeira.getX()),
                    static_cast<int>(primeira.getY()), 25.0f, cor);
}

// Desenha a tela principal completa (os dois lados, divisórias, rotas e cidades)
void desenharTelaPrincipal(const std::array<Cidades, numCidades> &cidadesAleatorias,
                           const std::array<Cidades, numCidades> &cidadesBenchmark,
                           const AnimacaoRota &animAleatoria,
                           const AnimacaoRota &animBenchmark,
                           float velA = 350.0f,
                           float velB = 350.0f,
                           const std::vector<float> &histA = {},
                           const std::vector<float> &histB = {},
                           double tempoA = 0.0,
                           double tempoB = 0.0,
                           const char *tagBusca = nullptr,
                           int geracaoAtual = 0)
{
    ClearBackground(Color{20, 20, 30, 255});

    int meio = GetScreenWidth() / 2;

    // Metade direita com tom ligeiramente diferenciado
    DrawRectangle(
        meio, 0,
        GetScreenWidth() - meio, GetScreenHeight(),
        Color{35, 35, 50, 255});

    // Divisoria e titulos
    DrawLine(meio, 0, meio, GetScreenHeight(), GRAY);
    DrawText("Aleatorio", 20, 20, 30, WHITE);
    DrawText("Benchmark", meio + 20, 20, 30, WHITE);
    DrawText(TextFormat("Tempo: %.2f s", tempoA), 20, 120, 20, LIGHTGRAY);
    DrawText(TextFormat("Tempo: %.2f s", tempoB), meio + 20, 120, 20, LIGHTGRAY);
    DrawText("A: buscar e animar rota", 20, 60, 20, LIGHTGRAY);
    DrawText("B: buscar e animar rota", meio + 20, 60, 20, LIGHTGRAY);

    // Desenha as rotas
    animaRota(animAleatoria, cidadesAleatorias, GREEN, velA);
    animaRota(animBenchmark, cidadesBenchmark, SKYBLUE, velB);

    if (animAleatoria.ativa)
    {
        DrawText(TextFormat("Distancia: %.2f", animAleatoria.melhor.fitness),
                 20, 90, 20, GREEN);
    }
    if (animBenchmark.ativa)
    {
        DrawText(TextFormat("Distancia: %.2f", animBenchmark.melhor.fitness),
                 meio + 20, 90, 20, SKYBLUE);
    }

    // Desenha as cidades aleatorias (painel esquerdo)
    for (int i = 0; i < numCidades; i++)
    {
        DrawCircle(cidadesAleatorias[i].getX(), cidadesAleatorias[i].getY(), 20, cidadesAleatorias[i].getCor());
        DrawText(TextFormat("%d", cidadesAleatorias[i].getId()),
                 cidadesAleatorias[i].getX() - 6, cidadesAleatorias[i].getY() - 10, 20, BLACK);
    }

    // Desenha as cidades do benchmark (painel direito)
    for (int i = 0; i < numCidades; i++)
    {
        DrawCircle(cidadesBenchmark[i].getX(), cidadesBenchmark[i].getY(), 20, cidadesBenchmark[i].getCor());
        DrawText(TextFormat("%d", cidadesBenchmark[i].getId()),
                 cidadesBenchmark[i].getX() - 6, cidadesBenchmark[i].getY() - 10, 20, BLACK);
    }

    // Barra informativa durante a execucao da busca genetica
    if (tagBusca != nullptr)
    {
        DrawRectangle(0, GetScreenHeight() - 20, GetScreenWidth(), 20, Color{15, 15, 25, 230});
        DrawLine(0, GetScreenHeight() - 20, GetScreenWidth(), GetScreenHeight() - 20, DARKGRAY);
        DrawText(TextFormat("[%s] Geracao: %d/%d ",
                            tagBusca, geracaoAtual, maximogeracao),
                 15, GetScreenHeight() - 15, 15, YELLOW);
    }

    // ==========================================
    // Gráfico do Lado A (Aleatório)
    // ==========================================
    {
        int inicioX = 60;
        int fimX = meio - 40;
        int baseY = GetScreenHeight() - 70;
        int topoY = GetScreenHeight() - 300;

        // Eixo X (Gerações) e Eixo Y (Fitness) do Lado A
        DrawLine(inicioX, baseY, fimX, baseY, DARKGRAY);
        DrawLine(inicioX, baseY, inicioX, topoY, DARKGRAY);
        DrawText("Fitness", inicioX, topoY - 25, 20, DARKGRAY);
        DrawText("Geracoes", fimX - 95, baseY + 10, 20, DARKGRAY);

        // Plota a curva de evolução da busca A
        if (histA.size() >= 2)
        {
            float maxFit = histA[0];
            float minFit = histA[0];
            for (float f : histA)
            {
                if (f > maxFit) maxFit = f;
                if (f < minFit) minFit = f;
            }
            if (maxFit == minFit) maxFit += 1.0f;

            float larg = fimX - inicioX;
            float alt = (baseY - topoY) - 20;
            float totalPontos = (float)(histA.size() - 1);

            for (size_t j = 1; j < histA.size(); j++)
            {
                float xAntigo = inicioX + ((float)(j - 1) / totalPontos) * larg;
                float yAntigo = (baseY - 10) - ((histA[j - 1] - minFit) / (maxFit - minFit)) * alt;

                float xNovo = inicioX + ((float)j / totalPontos) * larg;
                float yNovo = (baseY - 10) - ((histA[j] - minFit) / (maxFit - minFit)) * alt;

                DrawLineEx(Vector2{xAntigo, yAntigo}, Vector2{xNovo, yNovo}, 3.0f, GREEN);
            }
        }
    }

    // ==========================================
    // Gráfico do Lado B (Benchmark)
    // ==========================================
    {
        int inicioX = meio + 60;
        int fimX = GetScreenWidth() - 40;
        int baseY = GetScreenHeight() - 70;
        int topoY = GetScreenHeight() - 300;

        // Eixo X (Gerações) e Eixo Y (Fitness) do Lado B
        DrawLine(inicioX, baseY, fimX, baseY, DARKGRAY);
        DrawLine(inicioX, baseY, inicioX, topoY, DARKGRAY);
        DrawText("Fitness", inicioX, topoY - 25, 20, DARKGRAY);
        DrawText("Geracoes", fimX - 95, baseY + 10, 20, DARKGRAY);

        // Plota a curva de evolução da busca B
        if (histB.size() >= 2)
        {
            float maxFit = histB[0];
            float minFit = histB[0];
            for (float f : histB)
            {
                if (f > maxFit) maxFit = f;
                if (f < minFit) minFit = f;
            }
            if (maxFit == minFit) maxFit += 1.0f;

            float larg = fimX - inicioX;
            float alt = (baseY - topoY) - 20;
            float totalPontos = (float)(histB.size() - 1);

            for (size_t j = 1; j < histB.size(); j++)
            {
                float xAntigo = inicioX + ((float)(j - 1) / totalPontos) * larg;
                float yAntigo = (baseY - 10) - ((histB[j - 1] - minFit) / (maxFit - minFit)) * alt;

                float xNovo = inicioX + ((float)j / totalPontos) * larg;
                float yNovo = (baseY - 10) - ((histB[j] - minFit) / (maxFit - minFit)) * alt;

                DrawLineEx(Vector2{xAntigo, yAntigo}, Vector2{xNovo, yNovo}, 3.0f, SKYBLUE);
            }
        }
    }
}

//////////////////////////////////////////////busca//////////////////////////////////////////////////////////////

void IniciaPopulacao(std::array<int, numCidades> *rota)
{
    for (int i = 0; i < numCidades; i++)
    {
        (*rota)[i] = i;
    }

    for (int i = numCidades - 1; i > 0; i--)
    {
        int sorteado = rand() % (i + 1);
        std::swap((*rota)[i], (*rota)[sorteado]);
    }
}

void avaliafitiness(std::array<individuo, populacao> *rotas, const std::array<Cidades, numCidades> &cidades)
{

    for (int i = 0; i < populacao; i++)
    {
        (*rotas)[i].fitness = 0;
        for (int j = 0; j < numCidades; j++)
        {
            float dx = cidades[(*rotas)[i].rota[j]].getX() - cidades[(*rotas)[i].rota[(j + 1) % numCidades]].getX();
            float dy = cidades[(*rotas)[i].rota[j]].getY() - cidades[(*rotas)[i].rota[(j + 1) % numCidades]].getY();
            (*rotas)[i].fitness += std::sqrt(dx * dx + dy * dy);
        }
    }
}

// Realiza um torneio selecionando aleatoriamente 'k' indivíduos e retorna o de menor fitness
individuo torneio(const std::array<individuo, populacao> &rotas, int k = tamanhoTorneio)
{
    int melhorIdx = rand() % populacao;
    for (int i = 1; i < k; i++)
    {
        int sorteado = rand() % populacao;
        if (rotas[sorteado].fitness < rotas[melhorIdx].fitness)
        {
            melhorIdx = sorteado;
        }
    }
    return rotas[melhorIdx];
}

// Seleção por torneio: seleciona os 2 pais disputando torneios independentes
void selecao(const std::array<individuo, populacao> *rotas, individuo *pai1, individuo *pai2)
{
    *pai1 = torneio(*rotas);
    *pai2 = torneio(*rotas);
}

// Retorna o melhor indivíduo da população (menor distância/fitness)
individuo obterMelhor(const std::array<individuo, populacao> &rotas)
{
    individuo melhor = rotas[0];
    for (int i = 1; i < populacao; i++)
    {
        if (rotas[i].fitness < melhor.fitness)
        {
            melhor = rotas[i];
        }
    }
    return melhor;
}

individuo cruzamento(individuo pai1, individuo pai2)
{
    individuo filho{};
    std::array<bool, numCidades> usada{};

    // Corte entre 1 e numCidades - 1
    int corte = 1 + rand() % (numCidades - 1);

    // Copia o inicio do pai 1
    for (int i = 0; i < corte; i++)
    {
        int cidade = pai1.rota[i];

        filho.rota[i] = cidade;
        usada[cidade] = true;
    }

    // Completa na ordem do pai 2, sem repetir cidades
    int posicao = corte;

    for (int i = 0; i < numCidades; i++)
    {
        int cidade = pai2.rota[i];

        if (!usada[cidade])
        {
            filho.rota[posicao] = cidade;
            usada[cidade] = true;
            posicao++;
        }
    }

    return filho;
}

void mutacao(individuo &filho)
{
    float sorteio = static_cast<float>(rand()) / (RAND_MAX + 1.0);

    if (sorteio < taxaMutacao)
    {
        int pos1 = rand() % numCidades;
        int pos2;

        do
        {
            pos2 = rand() % numCidades;
        } while (pos1 == pos2);

        std::swap(filho.rota[pos1], filho.rota[pos2]);
    }
}

void elitismofunc(std::array<individuo, populacao> *novaPopulacao, std::array<individuo, populacao> rotas)
{

    std::sort(rotas.begin(), rotas.end(), [](const individuo &a, const individuo &b)
              { return a.fitness < b.fitness; });

    for (int i = 0; i < elitismo; i++)
    {
        (*novaPopulacao)[i] = rotas[i];
    }
}

individuo buscaGenetica(
    const std::array<Cidades, numCidades> &cidadesA,
    const std::array<Cidades, numCidades> &cidadesB,
    AnimacaoRota animA,
    AnimacaoRota animB,
    bool buscaNoLadoA,
    const char *tipoBusca,
    const char *tag,
    int idExecucao,
    std::vector<float> &historicoFitness)
{
    historicoFitness.clear();



    // Seleciona o conjunto de cidades correspondente ao lado escolhido
    const std::array<Cidades, numCidades> &cidades = buscaNoLadoA ? cidadesA : cidadesB;


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
    for (int i = 0; i < populacao; i++)
    {
        IniciaPopulacao(&rotas[i].rota);
    }
    // avalia as rotas e calcula o fitness
    avaliafitiness(&rotas, cidades);
    //primera melhor rota global
    melhorGlobal = obterMelhor(rotas);
    historicoFitness.push_back(melhorGlobal.fitness);

    // loop de gerações
    for (int i = 0, geracoesSemMelhora = 0; i < maximogeracao && geracoesSemMelhora < limiteEstagnacao; i++, geracoesSemMelhora++)
    {

        // elitismo: copia o melhor individuo para a nova população
        elitismofunc(&novaPopulacao, rotas);

        for (int j = elitismo; j < populacao; j++)
        {
            individuo pai1, pai2;
            selecao(&rotas, &pai1, &pai2);
            novaPopulacao[j] = cruzamento(pai1, pai2);
            mutacao(novaPopulacao[j]);
        }

        rotas = novaPopulacao;

        avaliafitiness(&rotas, cidades);

        individuo melhor = obterMelhor(rotas);
        if (melhor.fitness < melhorGlobal.fitness)
        {
            if (totalMelhorias == 0)
            {
                printf("[%s][MELHORIA] Geracao %d | Inicializacao do melhor global: %.2f\n",
                       tag, i + 1, melhor.fitness);
            }
            else
            {
                float distanciaAnterior = melhorGlobal.fitness;
                float progresso = distanciaAnterior - melhor.fitness;
                printf("[%s][MELHORIA] Geracao %d | Anterior: %.2f -> Nova: %.2f | Progresso: %.2f\n",
                       tag, i + 1, distanciaAnterior, melhor.fitness, progresso);
            }
            totalMelhorias++;
            melhorGlobal = melhor;
            geracoesSemMelhora = -1;
        }

        historicoFitness.push_back(melhorGlobal.fitness);

        geracoesExecutadas = i + 1;
        int semMelhoraAtual = (geracoesSemMelhora == -1) ? 0 : (geracoesSemMelhora + 1);

        if ((i + 1) % 10 == 0)
        {
            printf("[%s][GERACAO] Geracao %d | Melhor Atual: %.2f | Melhor Global: %.2f | Sem Melhora: %d\n",
                   tag, i + 1, melhor.fitness, melhorGlobal.fitness, semMelhoraAtual);
        }

        if (geracoesSemMelhora + 1 >= limiteEstagnacao)
        {
            estagnou = true;
        }

        // Atualiza a animacao correspondente ao lado que esta buscando
        AnimacaoRota animTempA = animA;
        AnimacaoRota animTempB = animB;

        if (buscaNoLadoA)
        {
            animTempA.melhor = melhor;
            animTempA.ativa = true;
        }
        else
        {
            animTempB.melhor = melhor;
            animTempB.ativa = true;
        }

        // Renderiza a interface principal completa com a geracao atual
        BeginDrawing();
        desenharTelaPrincipal(
            cidadesA, cidadesB,
            animTempA, animTempB,
            0.0f, 0.0f,
            buscaNoLadoA ? historicoFitness : std::vector<float>{},
            buscaNoLadoA ? std::vector<float>{} : historicoFitness,0.0,0.0,
            tag, i + 1);
        EndDrawing();

        if (WindowShouldClose())
        {
            break;
        }
    }

    if (estagnou)
    {
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

////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{

    InitWindow(1920, 1080, "Trabalho 2 - Caixeiro-viajante");

    SetTargetFPS(60);

    std::array<Cidades, numCidades> cidades;
    std::array<Cidades, numCidades> cidadesBenchmark;
    std::vector<float> historicoA;
    std::vector<float> historicoB;

    srand(static_cast<unsigned int>(time(NULL)));

    const int larguraPainel = GetScreenWidth() / 2;
    // Centro do painel direito
    const float centroX = GetScreenWidth() * 0.75f;
    const float centroY = GetScreenHeight() * 0.35f;
    // const float raio = std::fmin(GetScreenWidth() / 2.0f, static_cast<float>(GetScreenHeight())) * 0.25f;

    // Cidades aleatorias no centro do painel esquerdo, com margens de 25%.
    for (int i = 0; i < numCidades; i++)
    {
        float x = GetRandomValue(larguraPainel / 4, larguraPainel * 3 / 4);
        float y = GetRandomValue(GetScreenHeight() / 6, GetScreenHeight() * 0.65f);
        cidades[i] = Cidades(x, y, i);
    }

    // cidades para o benchmark, no painel direito, com margens de 25%.
    for (int i = 0; i < numCidades; i++)
    {

        float angulo = 2.0f * PI * i / numCidades;
        float x = centroX + raio * std::cos(angulo);
        float y = centroY + raio * std::sin(angulo);

        cidadesBenchmark[i] = Cidades(x, y, i);
    }

    AnimacaoRota animacaoAleatoria;
    AnimacaoRota animacaoBenchmark;

    int execucaoA = 0;
    int execucaoB = 0;

    double tempoA=0,tempoB=0;

    while (!WindowShouldClose())
    {

        if (IsKeyPressed(KEY_A))
        {
            execucaoA++;
            tempoA= GetTime();
            animacaoAleatoria.melhor = buscaGenetica(
                cidades, cidadesBenchmark,
                animacaoAleatoria, animacaoBenchmark,
                true, "Cidades Aleatorias", "BUSCA A", execucaoA,historicoA);
            tempoA= GetTime() - tempoA;
            printf("[BUSCA A] Tempo de execucao: %.2f segundos\n", tempoA);
            animacaoAleatoria.inicio = GetTime();
            animacaoAleatoria.ativa = true;
        }


        if (IsKeyPressed(KEY_B))
        {
            execucaoB++;
            tempoB= GetTime();
            animacaoBenchmark.melhor = buscaGenetica(
                cidades, cidadesBenchmark,
                animacaoAleatoria, animacaoBenchmark,
                false, "Circulo (Benchmark)", "BUSCA B", execucaoB,historicoB);
            tempoB= GetTime() - tempoB;
            printf("[BUSCA B] Tempo de execucao: %.2f segundos\n", tempoB);
            animacaoBenchmark.inicio = GetTime();
            animacaoBenchmark.ativa = true;
        }

        BeginDrawing();
        desenharTelaPrincipal(cidades, cidadesBenchmark, animacaoAleatoria, animacaoBenchmark, 350.0f, 350.0f,historicoA,historicoB,tempoA,tempoB);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
