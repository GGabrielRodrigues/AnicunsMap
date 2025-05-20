/*
	Institui��o: INF/UFG
	Cursos: BSI e BES
	Disciplina: Algoritmos e Estruturas de Dados 2 - 2025-1
	Professor: Andr� Luiz Moura
	Finalidade: Implementar o algoritmo do menor caminho (Dijkstra)
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define INF 1e9

//
#define MAX_VERTICES 10000
#define MAX_ARESTAS 20000
//

typedef struct {
    int id;
    double x;
    double y;
} Vertices;

typedef struct {
	int orig;
	int dest;
	double dist;
} Arestas;

double matrizAdj[MAX_VERTICES][MAX_VERTICES];
Vertices vertices[MAX_VERTICES];

int totalVertices;
int totalArestas;

//
Arestas arestas[MAX_ARESTAS];
//

void carregarPoly(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo .poly");
        exit(1);
    }

    int dim, tipo_v, tipo_a;
    fscanf(fp, "%d %d %d %d", &totalVertices, &dim, &tipo_v, &tipo_a);

    for (int i = 0; i < totalVertices; i++) {
        int id;
        double x, y;
        fscanf(fp, "%d %lf %lf", &id, &x, &y);
        vertices[i].id = id;
        vertices[i].x = x;
        vertices[i].y = y;
    }

    fscanf(fp, "%d %d", &totalArestas, &tipo_a);

    for (int i = 0; i < totalArestas; i++) {
        int id, orig, dest, peso;
        fscanf(fp, "%d %d %d %d", &id, &orig, &dest, &peso);
        arestas[i].orig = orig;
        arestas[i].dest = dest;
        arestas[i].dist = 0;  // ser� calculada com base nas coordenadas
    }

    int fim;
    fscanf(fp, "%d", &fim);  // leitura do �ltimo 0
    fclose(fp);
}

double calcDist(double x0, double y0, double x1, double y1)
{
	return sqrt(pow(x0 - x1, 2.0) + pow(y0 - y1, 2.0));
}


void construirGrafo(Vertices nos[], Arestas arestas[])
{
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


void dijkstra(int inicio, int fim)
{
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

    // Reconstroi caminho
    int path[MAX_VERTICES], path_len = 0;
    for (int v = fim; v != -1; v = prev[v])
        path[path_len++] = v;

    printf("\nCaminho (do inicio ao fim):\n");
    for (int i = path_len - 1; i >= 0; i--)
        printf("%d (x=%f, y=%f)\n", path[i], vertices[path[i]].x, vertices[path[i]].y);

	system("pause > nul");
}


int main()
{
	int origem, destino;
	int i;

	carregarPoly("anicuns.poly");

	construirGrafo(vertices, arestas);

	printf("Menor caminho entre dois v�rtices - algoritmo de Dijkstra\n\n");

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

	system("pause > nul");

   return 0;
}
