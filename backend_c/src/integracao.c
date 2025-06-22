#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h> 
#include <time.h>  
#include <ctype.h> 
#include <locale.h>

#include "integracao.h" 

// ================== DEFINIÇÃO DA ÁREA DE CORTE ==================
#define MIN_LAT -90.0  // Polo Sul
#define MAX_LAT 90.0   // Polo Norte
#define MIN_LON -180.0 // Extremo Oeste
#define MAX_LON 180.0 // Extremo Leste
// ==============================================================

// ========================== VARIÁVEIS GLOBAIS ==========================

Node nodes[MAX_NODES];
int total_nodes = 0;

Way ways[MAX_WAYS];
int total_ways = 0;

Vertices vertices[MAX_VERTICES];
Arestas arestas_poly[MAX_ARESTAS]; 
AdjList* grafoAdj = NULL; 

int totalVertices = 0, totalArestas = 0;

// Parâmetros da zona UTM 23S
const double a = 6378137.0;            
const double f = 1.0 / 298.257223563;  
const double k0 = 0.9996;
const double lon0_deg = -45.0; 

// ========================== FUNÇÕES AUXILIARES DE STRING E MATEMÁTICA ==========================

/**
 * @brief Extrai o valor de um atributo de uma tag XML.
 * @param line A linha contendo a tag.
 * @param key A chave do atributo (ex: "id=").
 * @param value O buffer onde o valor extraído será armazenado.
 */
void extract_attr(const char* line, const char* key, char* value) {
    const char* p = strstr(line, key);
    if (p) {
        p = strchr(p, '\"');
        if (p) {
            p++;
            const char* q = strchr(p, '\"');
            if (q) {
                size_t len = q - p;
                strncpy(value, p, len);
                value[len] = '\0';
            }
        }
    }
}

/**
 * @brief Encontra o índice interno de um nó a partir de seu ID original do OSM.
 * @param id O ID original do nó.
 * @return O índice interno do nó ou -1 se não for encontrado.
 */
int get_node_index(long long id) {
    for (int i = 0; i < total_nodes; i++) {
        if (nodes[i].id_original == id)
            return nodes[i].id_interno;
    }
    return -1;
}

/**
 * @brief Encontra a última ocorrência de uma substring em uma string.
 * @param sub A substring a ser procurada.
 * @param string A string principal.
 * @return A posição da última ocorrência ou -1.
 */
int RAt(const char *sub, const char *string) {
    int tamString = strlen(string);
    int tamSub = strlen(sub);
    if (tamSub == 0) return -1;

    for (int j = tamString - tamSub; j >= 0; j--) {
        if (strncmp(string + j, sub, tamSub) == 0)
            return j;
    }
    return -1;
}

/**
 * @brief Converte coordenadas geográficas (latitude, longitude) para UTM.
 * @param lat_deg Latitude em graus.
 * @param lon_deg Longitude em graus.
 * @param x Ponteiro para armazenar a coordenada Leste (X).
 * @param y Ponteiro para armazenar a coordenada Norte (Y).
 */
void converter_para_utm(double lat_deg, double lon_deg, double* x, double* y) {
    double e2 = f * (2 - f);
    double ep2 = e2 / (1 - e2);
    double lat = lat_deg * PI / 180.0;
    double lon = lon_deg * PI / 180.0;
    double lon0 = lon0_deg * PI / 180.0;

    double N = a / sqrt(1 - e2 * sin(lat) * sin(lat));
    double T = tan(lat) * tan(lat);
    double C = ep2 * cos(lat) * cos(lat);
    double A = (lon - lon0) * cos(lat);

    double M = a * ((1 - e2/4 - 3*e2*e2/64 - 5*e2*e2*e2/256) * lat
        - (3*e2/8 + 3*e2*e2/32 + 45*e2*e2*e2/1024) * sin(2*lat)
        + (15*e2*e2/256 + 45*e2*e2*e2/1024) * sin(4*lat)
        - (35*e2*e2*e2/3072) * sin(6*lat));

    *x = k0 * N * (A + (1 - T + C) * pow(A,3)/6 + (5 - 18*T + T*T + 72*C - 58*ep2) * pow(A,5)/120) + 500000.0;
    *y = k0 * (M + N * tan(lat) * (A*A/2 + (5 - T + 9*C + 4*C*C) * pow(A,4)/24
         + (61 - 58*T + T*T + 600*C - 330*ep2) * pow(A,6)/720));

    if (lat_deg < 0)
        *y += 10000000.0;
}

