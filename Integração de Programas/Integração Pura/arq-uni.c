#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

// ========================== VARIÁVEIS GLOBAIS ==========================

Node nodes[MAX_NODES];
int total_nodes = 0;

Way ways[MAX_WAYS];
int total_ways = 0;

Vertices vertices[MAX_VERTICES];
Arestas arestas[MAX_ARESTAS];
double matrizAdj[MAX_VERTICES][MAX_VERTICES];
int totalVertices = 0, totalArestas = 0;

// ========================== FUNÇÕES AUXILIARES ==========================

double calcDist(double x0, double y0, double x1, double y1) {
    return sqrt(pow(x0 - x1, 2.0) + pow(y0 - y1, 2.0));
}

int get_node_index(long long id) {
    for (int i = 0; i < total_nodes; i++)
        if (nodes[i].id_original == id)
            return nodes[i].id_interno;
    return -1;
}

void extract_attr(const char* line, const char* key, char* value) {
    char* p = strstr(line, key);
    if (p) {
        p = strchr(p, '\"');
        if (p) {
            p++;
            char* q = strchr(p, '\"');
            if (q) {
                strncpy(value, p, q - p);
                value[q - p] = '\0';
            }
        }
    }
}

char* Substr(char *s, int pos, int n) {
    char *str = (char *) malloc(strlen(s) + 1);
    for (int i = 0; i < n; i++)
        str[i] = s[pos + i];
    str[n] = '\0';
    return str;
}

char* Left(char *str, int n) {
    char *substr = (char*) malloc(sizeof(char) * (strlen(str) + 1));
    for (int i = 0; i < n; i++)
        substr[i] = str[i];
    substr[n] = '\0';
    return substr;
}

int RAt(char *sub, char *string) {
    int tamString = strlen(string);
    int tamSub = strlen(sub);
    for (int j = tamString - tamSub; j >= 0; j--) {
        if (strncmp(Substr(string, j, tamSub), sub, tamSub) == 0)
            return j;
    }
    return -1;
}

void converter_para_utm(double lat_deg, double lon_deg, double* x, double* y) {
    const double a = 6378137.0;
    const double f = 1.0 / 298.257223563;
    const double k0 = 0.9996;
    const double lon0_deg = -45.0;

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

void reduzirEscala(Node pontos[], int n, int redutor) {
    double minX = pontos[0].x, minY = pontos[0].y;
    for (int i = 1; i < n; i++) {
        if (pontos[i].x < minX) minX = pontos[i].x;
        if (pontos[i].y < minY) minY = pontos[i].y;
    }
    for (int i = 0; i < n; i++) {
        pontos[i].x = (pontos[i].x - minX) / redutor;
        pontos[i].y = (pontos[i].y - minY) / redutor;
    }
}

void parse_osm(const char* filename) {
    char arqSaida[100];
    int posPonto = RAt(".", filename);
    strcpy(arqSaida, Left((char*)filename, posPonto));
    strcat(arqSaida, ".poly");

    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }
    FILE* outFile = fopen(arqSaida, "w");

    char line[1024];
    int inside_way = 0;
    Way current_way;
    current_way.count = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "<node") && strstr(line, "lat=") && strstr(line, "lon=")) {
            char id_str[64], lat_str[64], lon_str[64];
            extract_attr(line, "id=", id_str);
            extract_attr(line, "lat=", lat_str);
            extract_attr(line, "lon=", lon_str);

            if (total_nodes < MAX_NODES) {
                nodes[total_nodes].id_original = atoll(id_str);
                nodes[total_nodes].lat = atof(lat_str);
                nodes[total_nodes].lon = atof(lon_str);
                converter_para_utm(nodes[total_nodes].lat, nodes[total_nodes].lon, &nodes[total_nodes].x, &nodes[total_nodes].y);
                nodes[total_nodes].id_interno = total_nodes;
                total_nodes++;
            }
        } else if (strstr(line, "<way")) {
            inside_way = 1;
            current_way.count = 0;
        } else if (inside_way && strstr(line, "<nd")) {
            char ref_str[64];
            extract_attr(line, "ref=", ref_str);
            int index = get_node_index(atoll(ref_str));
            if (index != -1 && current_way.count < MAX_WAY_NODES)
                current_way.node_ids[current_way.count++] = index;
        } else if (inside_way && strstr(line, "</way>")) {
            inside_way = 0;
            if (current_way.count > 1 && total_ways < MAX_WAYS)
                ways[total_ways++] = current_way;
        }
    }

    fclose(f);

    reduzirEscala(nodes, total_nodes, 2);

    double maxY = nodes[0].y;
    for (int i = 1; i < total_nodes; i++)
        if (nodes[i].y > maxY) maxY = nodes[i].y;

    for (int i = 0; i < total_nodes; i++)
        nodes[i].y = maxY - nodes[i].y;

    fprintf(outFile, "%d\t%d\t%d\t%d\n", total_nodes, 2, 0, 1);
    for (int i = 0; i < total_nodes; i++)
        fprintf(outFile, "%d\t%f\t%f\n", nodes[i].id_interno, nodes[i].x, nodes[i].y);

    int numID = 0;
    for (int i = 0; i < total_ways; i++)
        for (int j = 0; j < ways[i].count - 1; j++)
            numID++;
    fprintf(outFile, "%d\t%d\n", numID, 1);

    numID = 0;
    for (int i = 0; i < total_ways; i++)
        for (int j = 0; j < ways[i].count - 1; j++)
            fprintf(outFile, "%d\t%d\t%d\t%d\n", numID++, ways[i].node_ids[j], ways[i].node_ids[j + 1], 0);

    fprintf(outFile, "0\n");
    fclose(outFile);
    printf("Arquivo \"%s\" criado com sucesso.\n", arqSaida);
}

