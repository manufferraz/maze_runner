#include <stdio.h>
#include <stack>
#include <cstdio>
#include <iostream>
#include <vector>

char** maze = nullptr;
int num_rows = 0;
int num_cols = 0;

struct pos_t {
    int i;
    int j;
};

std::stack<pos_t> valid_positions;

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
}


bool isValid(int i, int j) {
    return i >= 0 && i < num_rows && j >= 0 && j < num_cols && (maze[i][j] == 'x' || maze[i][j] == 's');
}


bool walk(pos_t initial_pos) {
    valid_positions.push(initial_pos); 

    while (!valid_positions.empty()) {
        pos_t current_pos = valid_positions.top();
        valid_positions.pop();

        int i = current_pos.i;
        int j = current_pos.j;

        if (maze[i][j] != 'x' && maze[i][j] != 'e' && maze[i][j] != 's') continue;

        // Verifica se a posição atual é a saída
        if (maze[i][j] == 's') {
            maze[i][j] = 'o'; // Marca a posição da saída
            print_maze(maze, num_rows, num_cols);
            return true;
        }

        // Marca a posição atual como visitada
        if (maze[i][j] == 'x') {
            maze[i][j] = '.';
        } else if (maze[i][j] == 'e') {
            maze[i][j] = '.';
        }
        // Direções: direita, esquerda, baixo, cima
        pos_t directions[4] = {{i, j + 1}, {i, j - 1}, {i + 1, j}, {i - 1, j}};

        for (auto& dir : directions) {
            if (isValid(dir.i, dir.j)) {
                // Adiciona a posição válida e não visitada à pilha
                valid_positions.push(dir);
            }
        }

        // Imprime o labirinto após cada passo
        print_maze(maze, num_rows, num_cols);
    }

    return false; // Retorna falso se a saída não for encontrada
}



int main(int argc, char* argv[]) {
    pos_t initial_pos = load_maze(argv[1], &maze, num_rows, num_cols);
    std::cout << "Posição inicial: (" << initial_pos.i << ", " << initial_pos.j << ")" << std::endl;

    bool exit_found = walk(initial_pos);
    if(exit_found) {
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