/**
 * @brief Normaliza as coordenadas dos pontos, transladando para a origem e aplicando um fator de redução.
 * @param pontos Array de nós.
 * @param n Número de nós.
 * @param redutor Fator de escala a ser aplicado.
 */

void reduzirEscala(Node pontos[], int n, int redutor) {
    if (n <= 1 || redutor <= 0) return;

    double minX = DBL_MAX;
    double minY = DBL_MAX;

    for (int i = 0; i < n; i++) {
        if (!isnan(pontos[i].x) && !isinf(pontos[i].x) && pontos[i].x < minX) {
            minX = pontos[i].x;
        }
        if (!isnan(pontos[i].y) && !isinf(pontos[i].y) && pontos[i].y < minY) {
            minY = pontos[i].y;
        }
    }

    if (minX == DBL_MAX || minY == DBL_MAX) {
        return;
    }

    for (int i = 0; i < n; i++) {
        pontos[i].x = (pontos[i].x - minX) / redutor;
        pontos[i].y = (pontos[i].y - minY) / redutor;
    }
}

/**
 * @brief Calcula a distância euclidiana entre dois pontos.
 * @return A distância.
 */
double calcDist(double x0, double y0, double x1, double y1) {
    return sqrt(pow(x0 - x1, 2.0) + pow(y0 - y1, 2.0));
}

// ========================== FUNÇÕES DA HEAP MÍNIMA (PARA DIJKSTRA) ==========================
void trocarHeapItem(HeapItem* a, HeapItem* b) {
    HeapItem temp = *a; *a = *b; *b = temp;
}

void heapify(MinHeap* heap, int idx) {
    int menor = idx;
    int esquerda = 2 * idx + 1;
    int direita = 2 * idx + 2;

    if (esquerda < heap->tamanho && heap->items[esquerda].distancia < heap->items[menor].distancia) menor = esquerda;
    if (direita < heap->tamanho && heap->items[direita].distancia < heap->items[menor].distancia) menor = direita;

    if (menor != idx) {
        heap->pos[heap->items[menor].vertice] = idx;
        heap->pos[heap->items[idx].vertice] = menor;
        trocarHeapItem(&heap->items[menor], &heap->items[idx]);
        heapify(heap, menor);
    }
}

MinHeap* criarMinHeap(int capacidade) {
    MinHeap* heap = (MinHeap*) malloc(sizeof(MinHeap));
    heap->pos = (int*) malloc(capacidade * sizeof(int));
    heap->items = (HeapItem*) malloc(capacidade * sizeof(HeapItem));
    heap->tamanho = 0;
    heap->capacidade = capacidade;
    for(int i=0; i < capacidade; ++i) heap->pos[i] = -1;
    return heap;
}

HeapItem extrairMin(MinHeap* heap) {
    if (heap->tamanho == 0) return (HeapItem){-1, INF};
    HeapItem raiz = heap->items[0];
    HeapItem ultimoItem = heap->items[heap->tamanho - 1];
    heap->items[0] = ultimoItem;
    heap->pos[ultimoItem.vertice] = 0;
    heap->pos[raiz.vertice] = -1; 
    heap->tamanho--;
    heapify(heap, 0);
    return raiz;
}

