1. Adição de uma barra de rolagem no painel esquerdo que permite "scrollar" -- isso resolve o problema de algumas caixas ficarem cortadas em tela cheia;

2. Alteração na forma como se dá zoom: agora o zoom é no scroll do mouse, não mais nos botões de zoom-in e zoom-out presentes no painel esquerdo;

3. Alteração na movimentação do mapa: retirei as barras de rolagem e fiz a movimentação ser por meio do botão direito do mouse. É só segurar e arrastar para direção desejada;

4. Mudança no zoom inicial da aplicação. Eu percebi que, quando a gente abre a aplicação, ela está com um zoom numa área sem nenhum ponto ou aresta, então a tela do mapa fica toda sem nada. Mudei isso e deixei para enquadrar todo o mapa ao abrir a aplicação
	- O problema disso é que, por algum motivo, tem mais pontos e arestas do que foi exportado no OpenStreetMap,e tem algumas arestas que são extremamente longas; isso faz com que o enquadramento fique com um zoom muito longe. Eu tentei arrumar isso colocando um multiplicador de zoom inicial e até deu certo, mas o zoom pode ser excessivo quando outros mapas são carregados. Peço que testem isso.