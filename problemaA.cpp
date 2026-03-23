#include <iostream>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

//representa um trikit
struct Trikit {
    int v[3];
};

int n = 0;
vector<Trikit> pieces;
bool used_piece[20];

//grelha para simular a mesa (40x40 é espaço de sobra para <= 20 peças)
//guarda um par {id_da_peca, rotacao}
pair<int, int> grid[40][40];

int max_score = 0;

// GEOMETRIA DA GRELHA TRIANGULAR

//direções: 0 = Direita/Topo, 1 = Fundo/Direita, 2 = Esquerda
// dr[0] e dc[0] são para triângulos cima(linha + coluna == PAR)
// dr[1] e dc[1] são para triângulos baixo (linha + coluna == ÍMPAR)
const int dr[2][3] = { {0, 1, 0}, {-1, 0, 0} };
const int dc[2][3] = { {1, 0, -1}, {0, 1, -1} };

// Mapeamento de portas opostas:
// Se o triangulo para cima se liga a um traingulo para baixo pela sua porta 0 (Direita), o trinagulo para baixo recebe-o na sua porta 2 (Esquerda).
const int opposite[2][3] = { {2, 0, 1}, {1, 2, 0} };

//devolve a aresta direcionada (x, y) de um trikit, considerando a sua rotação e porta pedida
pair<int, int> get_edge(int id, int rot, int port) {
    int idx = (rot + port) % 3;
    int u = pieces[id].v[idx];
    int v = pieces[id].v[(idx + 1) % 3];
    return {u, v};
}

//verifica se podemos colocar a peça 'id' com a rotação 'rot' na coordenada (r, c)
//se sim, devolve os pontos ganhos nesta jogada; ->Se não, devolve -1.
int can_place(int r, int c, int id, int rot) {
    int type = (r + c) % 2; // 0 para cima, 1 para baixo
    int matches = 0;
    int score = 0;
    
    for (int p = 0; p < 3; p++) {
        int nr = r + dr[type][p];
        int nc = c + dc[type][p];
        
        // Se houver uma peça vizinha nesta direção -> verificar o encaixe
        if (grid[nr][nc].first != -1) {
            matches++;
            int nid = grid[nr][nc].first;
            int nrot = grid[nr][nc].second;
            
            int op = opposite[type][p];
            pair<int, int> my_edge = get_edge(id, rot, p);
            pair<int, int> their_edge = get_edge(nid, nrot, op);
            
            //o sentido (x -> y) tem de bater com o sentido inverso (y -> x)
            if (my_edge.first == their_edge.second && my_edge.second == their_edge.first) {
                score += my_edge.first + my_edge.second;
            } else {
                return -1; // ALERTA conflito: há uma peça, mas os números não batem
            }
        }
    }
    
    //tem de se ligar a pelo menos uma peça (exceto a primeira que pomos no tabuleiro, 
    //mas chamamos isto apenas para casas com vizinhos)
    if (matches == 0) return -1;
    
    return score;
}

//
//algoritmo de backtracking (DFS)
//

void dfs(int current_score, set<pair<int,int>> candidates) {
    //atualiza a pontuação máxima alcançada
    max_score = max(max_score, current_score);
    
    if (candidates.empty()) return;
    
    //tira o primeiro candidato da lista (abordagem de geração de polyominos para evitar loops infinitos)
    auto it = candidates.begin();
    pair<int,int> cell = *it;
    candidates.erase(it);
    
    int r = cell.first;
    int c = cell.second;
    
    //opcao 1: Tentar colocar uma peça nesta casa
    for (int i = 0; i < n; i++) {
        if (!used_piece[i]) {
            for (int rot = 0; rot < 3; rot++) {
                int score = can_place(r, c, i, rot);
                
                if (score != -1) {
                    // Se encaixa perfeitamente, colocamos na mesa
                    grid[r][c] = {i, rot};
                    used_piece[i] = true;
                    
                    // Descobrir novos candidatos (espaços vazios à volta da peça que acabamos de pôr)
                    set<pair<int,int>> next_candidates = candidates;
                    int type = (r + c) % 2;
                    for (int p = 0; p < 3; p++) {
                        int nr = r + dr[type][p];
                        int nc = c + dc[type][p];
                        if (grid[nr][nc].first == -1) {
                            next_candidates.insert({nr, nc});
                        }
                    }
                    
                    // Chamada recursiva com a nova pontuação e os novos candidatos
                    dfs(current_score + score, next_candidates);
                    
                    //desfazer (Backtrack)
                    used_piece[i] = false;
                    grid[r][c] = {-1, -1};
                }
            }
        }
    }
    
    //opcao 2: Não colocar NADA nesta casa nesta ramificação,
    //e passar para o próximo candidato. Isto garante que testamos todas as formas possíveis.
    dfs(current_score, candidates);
}

int main() {
    //le os inputs até acabar o ficheiro
    int u, v, w;
    while (cin >> u >> v >> w) {
        pieces.push_back({u, v, w});
    }
    n = pieces.size();
    
    //iinicar a grelha toda a -1 (vazia)
    for (int r = 0; r < 40; r++) {
        for (int c = 0; c < 40; c++) {
            grid[r][c] = {-1, -1};
        }
    }
    
    // tenta começar o jogo, cada peça disponível no centro da mesa (20, 20)
    for (int i = 0; i < n; i++) {
        for (int rot = 0; rot < 3; rot++) {
            grid[20][20] = {i, rot};
            used_piece[i] = true;
            
            //candidatos para a 2ª peça serão os 3 espaços à volta desta peça inciail
            set<pair<int,int>> initial_candidates;
            int type = (20 + 20) % 2; // Será 0 (▲)
            for (int p = 0; p < 3; p++) {
                initial_candidates.insert({20 + dr[type][p], 20 + dc[type][p]});
            }
            
            dfs(0, initial_candidates);
            
            used_piece[i] = false;
            grid[20][20] = {-1, -1};
        }
    }
    
    cout << max_score << endl;
    
    return 0;
}