void diminuirChave(MinHeap* heap, int vertice, double novaDistancia) {
    int i = heap->pos[vertice];
    if(i < 0 || i >= heap->tamanho) return; 

    heap->items[i].distancia = novaDistancia;
    while (i != 0 && heap->items[(i - 1) / 2].distancia > heap->items[i].distancia) {
        heap->pos[heap->items[i].vertice] = (i - 1) / 2;
        heap->pos[heap->items[(i - 1) / 2].vertice] = i;
        trocarHeapItem(&heap->items[i], &heap->items[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void liberarMinHeap(MinHeap* heap) {
    if (heap) {
        free(heap->pos);
        free(heap->items);
        free(heap);
    }
}

// ========================== LEITURA DE ARQUIVOS E CONSTRUÇÃO DO GRAFO ==========================

/**
 * @brief Analisa um arquivo .osm, extrai nós e vias, e gera um arquivo .poly.
 * @param filename O caminho do arquivo .osm.
 */
void parse_osm(const char* filename) {
    char* old_locale = setlocale(LC_NUMERIC, "C");

    char arqSaida[256];
    
    int posPonto = RAt(".", filename);
    if (posPonto != -1) { strncpy(arqSaida, filename, posPonto); arqSaida[posPonto] = '\0'; }
    else { strcpy(arqSaida, filename); }
    strcat(arqSaida, ".poly");


    FILE* f = fopen(filename, "r");
    if (!f) { perror("Erro ao abrir o arquivo OSM"); return; }
    
    char line[2048];
    Way current_way;
    int inside_way = 0; 
    total_nodes = 0;
    total_ways = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "<way")) {
            // Início de uma nova via
            inside_way = 1;
            current_way.count = 0;
            current_way.is_oneway = 0; // Padrão é mão dupla
        } else if (inside_way && strstr(line, "<nd")) {
            // Nó dentro de uma via
            char ref_str[64] = "";
            extract_attr(line, "ref=", ref_str);
            if(ref_str[0]){
                long long ref_id = atoll(ref_str);
                int index = get_node_index(ref_id);
                if (index != -1 && current_way.count < MAX_WAY_NODES) {
                    current_way.node_ids[current_way.count++] = index;
                }
            }
        } else if (inside_way && strstr(line, "<tag") && strstr(line, "k=\"oneway\"") && strstr(line, "v=\"yes\"")) {
            
            current_way.is_oneway = 1;
        }
        else if (inside_way && strstr(line, "</way>")) {
            // Fim da via
            if (current_way.count > 1 && total_ways < MAX_WAYS) {
                ways[total_ways++] = current_way;
            }
            inside_way = 0; 
        }
        else if (!inside_way && strstr(line, "<node")) {
            
            char id_str[64] = "", lat_str[64] = "", lon_str[64] = "";
            extract_attr(line, "id=", id_str);
            extract_attr(line, "lat=", lat_str);
            extract_attr(line, "lon=", lon_str);
            if (id_str[0] && lat_str[0] && lon_str[0]) {
                double lat = atof(lat_str);
                double lon = atof(lon_str);
                if (lat >= MIN_LAT && lat <= MAX_LAT && lon >= MIN_LON && lon <= MAX_LON) {
                    

                    if (total_nodes < MAX_NODES) {
                        nodes[total_nodes].id_original = atoll(id_str);
                        nodes[total_nodes].lat = lat;
                        nodes[total_nodes].lon = lon;
                        converter_para_utm(nodes[total_nodes].lat, nodes[total_nodes].lon, &nodes[total_nodes].x, &nodes[total_nodes].y);
                        nodes[total_nodes].id_interno = total_nodes;
                        total_nodes++;
                        
                    } else {
                       
                    }
                } else {
                    
                }
            }
        }
    }
    fclose(f);

    // ================== DEBUG ANTES E DEPOIS ==================
    for (int i = 0; i < 10 && i < total_nodes; i++) {
        printf("Nó %d: (X=%.2f, Y=%.2f)\n", i, nodes[i].x, nodes[i].y);
    }
    
    reduzirEscala(nodes, total_nodes, 2);

    for (int i = 0; i < 10 && i < total_nodes; i++) {
        printf("Nó %d: (X=%.2f, Y=%.2f)\n", i, nodes[i].x, nodes[i].y);
    }
    // =========================================================

    if (total_nodes > 0) {
        double maxY = 0.0;
        for (int i = 0; i < total_nodes; i++) {
            if (nodes[i].y > maxY) maxY = nodes[i].y;
        }
        for (int i = 0; i < total_nodes; i++) {
            nodes[i].y = maxY - nodes[i].y;
        }
    }

    FILE* outFile = fopen(arqSaida, "w");
    if (!outFile) { perror("Erro ao criar o arquivo .poly"); return; }
    
    int numArestasParaPoly = 0;
    for (int i = 0; i < total_ways; i++) numArestasParaPoly += (ways[i].count - 1);
    fprintf(outFile, "%d\t2\t0\t0\n", total_nodes);
    for (int i = 0; i < total_nodes; i++) fprintf(outFile, "%d\t%f\t%f\n", nodes[i].id_interno, nodes[i].x, nodes[i].y);
    fprintf(outFile, "%d\t0\n", numArestasParaPoly);
    int currentPolyEdgeId = 0;
    for (int i = 0; i < total_ways; i++) {
        for (int j = 0; j < ways[i].count - 1; j++) {
            fprintf(outFile, "%d\t%d\t%d\t%d\n", currentPolyEdgeId++, ways[i].node_ids[j], ways[i].node_ids[j + 1], ways[i].is_oneway);
        }
    }
    fprintf(outFile, "0\n");
    setlocale(LC_NUMERIC, old_locale);
    fclose(outFile);
    printf("Arquivo \"%s\" criado com sucesso.\n", arqSaida);
}

/**
 * @brief Carrega os dados de um arquivo .poly para as estruturas de dados globais.
 * @param filename O caminho do arquivo .poly.
 */
void carregarPoly(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo .poly");
        return;
    }

    int dummy;
    fscanf(fp, "%d %d %d %d", &totalVertices, &dummy, &dummy, &dummy);

    if (totalVertices > MAX_VERTICES) totalVertices = MAX_VERTICES;
    for (int i = 0; i < totalVertices; i++) {
        fscanf(fp, "%d %lf %lf", &vertices[i].id, &vertices[i].x, &vertices[i].y);
    }
    
    fscanf(fp, "%d %d", &totalArestas, &dummy);
    if (totalArestas > MAX_ARESTAS) totalArestas = MAX_ARESTAS;
    for (int i = 0; i < totalArestas; i++) {
        fscanf(fp, "%d %d %d %d", &dummy, &arestas_poly[i].orig, &arestas_poly[i].dest, &arestas_poly[i].is_oneway); // Leia o is_oneway
    }

    fclose(fp);
}


