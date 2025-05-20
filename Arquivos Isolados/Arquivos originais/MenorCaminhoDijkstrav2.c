/*
	Instituição: INF/UFG
	Cursos: BSI e BES
	Disciplina: Algoritmos e Estruturas de Dados 2 - 2025-1
	Professor: André Luiz Moura
	Finalidade: Implementar o algoritmo do menor caminho (Dijkstra)
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_VERTICES 18
#define INF 1e9

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
	
	Vertices V[] = {{0, 	149, 200}, 
						{1, 	225, 200},  
						{2,	156.175936136879,	193.978674634193},
						{3,	164.288445856511,	189.29491496376},
						{4,	173.091034656191,	186.09103465619},
						{5,	182.316240329567,	184.46438199339},
						{6,	191.683759670433,	184.46438199339},
						{7,	200.908965343809,	186.091034656191},
						{8,	209.711554143489,	189.29491496376},
						{9,	217.824063863121,	193.978674634193},
						{10,	217.824063863121,	206.021325365807},
						{11,	209.711554143489,	210.70508503624},
						{12,	200.908965343809,	213.908965343809},
						{13,	191.683759670433,	215.53561800661},
						{14,	182.316240329567,	215.53561800661},
						{15,	173.091034656191,	213.908965343809},
						{16,	164.288445856511,	210.70508503624},
						{17,	156.175936136879,	206.021325365807}};

	totalVertices = sizeof(V) / sizeof(V[0]);
	
	for (i=0; i<totalVertices; i++)
		vertices[i] = V[i];

	Arestas A[] = {{0, 2, 0},
						{2, 3, 0},
						{3, 4, 0},
						{4, 5, 0},
						{5, 6, 0},
						{6, 7, 0},
						{7, 8, 0},
						{8, 9, 0},
						{9, 0, 0},
						{9, 10, 0},
						{10, 11, 0},
						{11, 12, 0},
						{12, 13, 0},
						{13, 14, 0},
						{14, 15, 0},
						{15, 16, 0},
						{16, 17, 0},
						{17, 0,	0}};
						
	totalArestas = sizeof(A) / sizeof(A[0]);				
		
	construirGrafo(V, A); 
	
	printf("Menor caminho entre dois vértices - algoritmo de Dijkstra\n\n");
	
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

