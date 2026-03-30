#include <iostream>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

//representa um trikit
struct Trikit {
    int valores[3];
};

int num_pecas = 0;
vector<Trikit> pecas;
bool peca_usada[20];

//grelha para simular a mesa (40x40 é espaço de sobra para <= 20 peças)
//guarda um par {id_da_peca, rotacao}
pair<int, int> mesa[40][40];

int pontuacao_maxima = 0;

// GEOMETRIA DA GRELHA TRIANGULAR

//direções: 0 = Direita/Topo, 1 = Fundo/Direita, 2 = Esquerda
// dr[0] e dc[0] são para triângulos cima(linha + coluna == PAR)
// dr[1] e dc[1] são para triângulos baixo (linha + coluna == ÍMPAR)
const int delta_linha[2][3] = { {0, 1, 0}, {-1, 0, 0} };
const int delta_coluna[2][3] = { {1, 0, -1}, {0, 1, -1} };

// Mapeamento de portas opostas:
// Se o triangulo para cima se liga a um traingulo para baixo pela sua porta 0 (Direita), o trinagulo para baixo recebe-o na sua porta 2 (Esquerda).
const int lado_oposto[2][3] = { {2, 0, 1}, {1, 2, 0} };

//devolve a aresta direcionada (x, y) de um trikit, considerando a sua rotação e porta pedida
pair<int, int> obter_aresta(int id_peca, int rotacao, int lado) {
    int indice = (rotacao + lado) % 3;
    int val1 = pecas[id_peca].valores[indice];
    int val2 = pecas[id_peca].valores[(indice + 1) % 3];
    return {val1, val2};
}

//verifica se podemos colocar a peça 'id' com a rotação 'rot' na coordenada (r, c)
//se sim, devolve os pontos ganhos nesta jogada; ->Se não, devolve -1.
int pode_colocar(int linha, int coluna, int id_peca, int rotacao) {
    int tipo_triangulo = (linha + coluna) % 2; // 0 para cima, 1 para baixo
    int vizinhos = 0;
    int pontos = 0;
    
    for (int lado = 0; lado < 3; lado++) {
        int nova_linha = linha + delta_linha[tipo_triangulo][lado];
        int nova_coluna = coluna + delta_coluna[tipo_triangulo][lado];
        
        // Se houver uma peça vizinha nesta direção -> verificar o encaixe
        if (mesa[nova_linha][nova_coluna].first != -1) {
            vizinhos++;
            int id_vizinho = mesa[nova_linha][nova_coluna].first;
            int rotacao_vizinho = mesa[nova_linha][nova_coluna].second;
            
            int porta_vizinho = lado_oposto[tipo_triangulo][lado];
            pair<int, int> minha_aresta = obter_aresta(id_peca, rotacao, lado);
            pair<int, int> aresta_vizinho = obter_aresta(id_vizinho, rotacao_vizinho, porta_vizinho);
            
            //o sentido (x -> y) tem de bater com o sentido inverso (y -> x)
            if (minha_aresta.first == aresta_vizinho.second && minha_aresta.second == aresta_vizinho.first) {
                pontos += minha_aresta.first + minha_aresta.second;
            } else {
                return -1; // ALERTA conflito: há uma peça, mas os números não batem
            }
        }
    }
    
    //tem de se ligar a pelo menos uma peça (exceto a primeira que pomos no tabuleiro, 
    //mas chamamos isto apenas para casas com vizinhos)
    if (vizinhos == 0) return -1;
    
    return pontos;
}

//
//algoritmo de backtracking (DFS)
//

void explorar_jogadas(int pontuacao_atual, set<pair<int,int>> candidatos) {
    //atualiza a pontuação máxima alcançada
    pontuacao_maxima = max(pontuacao_maxima, pontuacao_atual);
    
    if (candidatos.empty()) return;
    
    //tira o primeiro candidato da lista (abordagem de geração de polyominos para evitar loops infinitos)
    auto it = candidatos.begin();
    pair<int,int> posicao = *it;
    candidatos.erase(it);
    
    int linha = posicao.first;
    int coluna = posicao.second;
    
    //opcao 1: Tentar colocar uma peça nesta casa
    for (int i = 0; i < num_pecas; i++) {
        if (!peca_usada[i]) {
            for (int rotacao = 0; rotacao < 3; rotacao++) {
                int pontos = pode_colocar(linha, coluna, i, rotacao);
                
                if (pontos != -1) {
                    // Se encaixa perfeitamente, colocamos na mesa
                    mesa[linha][coluna] = {i, rotacao};
                    peca_usada[i] = true;
                    
                    // Descobrir novos candidatos (espaços vazios à volta da peça que acabamos de pôr)
                    set<pair<int,int>> proximos_candidatos = candidatos;
                    int tipo_triangulo = (linha + coluna) % 2;
                    for (int lado = 0; lado < 3; lado++) {
                        int nova_linha = linha + delta_linha[tipo_triangulo][lado];
                        int nova_coluna = coluna + delta_coluna[tipo_triangulo][lado];
                        if (mesa[nova_linha][nova_coluna].first == -1) {
                            proximos_candidatos.insert({nova_linha, nova_coluna});
                        }
                    }
                    
                    // Chamada recursiva com a nova pontuação e os novos candidatos
                    explorar_jogadas(pontuacao_atual + pontos, proximos_candidatos);
                    
                    //desfazer (Backtrack)
                    peca_usada[i] = false;
                    mesa[linha][coluna] = {-1, -1};
                }
            }
        }
    }
    
    //opcao 2: Não colocar NADA nesta casa nesta ramificação,
    //e passar para o próximo candidato. Isto garante que testamos todas as formas possíveis.
    explorar_jogadas(pontuacao_atual, candidatos);
}

int main() {
    //le os inputs até acabar o ficheiro
    int val_u, val_v, val_w;
    while (cin >> val_u >> val_v >> val_w) {
        pecas.push_back({val_u, val_v, val_w});
    }
    num_pecas = pecas.size();
    
    //iinicar a grelha toda a -1 (vazia)
    for (int linha = 0; linha < 40; linha++) {
        for (int coluna = 0; coluna < 40; coluna++) {
            mesa[linha][coluna] = {-1, -1};
        }
    }
    
    // tenta começar o jogo, cada peça disponível no centro da mesa (20, 20)
    for (int i = 0; i < num_pecas; i++) {
        for (int rotacao = 0; rotacao < 3; rotacao++) {
            mesa[20][20] = {i, rotacao};
            peca_usada[i] = true;
            
            //candidatos para a 2ª peça serão os 3 espaços à volta desta peça inciail
            set<pair<int,int>> candidatos_iniciais;
            int tipo_triangulo = (20 + 20) % 2; // Será 0 (triangulo para cima)
            for (int lado = 0; lado < 3; lado++) {
                candidatos_iniciais.insert({20 + delta_linha[tipo_triangulo][lado], 20 + delta_coluna[tipo_triangulo][lado]});
            }
            
            explorar_jogadas(0, candidatos_iniciais);
            
            peca_usada[i] = false;
            mesa[20][20] = {-1, -1};
        }
    }
    
    cout << pontuacao_maxima << endl;
    
    return 0;
}