/**
 * @brief Adiciona uma aresta à lista de adjacência.
 * @param u Vértice de origem.
 * @param v Vértice de destino.
 * @param peso O peso da aresta.
 */
void adicionarArestaAdj(int u, int v, double peso) {
    AdjNode* novoNo = (AdjNode*) malloc(sizeof(AdjNode));
    novoNo->vertice = v;
    novoNo->peso = peso;
    novoNo->proximo = grafoAdj[u].cabeca;
    grafoAdj[u].cabeca = novoNo;
}


/**
 * @brief Constrói o grafo (lista de adjacência) a partir dos dados carregados.
 */
void construirGrafo() {
    if (grafoAdj != NULL) {
        for (int i = 0; i < totalVertices; i++) {
            AdjNode* noAtual = grafoAdj[i].cabeca;
            while (noAtual != NULL) {
                AdjNode* temp = noAtual;
                noAtual = noAtual->proximo;
                free(temp);
            }
        }
        free(grafoAdj);
    }

    grafoAdj = (AdjList*) malloc(totalVertices * sizeof(AdjList));
    for (int i = 0; i < totalVertices; i++) {
        grafoAdj[i].cabeca = NULL;
    }

    totalArestas = 0;

    for (int i = 0; i < total_ways; i++) {
        for (int j = 0; j < ways[i].count - 1; j++) {
            if (totalArestas >= MAX_ARESTAS) break;

            int u = ways[i].node_ids[j];
            int v = ways[i].node_ids[j + 1];

            if (u >= 0 && u < totalVertices && v >= 0 && v < totalVertices) {
                // Adiciona a aresta na nossa lista para exibição
                arestas_poly[totalArestas].orig = u;
                arestas_poly[totalArestas].dest = v;
                arestas_poly[totalArestas].is_oneway = ways[i].is_oneway;
                totalArestas++;
                
                // Adiciona a aresta na lista de adjacência para o Dijkstra
                double dist = calcDist(vertices[u].x, vertices[u].y, vertices[v].x, vertices[v].y);
                adicionarArestaAdj(u, v, dist); // Sempre adiciona no sentido da via

                // Se NÃO for mão única, adiciona a aresta no sentido contrário também
                if (ways[i].is_oneway == 0) {
                    adicionarArestaAdj(v, u, dist);
                }
            }
        }
    }
}


