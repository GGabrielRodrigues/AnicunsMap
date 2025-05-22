// ========================== DEFINES ==========================

#define MAX_NODES 10000
#define MAX_WAYS 5000
#define MAX_WAY_NODES 100

#define MAX_VERTICES 10000
#define MAX_ARESTAS 20000
#define INF 1e9
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
} Way;

typedef struct {
    int id;
    double x, y;
} Vertices;

typedef struct {
    int orig, dest;
    double dist;
} Arestas;

// ========================== EXTERN DAS VARIÁVEIS GLOBAIS ==========================

extern Node nodes[MAX_NODES];
extern int total_nodes;

extern Way ways[MAX_WAYS];
extern int total_ways;

extern Vertices vertices[MAX_VERTICES];
extern Arestas arestas[MAX_ARESTAS];
extern double matrizAdj[MAX_VERTICES][MAX_VERTICES];
extern int totalVertices, totalArestas;

// ========================== PROTÓTIPOS DAS FUNÇÕES ==========================

// Conversão e processamento do OSM
void parse_osm(const char* filename);

// Leitura e construção do grafo
void carregarPoly(const char *filename);
void construirGrafo();

// Algoritmo de Dijkstra
void dijkstra(int inicio, int fim);

// Funções auxiliares úteis para string ou matemática
int RAt(char *sub, char *string);
char* Left(char *str, int n);
char* Substr(char *s, int pos, int n);
double calcDist(double x0, double y0, double x1, double y1);