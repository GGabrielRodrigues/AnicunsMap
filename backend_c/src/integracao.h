#ifndef INTEGRACAO_H
#define INTEGRACAO_H

// ========================== DEFINES ==========================
#define MAX_NODES 100000
#define MAX_WAYS 50000
#define MAX_WAY_NODES 100
#define MAX_VERTICES 100000
#define MAX_ARESTAS 200000
#define INF 1e9 // Representa o infinito para distâncias
#define PI 3.14159265358979323846

// ========================== ESTRUTURAS ==========================
typedef struct {
    long long id_original;
    double lat, lon;
    double x, y;
    int id_interno;
} Node;

typedef struct {
    int node_ids[MAX_WAY_NODES];
    int count;
    int is_oneway;
} Way;

typedef struct {
    int id;
    double x, y;
    int ativo; // 1 se ativo, 0 se removido
} Vertices;

typedef struct {
    int orig, dest;
    double dist;
    int is_oneway;
} Arestas;

typedef struct AdjNode {
    int vertice;
    double peso;
    struct AdjNode* proximo;
} AdjNode;

typedef struct {
    AdjNode* cabeca;
} AdjList;

typedef struct {
    int vertice;
    double distancia;
} HeapItem;

typedef struct {
    HeapItem* items;
    int capacidade;
    int tamanho;
    int* pos;
} MinHeap;

typedef struct {
    double distancia_total;
    int num_nos_explorados;
    double tempo_processamento;
    int path_len;
} DijkstraResult;

// ========================== PROTÓTIPOS DAS FUNÇÕES ==========================
// Conversão e processamento do OSM
void parse_osm(const char* filename);

// Leitura e construção do grafo
void carregarPoly(const char *filename);
void construirGrafo();

// Algoritmo de Dijkstra
DijkstraResult dijkstra_gui(int inicio, int fim, int* path_buffer, int max_path_len);

// Funções para acesso de dados do grafo pelo Python
int get_total_vertices();
void get_vertex_coords(int id, double* x, double* y);
int get_total_edges_from_poly();
void get_edge_endpoints(int index, int* orig_id, int* dest_id);
void get_edge_info(int index, int* orig_id, int* dest_id, double* weight, int* is_oneway);


// Funções para edição do grafo 
int adicionar_vertice(double x, double y);
int adicionar_aresta(int u, int v);
int remover_aresta(int u, int v);
int remover_vertice(int id_vertice_remover);

// Funções auxiliares de string e matemática
void extract_attr(const char* line, const char* key, char* value);
int get_node_index(long long id);
int RAt(const char *sub, const char *string);
void converter_para_utm(double lat_deg, double lon_deg, double* x, double* y);
void reduzirEscala(Node pontos[], int n, int redutor);
double calcDist(double x0, double y0, double x1, double y1);
void remover_no_adj(int u, int v); 

#endif // INTEGRACAO_H