// ========================== FUNÇÕES DE ACESSO PARA A GUI (PYTHON) ==========================

int get_total_vertices() { return totalVertices; }

void get_vertex_coords(int id, double* x, double* y) {
    if (id >= 0 && id < totalVertices) { *x = vertices[id].x; *y = vertices[id].y; } 
    else { *x = -1; *y = -1; }
}

int get_total_edges_from_poly() { return totalArestas; }

void get_edge_endpoints(int index, int* orig_id, int* dest_id) {
    if (index >= 0 && index < totalArestas) { *orig_id = arestas_poly[index].orig; *dest_id = arestas_poly[index].dest; }
    else { *orig_id = -1; *dest_id = -1; }
}

void get_edge_info(int index, int* orig_id, int* dest_id, double* weight, int* is_oneway) {
    if (index >= 0 && index < totalArestas) {
        int u = arestas_poly[index].orig;
        int v = arestas_poly[index].dest;

        
        *orig_id = u;
        *dest_id = v;
        *is_oneway = arestas_poly[index].is_oneway; 

        if (u >= 0 && u < totalVertices && v >= 0 && v < totalVertices) {
            *weight = calcDist(vertices[u].x, vertices[u].y, vertices[v].x, vertices[v].y);
        } else {
            *weight = -1.0;
        }
    } else {
        // Valores de erro se o índice for inválido
        *orig_id = -1;
        *dest_id = -1;
        *weight = -1.0;
        *is_oneway = 0; 
    }
}

// ========================== ALGORITMO DE DIJKSTRA (PARA A GUI) ==========================
/**
 * @brief Executa o algoritmo de Dijkstra para encontrar o menor caminho.
 * @param inicio Vértice de partida.
 * @param fim Vértice de chegada.
 * @param path_buffer Buffer para armazenar os IDs dos vértices no caminho.
 * @param max_path_len Tamanho máximo do buffer do caminho.
 * @return Uma struct DijkstraResult com as informações do cálculo.
 */
