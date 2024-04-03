#include <stdio.h>
#include <stack>
#include <cstdio>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

char** maze = nullptr;
int num_rows = 0;
int num_cols = 0;

struct pos_t {
    int i;
    int j;
};

std::stack<pos_t> valid_positions;
std::atomic<bool> exit_found(false);
std::mutex mtx; // Mutex global
std::mutex print_mtx; // Mutex print


using namespace std;

pos_t load_maze(const char* file_name, char*** mazePtr, int& num_rows, int& num_cols) {
    pos_t initial_pos = {-1, -1}; // Inicializa com valores inválidos
    FILE *arquivo;

    arquivo = fopen(file_name, "r");
    if (arquivo == nullptr) {
        std::cerr << "Não foi possível abrir o arquivo: " << file_name << std::endl;
        return initial_pos;
    }

    if (fscanf(arquivo, "%d %d", &num_rows, &num_cols) != 2) {
        std::cerr << "Erro ao ler as dimensões." << std::endl;
        fclose(arquivo);
        return initial_pos;
    }

    // Aloca a matriz do labirinto
    *mazePtr = (char**)malloc(num_rows * sizeof(char*));
    for (int i = 0; i < num_rows; ++i) {
        (*mazePtr)[i] = (char*)malloc(num_cols * sizeof(char));
    }

    // Lê o labirinto do arquivo
    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            char caractere = fgetc(arquivo);
            if (caractere == '\n') {
                caractere = fgetc(arquivo);
            }
            if (caractere == 'e') {
                initial_pos.i = i;
                initial_pos.j = j;
            }
            (*mazePtr)[i][j] = caractere;
        }
    }

    fclose(arquivo); 
    return initial_pos;
}

// Função que imprime o labirinto
void print_maze(char** maze, int num_rows, int num_cols) {
    if (maze == nullptr) {
        std::cerr << "O labirinto não foi carregado corretamente." << std::endl;
        return;
    }

    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            std::cout << maze[i][j];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


bool isValid(int i, int j) {
    return i >= 0 && i < num_rows && j >= 0 && j < num_cols && (maze[i][j] == 'x' || maze[i][j] == 's');
}


bool walk(pos_t initial_pos) {
    mtx.lock();
    valid_positions.push(initial_pos); 
    mtx.unlock();

    while (!valid_positions.empty()) {
        mtx.lock();
        if (exit_found) {
            mtx.unlock();
            return false;
        }
        pos_t current_pos = valid_positions.top();
        valid_positions.pop();
        mtx.unlock();

        int i = current_pos.i;
        int j = current_pos.j;

        // Se a posição atual já foi visitada ou é inválida, pula para a próxima iteração
        if (maze[i][j] != 'x' && maze[i][j] != 'e' && maze[i][j] != 's') continue;

        // Verifica se a posição atual é a saída
        if (maze[i][j] == 's') {
            mtx.lock();
            maze[i][j] = 'o'; // Marca a posição da saída
            exit_found = true;
            mtx.unlock();
            print_maze(maze, num_rows, num_cols);
            return true;
        } 

        // Marca a posição atual como visitada
        if (maze[i][j] == 'x') {
            mtx.lock();
            maze[i][j] = '.';
            mtx.unlock();
        } else if (maze[i][j] == 'e') {
            mtx.lock();
            maze[i][j] = '.';
            mtx.unlock();
        }
        // Direções: direita, esquerda, baixo, cima
        pos_t directions[4] = {{i, j + 1}, {i, j - 1}, {i + 1, j}, {i - 1, j}};

        for (auto& dir : directions) {
            if (isValid(dir.i, dir.j)) {
                // Adiciona a posição válida e não visitada à pilha
                mtx.lock();
                valid_positions.push(dir);
                mtx.unlock();
            }
        }

        // Imprime o labirinto após cada passo
        print_mtx.lock();
        print_maze(maze, num_rows, num_cols);
        print_mtx.unlock();
    }

    return false; // Retorna falso se a saída não for encontrada
}





int main(int argc, char* argv[]) {
    pos_t initial_pos = load_maze(argv[1], &maze, num_rows, num_cols);

    std::thread t1(walk, initial_pos);

    t1.join();

    if (exit_found) {
        std::cout << "Saída encontrada!" << std::endl;
    } else {
        std::cout << "Saída não encontrada." << std::endl;
    }

    for (int i = 0; i < num_rows; ++i) {
        free(maze[i]);
    }

    free(maze);

    return 0;
}
