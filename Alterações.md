# Histórico de Commits

Histórico de alterações feitas nessa branch.

## Commit 1

- Adição de uma barra de rolagem no painel esquerdo que permite "scrollar" -- isso resolve o problema de algumas caixas ficarem cortadas em tela cheia;
- Alteração na forma como se dá zoom: agora o zoom é no scroll do mouse, não mais nos botões de zoom-in e zoom-out presentes no painel esquerdo;
- Alteração na movimentação do mapa: retirei as barras de rolagem e fiz a movimentação ser por meio do botão direito do mouse. É só segurar e arrastar para direção desejada;
- Mudança no zoom inicial da aplicação. Eu percebi que, quando a gente abre a aplicação, ela está com um zoom numa área sem nenhum ponto ou aresta, então a tela do mapa fica toda sem nada. Mudei isso e deixei para enquadrar todo o mapa ao abrir a aplicação.
  - O problema disso é que, por algum motivo, tem mais pontos e arestas do que foi exportado no OpenStreetMap,e tem algumas arestas que são extremamente longas; isso faz com que o enquadramento fique com um zoom muito longe. Eu tentei arrumar isso colocando um multiplicador de zoom inicial e até deu certo, mas o zoom pode ser excessivo quando outros mapas são carregados. Peço que testem isso.

## Commit 2

- Troquei a borda dos vértices de preto para um roxo;
- Troquei a cor do path para laranja e aumentei a grossura dele;
- Troquei a cor dos pontos de início e destino para tons de laranja e aumentei os seus tamanhos;
- Troquei a cor das arestas para uma mais clara;
- No painel esquerdo, aumentei o tamanho padrão dos vértices (achei que estava muito pequeno) na barrinha que permite ajustar;
- Coloquei um nome para identificar a barrinha que permite aumentar/diminuir o tamanho dos vértices

## Commit 3

* Adição do *run_app.bat* e exclusão do *build_and_run.bat*:
  * Para resolver o problema da *integracao.dll* estar sendo compilada em 32bits por conta de versões desatualizadas do MinGW, agora essa dll já vai ficar compilada em 64bits, sem a necessidade de compilar. Assim, o *.bat* que rodava a aplicação não mais compila ela. Eu não sei se isso está funcionando para todas as máquinas, então eu peço que testem isso rápido para podermos resolver logo se estiver com problema;
  * Outra feature desse novo *run_app.bat* é que ele já faz automaticamente todo o processo de criar ambiente virtual, ativar ambiente virtual, baixar pyside6 e depois rodar: agora, tudo isso é feito simplesmente rodando esse *.bat*;
  * Ainda falta fazer esse processo para o Linux, mas eu não sei direito como funciona