DijkstraResult dijkstra_gui(int inicio, int fim, int* path_buffer, int max_path_len) {
    clock_t start_time = clock();
    DijkstraResult result = {INF, 0, 0.0, 0};
    
    if (totalVertices == 0 || inicio < 0 || inicio >= totalVertices || fim < 0 || fim >= totalVertices) {
        result.tempo_processamento = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        return result; 
    }

    double* dist = (double*) malloc(totalVertices * sizeof(double));
    int* prev = (int*) malloc(totalVertices * sizeof(int));

    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[inicio] = 0;

    MinHeap* minHeap = criarMinHeap(totalVertices);
    for(int i = 0; i < totalVertices; i++) {
        minHeap->items[i] = (HeapItem){i, dist[i]};
        minHeap->pos[i] = i;
    }
    minHeap->tamanho = totalVertices;
    diminuirChave(minHeap, inicio, 0.0);

    int nos_explorados = 0;
    while (minHeap->tamanho > 0) {
        HeapItem u_item = extrairMin(minHeap);
        int u = u_item.vertice;
        nos_explorados++;
        
        if (u == fim) break; 

        AdjNode* no_adj = grafoAdj[u].cabeca;
        while (no_adj != NULL) {
            int v = no_adj->vertice;
            if (dist[u] != INF && dist[u] + no_adj->peso < dist[v]) {
                dist[v] = dist[u] + no_adj->peso;
                prev[v] = u;
                diminuirChave(minHeap, v, dist[v]);
            }
            no_adj = no_adj->proximo;
        }
    }

    result.distancia_total = dist[fim];
    result.num_nos_explorados = nos_explorados;

    if (dist[fim] != INF) {
        int temp_path[MAX_VERTICES];
        int path_idx = 0;
        for (int v = fim; v != -1; v = prev[v]) {
            if(path_idx < MAX_VERTICES) temp_path[path_idx++] = v;
        }
        
        result.path_len = (path_idx < max_path_len) ? path_idx : max_path_len;
        for (int i = 0; i < result.path_len; i++) {
            path_buffer[i] = temp_path[path_idx - 1 - i];
        }
    }

    free(dist);
    free(prev);
    liberarMinHeap(minHeap);
    
    result.tempo_processamento = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    return result;
}

// ========================== FUNÇÕES DE EDIÇÃO DO GRAFO (RF05) ==========================
/**
 * @brief Remove um vértice e todas as arestas conectadas a ele.
 * O vértice removido é substituído pelo último vértice da lista, e os IDs são ajustados.
 * @param id_vertice_remover O ID do vértice a ser removido.
 * @return 1 se sucesso, 0 se o vértice não foi encontrado ou falha.
 */
int remover_vertice(int id_vertice_remover) {
    if (id_vertice_remover < 0 || id_vertice_remover >= totalVertices) {
        fprintf(stderr, "Erro: Tentativa de remover vértice inválido %d.\n", id_vertice_remover);
        return 0; // Vértice inválido
    }

    printf("DEBUG_C: Tentando remover vértice %d\n", id_vertice_remover);

    for (int i = totalArestas - 1; i >= 0; i--) {
        int u = arestas_poly[i].orig;
        int v = arestas_poly[i].dest;

        if (u == id_vertice_remover || v == id_vertice_remover) {
            
            remover_no_adj(u, v); // Remove u -> v
            if (!arestas_poly[i].is_oneway) { // Se não for mão única, remove também v -> u
                remover_no_adj(v, u);
            }
            
            arestas_poly[i] = arestas_poly[totalArestas - 1];
            totalArestas--;
        }
    }
    
    if (id_vertice_remover != totalVertices - 1) { 
        int id_vertice_movido = totalVertices - 1;
        vertices[id_vertice_remover] = vertices[id_vertice_movido];
        vertices[id_vertice_remover].id = id_vertice_remover; 
        
        for (int i = 0; i < totalArestas; i++) {
            if (arestas_poly[i].orig == id_vertice_movido) {
                arestas_poly[i].orig = id_vertice_remover;
            }
            if (arestas_poly[i].dest == id_vertice_movido) {
                arestas_poly[i].dest = id_vertice_remover;
            }
        }

        
        // Libera o grafo adjacente atual
        for (int i = 0; i < totalVertices; i++) {
            AdjNode* noAtual = grafoAdj[i].cabeca;
            while (noAtual != NULL) {
                AdjNode* temp = noAtual;
                noAtual = noAtual->proximo;
                free(temp);
            }
        }
        free(grafoAdj);
        grafoAdj = NULL; 
    }
    
    totalVertices--; 
    
    construirGrafo(); 

    printf("DEBUG_C: Vértice %d e suas arestas removidos. Novo total de vértices: %d\n", id_vertice_remover, totalVertices);
    return 1;
}
/**
 * @brief Adiciona um novo vértice ao grafo.
 * @param x A coordenada X do novo vértice.
 * @param y A coordenada Y do novo vértice.
 * @return O ID do novo vértice ou -1 em caso de falha (limite atingido).
 */