void carregarPoly(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo .poly");
        exit(1);
    }

    int dim, tipo_v, tipo_a;
    fscanf(fp, "%d %d %d %d", &totalVertices, &dim, &tipo_v, &tipo_a);

    for (int i = 0; i < totalVertices; i++)
        fscanf(fp, "%d %lf %lf", &vertices[i].id, &vertices[i].x, &vertices[i].y);

    fscanf(fp, "%d %d", &totalArestas, &tipo_a);

    for (int i = 0; i < totalArestas; i++) {
        int id, orig, dest, peso;
        fscanf(fp, "%d %d %d %d", &id, &orig, &dest, &peso);
        arestas[i].orig = orig;
        arestas[i].dest = dest;
        arestas[i].dist = 0;
    }

    int fim;
    fscanf(fp, "%d", &fim);
    fclose(fp);
}

void construirGrafo() {
    for (int i = 0; i < totalVertices; i++)
        for (int j = 0; j < totalVertices; j++)
            matrizAdj[i][j] = INF;

    for (int i = 0; i < totalArestas; i++) {
        int a = arestas[i].orig;
        int b = arestas[i].dest;
        double dist = calcDist(vertices[a].x, vertices[a].y, vertices[b].x, vertices[b].y);
        matrizAdj[a][b] = dist;
        matrizAdj[b][a] = dist;
    }
}

void dijkstra(int inicio, int fim) {
    double dist[MAX_VERTICES];
    int prev[MAX_VERTICES], visited[MAX_VERTICES];

    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }

    dist[inicio] = 0;

    for (int i = 0; i < totalVertices; i++) {
        int u = -1;
        double min = INF;
        for (int j = 0; j < totalVertices; j++) {
            if (!visited[j] && dist[j] < min) {
                u = j;
                min = dist[j];
            }
        }

        if (u == -1) break;

        visited[u] = 1;

        for (int v = 0; v < totalVertices; v++) {
            if (matrizAdj[u][v] < INF && dist[u] + matrizAdj[u][v] < dist[v]) {
                dist[v] = dist[u] + matrizAdj[u][v];
                prev[v] = u;
            }
        }
    }

    if (dist[fim] == INF) {
        printf("Sem caminho entre %d e %d\n", inicio, fim);
        return;
    }

    printf("\nDistancia total: %.2f u. m.\n", dist[fim]);

    int path[MAX_VERTICES], path_len = 0;
    for (int v = fim; v != -1; v = prev[v])
        path[path_len++] = v;

    printf("\nCaminho (do inicio ao fim):\n");
    for (int i = path_len - 1; i >= 0; i--)
        printf("%d (x=%.2f, y=%.2f)\n", path[i], vertices[path[i]].x, vertices[path[i]].y);
}

// ========================== MAIN ==========================

int main() {
    char nomeArquivoOSM[100], nomeArquivoPOLY[100];
    int origem, destino;

    printf("Informe o nome do arquivo .osm (ex: mapa.osm): ");
    scanf("%s", nomeArquivoOSM);

    parse_osm(nomeArquivoOSM);

    int pos = RAt(".", nomeArquivoOSM);
    strcpy(nomeArquivoPOLY, Left(nomeArquivoOSM, pos));
    strcat(nomeArquivoPOLY, ".poly");

    carregarPoly(nomeArquivoPOLY);
    construirGrafo();

    printf("Total de vertices: %d\n", totalVertices);
    printf("Total de arestas : %d\n", totalArestas);

    printf("Digite o vertice de origem (0 a %d): ", totalVertices - 1);
    scanf("%d", &origem);
    printf("Digite o vertice de destino (0 a %d): ", totalVertices - 1);
    scanf("%d", &destino);

    if (origem >= 0 && origem < totalVertices && destino >= 0 && destino < totalVertices)
        dijkstra(origem, destino);
    else
        printf("Indices invalidos.\n");

    return 0;
}
