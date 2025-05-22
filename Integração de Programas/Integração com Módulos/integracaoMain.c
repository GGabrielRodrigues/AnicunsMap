#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "integracao.h"


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