int adicionar_vertice(double x, double y) {
    if (totalVertices >= MAX_VERTICES) {
        fprintf(stderr, "Erro: Limite de vértices (%d) atingido.\n", MAX_VERTICES);
        return -1;
    }

    AdjList* novo_ponteiro = (AdjList*) realloc(grafoAdj, (totalVertices + 1) * sizeof(AdjList));
    if (novo_ponteiro == NULL) {
        perror("Erro ao realocar memória para o grafo");
        return -1; 
    }
    grafoAdj = novo_ponteiro; 

    grafoAdj[totalVertices].cabeca = NULL;
    
    vertices[totalVertices].id = totalVertices;
    vertices[totalVertices].x = x;
    vertices[totalVertices].y = y;
    
    int novo_id = totalVertices;
    totalVertices++;
    
    printf("DEBUG_C: Vértice %d adicionado em (%.2f, %.2f)\n", novo_id, x, y);
    return novo_id;
}

void remover_no_adj(int u, int v) {
    AdjNode* atual = grafoAdj[u].cabeca;
    AdjNode* anterior = NULL;

    while (atual != NULL && atual->vertice != v) {
        anterior = atual;
        atual = atual->proximo;
    }

    if (atual == NULL) return; // Aresta não encontrada

    if (anterior == NULL) { // O nó a ser removido é a cabeça
        grafoAdj[u].cabeca = atual->proximo;
    } else {
        anterior->proximo = atual->proximo;
    }
    free(atual);
}

/**
 * @brief Adiciona uma aresta bidirecional entre dois vértices.
 * @param u O ID do primeiro vértice.
 * @param v O ID do segundo vértice.
 * @return 1 se sucesso, 0 se falha.
 */
int adicionar_aresta(int u, int v) {
    if (u < 0 || u >= totalVertices || v < 0 || v >= totalVertices || u == v) {
        return 0; // Vértices inválidos
    }
    if (totalArestas >= MAX_ARESTAS) {
        fprintf(stderr, "Erro: Limite de arestas (%d) atingido.\n", MAX_ARESTAS);
        return 0;
    }

    // Adiciona na lista de adjacência
    double dist = calcDist(vertices[u].x, vertices[u].y, vertices[v].x, vertices[v].y);
    adicionarArestaAdj(u, v, dist);
    adicionarArestaAdj(v, u, dist);

    // Adiciona na lista de arestas para exibição
    arestas_poly[totalArestas].orig = u;
    arestas_poly[totalArestas].dest = v;
    totalArestas++;

    printf("DEBUG_C: Aresta adicionada entre %d e %d\n", u, v);
    return 1;
}

/**
 * @brief Remove uma aresta bidirecional entre dois vértices.
 * @param u O ID do primeiro vértice.
 * @param v O ID do segundo vértice.
 * @return 1 se sucesso, 0 se não encontrou a aresta.
 */
int remover_aresta(int u, int v) {
     if (u < 0 || u >= totalVertices || v < 0 || v >= totalVertices) {
        return 0; // Vértices inválidos
    }

    remover_no_adj(u, v);
    remover_no_adj(v, u);

    int i;
    int encontrou = -1;
    for (i = 0; i < totalArestas; i++) {
        if ((arestas_poly[i].orig == u && arestas_poly[i].dest == v) ||
            (arestas_poly[i].orig == v && arestas_poly[i].dest == u)) {
            encontrou = i;
            break;
        }
    }

    if (encontrou != -1) {
        arestas_poly[encontrou] = arestas_poly[totalArestas - 1];
        totalArestas--;
        printf("DEBUG_C: Aresta removida entre %d e %d\n", u, v);
        return 1;
    }
    
    return 0; // Aresta não encontrada
}