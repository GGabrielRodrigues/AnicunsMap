import sys
import ctypes
import os
import time
import math

from PySide6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                               QHBoxLayout, QPushButton, QLabel, QComboBox,
                               QCheckBox, QSlider, QTableWidget, QTableWidgetItem,
                               QGraphicsView, QGraphicsScene, QMessageBox, QHeaderView,
                               QGroupBox, QRadioButton, QFileDialog) 
from PySide6.QtGui import QPixmap, QColor, QPen, QBrush, QFont, QPainter, QClipboard, QTransform
from PySide6.QtCore import Qt, QPointF, QRectF

# --- 1. Configuração e Carregamento da Biblioteca C ---
if sys.platform.startswith('linux'):
    LIB_NAME = 'integracao_lib.so'
elif sys.platform.startswith('win'):
    LIB_NAME = 'integracao_lib.dll'
else:
    LIB_NAME = 'integracao_lib.dylib' 

# Caminho base do projeto (a pasta que contém backend_c e frontend_python)
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIB_PATH = os.path.join(BASE_DIR, 'backend_c', 'lib', LIB_NAME)
DATA_DIR = os.path.join(BASE_DIR, 'data')

os.makedirs(DATA_DIR, exist_ok=True)

try:
    lib = ctypes.CDLL(LIB_PATH)
except OSError as e:
    QMessageBox.critical(None, "Erro Crítico", f"Erro ao carregar a biblioteca C: {e}\nVerifique se o arquivo '{LIB_PATH}' existe. Compile o backend C primeiro.")
    sys.exit(1)

# --- 2. Mapeamento de Funções C e Estruturas ---
class DijkstraResult(ctypes.Structure):
    _fields_ = [
        ("distancia_total", ctypes.c_double),
        ("num_nos_explorados", ctypes.c_int),
        ("tempo_processamento", ctypes.c_double),
        ("path_len", ctypes.c_int),
    ]

# Definindo os tipos de argumentos e retorno para cada função C
lib.parse_osm.argtypes = [ctypes.c_char_p]
lib.parse_osm.restype = None

lib.carregarPoly.argtypes = [ctypes.c_char_p]
lib.carregarPoly.restype = None

lib.construirGrafo.argtypes = []
lib.construirGrafo.restype = None

lib.get_total_vertices.argtypes = []
lib.get_total_vertices.restype = ctypes.c_int

lib.get_vertex_coords.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double)]
lib.get_vertex_coords.restype = None

lib.get_total_edges_from_poly.argtypes = []
lib.get_total_edges_from_poly.restype = ctypes.c_int

lib.get_edge_endpoints.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
lib.get_edge_endpoints.restype = None

lib.get_edge_info.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int)]
lib.get_edge_info.restype = None

MAX_PATH_LEN = 10000
PathArray = ctypes.c_int * MAX_PATH_LEN

lib.dijkstra_gui.argtypes = [ctypes.c_int, ctypes.c_int, PathArray, ctypes.c_int]
lib.dijkstra_gui.restype = DijkstraResult

# Mapeamento para funções de edição do grafo (RF05)
lib.adicionar_vertice.argtypes = [ctypes.c_double, ctypes.c_double]
lib.adicionar_vertice.restype = ctypes.c_int

lib.adicionar_aresta.argtypes = [ctypes.c_int, ctypes.c_int]
lib.adicionar_aresta.restype = ctypes.c_int

lib.remover_aresta.argtypes = [ctypes.c_int, ctypes.c_int]
lib.remover_aresta.restype = ctypes.c_int

lib.remover_vertice.argtypes = [ctypes.c_int] 
lib.remover_vertice.restype = ctypes.c_int

INF_C = 1e9

# --- Classe Principal da Janela ---
class SistemaNavegacaoApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Sistema de Navegação - Dijkstra")
        self.setGeometry(100, 100, 1200, 800)

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.main_layout = QHBoxLayout(self.central_widget)

        self.total_vertices = 0
        self.vertices_coords = []
        self.edges_data = []
        self.selected_origin = -1
        self.selected_destination = -1
        self.current_path_ids = []

        self.editing_mode = "Navegação" 
        self.first_edge_vertex = -1 

        self.zoom_level = 0  
        self.zoom_factor = 1.25 

        self.init_ui()
        anicuns_osm_path = os.path.join(DATA_DIR, 'anicuns.osm')
        self.load_map_data(anicuns_osm_path)


    def refresh_graph_data_from_c(self):
        # Resetar dados existentes
        self.vertices_coords = []
        self.edges_data = []
        self.total_vertices = lib.get_total_vertices()

        # Recarregar vértices
        x_val, y_val = ctypes.c_double(), ctypes.c_double()
        for i in range(self.total_vertices):
            lib.get_vertex_coords(i, ctypes.byref(x_val), ctypes.byref(y_val))
            self.vertices_coords.append((x_val.value, y_val.value))

        # Recarregar arestas
        total_edges_from_poly = lib.get_total_edges_from_poly()
        orig_id_c, dest_id_c = ctypes.c_int(), ctypes.c_int()
        weight_c = ctypes.c_double()
        oneway_c = ctypes.c_int()
        for i in range(total_edges_from_poly):
            lib.get_edge_info(i, ctypes.byref(orig_id_c), ctypes.byref(dest_id_c), ctypes.byref(weight_c), ctypes.byref(oneway_c))
            self.edges_data.append((orig_id_c.value, dest_id_c.value, weight_c.value, oneway_c.value))

        self.update_map_display()


    def init_ui(self):
        # --- Painel Esquerdo: Controles ---
        self.controls_panel = QWidget()
        self.controls_layout = QVBoxLayout(self.controls_panel)
        self.controls_panel.setFixedWidth(350) 

        self.controls_layout.addWidget(QLabel("<h1>Sistema de Navegação</h1>"))
        self.controls_layout.addWidget(QLabel("<h3>Algoritmo de Dijkstra</h3>"))

        self.controls_layout.addWidget(QLabel("<b>Controle de Mapa:</b>"))
        
        # --- NOVO BLOCO PARA CARREGAR ARQUIVOS ---
        self.load_file_group_box = QGroupBox("Carregar Arquivo OSM")
        self.load_file_layout = QVBoxLayout()

        self.current_osm_file_label = QLabel("Nenhum arquivo OSM carregado.")
        self.load_file_layout.addWidget(self.current_osm_file_label)

        self.load_osm_button = QPushButton("Carregar Outro Arquivo OSM")
        self.load_osm_button.clicked.connect(self.load_new_osm_file)
        self.load_file_layout.addWidget(self.load_osm_button)

        self.load_file_group_box.setLayout(self.load_file_layout)
        self.controls_layout.addWidget(self.load_file_group_box)
        # --- FIM DO NOVO BLOCO ---

        self.controls_layout.addSpacing(10)

        self.controls_layout.addWidget(QLabel("<b>Visualização:</b>"))
        self.color_combo = QComboBox()
        self.color_combo.addItems(["Gray", "Black", "Blue", "Green", "Red"])
        self.color_combo.setCurrentText("Gray")
        self.color_combo.currentIndexChanged.connect(self.update_map_display)
        self.controls_layout.addWidget(self.color_combo)
        
        self.point_size_slider = QSlider(Qt.Horizontal)
        self.point_size_slider.setRange(1, 10); self.point_size_slider.setValue(3)
        self.point_size_slider.valueChanged.connect(self.update_map_display)
        self.controls_layout.addWidget(self.point_size_slider)

        self.check_num_vertices = QCheckBox("Numerar vértices")
        self.check_num_vertices.setChecked(False) # Inicia desmarcado
        self.check_num_vertices.stateChanged.connect(self.update_map_display)
        self.controls_layout.addWidget(self.check_num_vertices)

        self.check_label_edges = QCheckBox("Rotular arestas")
        self.check_label_edges.setChecked(False) # Inicia desmarcado
        self.check_label_edges.stateChanged.connect(self.update_map_display)
        self.controls_layout.addWidget(self.check_label_edges)
        
        self.controls_layout.addSpacing(10)

        # Layout para os botões de zoom
        zoom_layout = QHBoxLayout()
        self.zoom_in_btn = QPushButton("Zoom +")
        self.zoom_out_btn = QPushButton("Zoom -")
        zoom_layout.addWidget(self.zoom_in_btn)
        zoom_layout.addWidget(self.zoom_out_btn)
        
        # Conecta os botões às futuras funções de zoom
        self.zoom_in_btn.clicked.connect(self.zoom_in)
        self.zoom_out_btn.clicked.connect(self.zoom_out)

        self.controls_layout.addLayout(zoom_layout)
        # -------------------------
        
        self.controls_layout.addSpacing(10)
        
        # Controles de Edição (RF05)
        self.edit_group_box = QGroupBox("Modo de Edição")
        self.edit_layout = QVBoxLayout()
        self.radio_nav = QRadioButton("Navegação")
        self.radio_nav.setChecked(True)
        self.radio_add_v = QRadioButton("Adicionar Vértice")
        self.radio_add_e = QRadioButton("Adicionar Aresta")
        self.radio_rem_e = QRadioButton("Remover Aresta")
        self.radio_rem_v = QRadioButton("Remover Vértice") 
        
        self.radio_nav.toggled.connect(lambda: self.set_editing_mode("Navegação"))
        self.radio_add_v.toggled.connect(lambda: self.set_editing_mode("Adicionar Vértice"))
        self.radio_add_e.toggled.connect(lambda: self.set_editing_mode("Adicionar Aresta"))
        self.radio_rem_e.toggled.connect(lambda: self.set_editing_mode("Remover Aresta"))
        self.radio_rem_v.toggled.connect(lambda: self.set_editing_mode("Remover Vértice")) 

        self.edit_layout.addWidget(self.radio_nav)
        self.edit_layout.addWidget(self.radio_add_v)
        self.edit_layout.addWidget(self.radio_add_e)
        self.edit_layout.addWidget(self.radio_rem_e)
        self.edit_layout.addWidget(self.radio_rem_v) 
        self.edit_group_box.setLayout(self.edit_layout)
        self.controls_layout.addWidget(self.edit_group_box)
        self.controls_layout.addSpacing(10)

        self.controls_layout.addWidget(QLabel("<b>Dijkstra:</b>"))
        self.origin_label = QLabel("Origem: Não selecionado")
        self.dest_label = QLabel("Destino: Não selecionado")
        self.controls_layout.addWidget(self.origin_label)
        self.controls_layout.addWidget(self.dest_label)

        self.calculate_path_btn = QPushButton("Traçar Menor Caminho")
        self.calculate_path_btn.clicked.connect(self.calculate_shortest_path)
        self.calculate_path_btn.setEnabled(False)
        self.controls_layout.addWidget(self.calculate_path_btn)

        self.copy_image_btn = QPushButton("Copiar Imagem do Mapa")
        self.copy_image_btn.clicked.connect(self.copy_map_to_clipboard)
        self.controls_layout.addWidget(self.copy_image_btn)
        self.controls_layout.addSpacing(10)

        self.controls_layout.addWidget(QLabel("<b>Informações do Caminho:</b>"))
        self.total_dist_label = QLabel("Distância Total: --")
        self.controls_layout.addWidget(self.total_dist_label)
        self.num_vertices_path_label = QLabel("Vértices na Rota: --")
        self.controls_layout.addWidget(self.num_vertices_path_label)
        self.path_sequence_label = QLabel("Rota: --") 
        self.path_sequence_label.setWordWrap(True) # Para quebra de linha
        self.controls_layout.addWidget(self.path_sequence_label)
        

        self.controls_layout.addSpacing(10)
        
        self.controls_layout.addWidget(QLabel("<b>Estatísticas de Execução:</b>"))
        self.processing_time_label = QLabel("Tempo de Processamento: --")
        self.controls_layout.addWidget(self.processing_time_label)
        self.explored_nodes_label = QLabel("Nós Explorados: --")
        self.controls_layout.addWidget(self.explored_nodes_label)
        self.controls_layout.addSpacing(10)

        self.reset_btn = QPushButton("Resetar Seleção")
        self.reset_btn.clicked.connect(self.reset_selections)
        self.controls_layout.addWidget(self.reset_btn)

        self.exit_btn = QPushButton("Sair")
        self.exit_btn.clicked.connect(self.close)
        self.controls_layout.addWidget(self.exit_btn)
        self.controls_layout.addStretch()
        self.main_layout.addWidget(self.controls_panel)

        # --- Painel Direito: Área de Desenho do Mapa ---
        self.graphics_scene = QGraphicsScene()
        self.graphics_view = QGraphicsView(self.graphics_scene)
        self.graphics_view.setRenderHint(QPainter.Antialiasing)
        self.graphics_view.setMouseTracking(True)
        self.main_layout.addWidget(self.graphics_view)
        self.graphics_view.mousePressEvent = self.map_clicked
        self.statusBar().showMessage("Inicializando...")

    def load_map_data(self, osm_file_path): 
        # Determina o caminho do arquivo .poly a partir do nome do arquivo .osm
        # Ex: /data/mapa.osm -> /data/mapa.poly
        poly_file_name = os.path.basename(osm_file_path).replace('.osm', '.poly')
        poly_file_path = os.path.join(DATA_DIR, poly_file_name)

        if not os.path.exists(osm_file_path):
            QMessageBox.critical(self, "Erro", f"Arquivo de mapa não encontrado: {osm_file_path}\nVerifique se ele existe no diretório 'data'.")
            self.statusBar().showMessage(f"Erro: {os.path.basename(osm_file_path)} não encontrado.")
            # Limpa dados anteriores em caso de falha de carregamento
            self.graphics_scene.clear()
            self.vertices_coords = []
            self.edges_data = []
            self.current_osm_file_label.setText("Erro ao carregar arquivo.")
            return

        self.statusBar().showMessage(f"Processando Mapa: Convertendo {os.path.basename(osm_file_path)} para POLY...")
        QApplication.processEvents()
        try:
            lib.parse_osm(osm_file_path.encode('utf-8'))
        except Exception as e:
            QMessageBox.critical(self, "Erro na Conversão", f"Não foi possível converter o arquivo OSM: {e}")
            self.statusBar().showMessage("Erro na conversão do mapa.")
            self.graphics_scene.clear()
            self.vertices_coords = []
            self.edges_data = []
            self.current_osm_file_label.setText("Erro na conversão.")
            return
        
        self.statusBar().showMessage(f"Processando Mapa: Carregando grafo do arquivo {poly_file_name}...")
        QApplication.processEvents()
        try:
            lib.carregarPoly(poly_file_path.encode('utf-8'))
            lib.construirGrafo()

            self.total_vertices = lib.get_total_vertices()
            if self.total_vertices == 0:
                QMessageBox.warning(self, "Aviso", "O mapa foi carregado, mas não contém vértices.")
                # Limpa a exibição e os dados se o mapa estiver vazio
                self.graphics_scene.clear()
                self.vertices_coords = []
                self.edges_data = []
                self.current_osm_file_label.setText(f"Arquivo carregado: {os.path.basename(osm_file_path)} (Vazio)")
                return

            self.vertices_coords = []
            x_val, y_val = ctypes.c_double(), ctypes.c_double()
            for i in range(self.total_vertices):
                lib.get_vertex_coords(i, ctypes.byref(x_val), ctypes.byref(y_val))
                self.vertices_coords.append((x_val.value, y_val.value))
            
            self.edges_data = []
            total_edges_from_poly = lib.get_total_edges_from_poly()
            orig_id_c, dest_id_c = ctypes.c_int(), ctypes.c_int()
            weight_c = ctypes.c_double()
            oneway_c = ctypes.c_int() 
            for i in range(total_edges_from_poly):
                lib.get_edge_info(i, ctypes.byref(orig_id_c), ctypes.byref(dest_id_c), ctypes.byref(weight_c), ctypes.byref(oneway_c))
                self.edges_data.append((orig_id_c.value, dest_id_c.value, weight_c.value, oneway_c.value)) 

            self.update_map_display()
            self.statusBar().showMessage(f"Mapa carregado com {self.total_vertices} vértices e {len(self.edges_data)} arestas.")
            self.current_osm_file_label.setText(f"Arquivo carregado: {os.path.basename(osm_file_path)}") 

            # ================== BLOCO DE ZOOM AJUSTÁVEL==================
            full_rect = self.graphics_scene.sceneRect()
            if full_rect.isEmpty(): # Adicionado para evitar erro se o mapa estiver vazio
                return
            center_x = full_rect.center().x()
            center_y = full_rect.center().y()
            self.zoom_level = 0 
            self.apply_zoom() 
            self.graphics_view.centerOn(center_x, center_y) # Centraliza a view no mapa carregado
            # =============================================================

        except Exception as e:
            QMessageBox.critical(self, "Erro ao Carregar Grafo", f"Não foi possível carregar o grafo do arquivo .poly: {e}")
            self.statusBar().showMessage("Erro ao carregar o mapa.")
            self.graphics_scene.clear()
            self.vertices_coords = []
            self.edges_data = []
            self.current_osm_file_label.setText("Erro ao carregar arquivo.")

    def update_map_display(self):
        self.graphics_scene.clear()
        if not self.vertices_coords: return

        font = QFont()

        min_x = min(p[0] for p in self.vertices_coords)
        max_x = max(p[0] for p in self.vertices_coords)
        min_y = min(p[1] for p in self.vertices_coords)
        max_y = max(p[1] for p in self.vertices_coords)
        
        margin = 20
        self.graphics_scene.setSceneRect(min_x - margin, min_y - margin, (max_x - min_x) + 2 * margin, (max_y - min_y) + 2 * margin)

        edge_pen = QPen(QColor(self.color_combo.currentText().lower()).lighter(150), 0.5)
        arrow_pen = QPen(QColor("purple"), 1.5, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin)

        for u, v, weight, is_oneway in self.edges_data:
            if u < len(self.vertices_coords) and v < len(self.vertices_coords):
                x1, y1 = self.vertices_coords[u]
                x2, y2 = self.vertices_coords[v]
                self.graphics_scene.addLine(x1, y1, x2, y2, edge_pen)

                if is_oneway:
                    mid_x = (x1 + x2) / 2
                    mid_y = (y1 + y2) / 2
                    vec_x = x2 - x1
                    vec_y = y2 - y1
                    length = math.sqrt(vec_x**2 + vec_y**2)
                    if length > 0:
                        norm_x = vec_x / length
                        norm_y = vec_y / length
                        arrow_size = 4
                        angle = math.radians(135)
                        p2_x = mid_x + (norm_x * math.cos(angle) - norm_y * math.sin(angle)) * arrow_size
                        p2_y = mid_y + (norm_y * math.cos(angle) + norm_x * math.sin(angle)) * arrow_size
                        angle = math.radians(-135)
                        p3_x = mid_x + (norm_x * math.cos(angle) - norm_y * math.sin(angle)) * arrow_size
                        p3_y = mid_y + (norm_y * math.cos(angle) + norm_x * math.sin(angle)) * arrow_size
                        self.graphics_scene.addLine(p2_x, p2_y, mid_x, mid_y, arrow_pen)
                        self.graphics_scene.addLine(p3_x, p3_y, mid_x, mid_y, arrow_pen)

        if self.check_label_edges.isChecked():
            font.setPointSize(5)
            font.setBold(True)
            for u, v, weight, is_oneway in self.edges_data:
                 if u < len(self.vertices_coords) and v < len(self.vertices_coords):
                    x1, y1 = self.vertices_coords[u]
                    x2, y2 = self.vertices_coords[v]
                    mid_x = (x1 + x2) / 2
                    mid_y = (y1 + y2) / 2
                    text_item = self.graphics_scene.addText(str(int(weight)), font)
                    text_item.setDefaultTextColor(QColor("darkblue"))
                    text_rect = text_item.boundingRect()
                    text_item.setPos(mid_x - text_rect.width() / 2, mid_y - text_rect.height() / 2)

        point_size = self.point_size_slider.value()
        for i, (x, y) in enumerate(self.vertices_coords):
            brush = QBrush(QColor(self.color_combo.currentText().lower()))
            pen = QPen(Qt.black, 0.5)
            if i == self.first_edge_vertex:
                brush.setColor(QColor("orange")); pen.setColor(QColor("darkred")); pen.setWidth(2)
            elif i == self.selected_origin:
                brush.setColor(QColor("green")); pen.setColor(QColor("darkgreen")); pen.setWidth(2)
            elif i == self.selected_destination:
                brush.setColor(QColor("red")); pen.setColor(QColor("darkred")); pen.setWidth(2)
            self.graphics_scene.addEllipse(x - point_size/2, y - point_size/2, point_size, point_size, pen, brush)

            if self.check_num_vertices.isChecked():
                font.setPointSize(6)
                font.setBold(False) # Resetar bold para não afetar os números dos vértices
                text_item = self.graphics_scene.addText(str(i), font)
                text_item.setPos(x + point_size/2, y + point_size/2)
        
        if self.current_path_ids:
            path_pen = QPen(QColor("blue"), 2.5)
            for i in range(len(self.current_path_ids) - 1):
                p1_id, p2_id = self.current_path_ids[i], self.current_path_ids[i+1]
                if p1_id < len(self.vertices_coords) and p2_id < len(self.vertices_coords):
                    x1, y1 = self.vertices_coords[p1_id]
                    x2, y2 = self.vertices_coords[p2_id]
                    self.graphics_scene.addLine(x1, y1, x2, y2, path_pen)

        
    
    def map_clicked(self, event):
        if event.button() != Qt.LeftButton:
            return

        scene_pos = self.graphics_view.mapToScene(event.position().toPoint())

        # --- MODO DE ADICIONAR VÉRTICE ---
        if self.editing_mode == "Adicionar Vértice":
            new_id = lib.adicionar_vertice(scene_pos.x(), scene_pos.y())
            if new_id != -1:
                self.total_vertices += 1
                self.vertices_coords.append((scene_pos.x(), scene_pos.y()))
                self.update_map_display()
            else:
                QMessageBox.warning(self, "Erro", "Não foi possível adicionar o vértice. Limite máximo atingido.")
            return

        # Encontra o vértice mais próximo do clique para os outros modos
        clicked_vertex_id, min_dist_sq = -1, float('inf')
        tolerance_sq = (self.point_size_slider.value() * 2.0)**2
        for i, (vx, vy) in enumerate(self.vertices_coords):
            dist_sq = (scene_pos.x() - vx)**2 + (scene_pos.y() - vy)**2
            if dist_sq < min_dist_sq and dist_sq < tolerance_sq:
                min_dist_sq = dist_sq
                clicked_vertex_id = i
        
        if clicked_vertex_id == -1:
            self.reset_selections() # Se clicar fora de um vértice, limpa seleções
            return

        # --- MODO DE ADICIONAR ARESTA ---
        if self.editing_mode == "Adicionar Aresta":
            if self.first_edge_vertex == -1: # Primeiro clique
                self.first_edge_vertex = clicked_vertex_id
                self.statusBar().showMessage(f"Vértice {clicked_vertex_id} selecionado. Clique em outro para criar a aresta.")
            else: # Segundo clique
                if self.first_edge_vertex != clicked_vertex_id:
                    if lib.adicionar_aresta(self.first_edge_vertex, clicked_vertex_id):
                
                        total_arestas_c = lib.get_total_edges_from_poly()
                        orig_c, dest_c = ctypes.c_int(), ctypes.c_int()
                        weight_c = ctypes.c_double()
                        oneway_c = ctypes.c_int() 
                        lib.get_edge_info(total_arestas_c - 1, ctypes.byref(orig_c), ctypes.byref(dest_c), ctypes.byref(weight_c), ctypes.byref(oneway_c)) 
                
                # Adiciona a aresta completa (com o peso correto do C) na lista do python
                        self.edges_data.append((orig_c.value, dest_c.value, weight_c.value, oneway_c.value)) 
                
                        self.statusBar().showMessage(f"Aresta criada entre {self.first_edge_vertex} e {clicked_vertex_id}.")
                    else:
                        QMessageBox.warning(self, "Erro", "Não foi possível adicionar a aresta.")
                self.first_edge_vertex = -1 # Reseta para a próxima aresta
            self.update_map_display()
            return
            
        # --- MODO DE REMOVER ARESTA ---
        if self.editing_mode == "Remover Aresta":
            if self.first_edge_vertex == -1: # Primeiro clique
                self.first_edge_vertex = clicked_vertex_id
                self.statusBar().showMessage(f"Vértice {clicked_vertex_id} selecionado. Clique no vértice conectado para remover a aresta.")
            else: # Segundo clique
                if self.first_edge_vertex != clicked_vertex_id:
                    if lib.remover_aresta(self.first_edge_vertex, clicked_vertex_id):
                        # Remove a aresta da lista do python para exibição imediata
                        edge_to_remove = -1
                        for i, edge_tuple in enumerate(self.edges_data):
                            u, v, _, _ = edge_tuple 
                            if (u == self.first_edge_vertex and v == clicked_vertex_id) or \
                               (v == self.first_edge_vertex and u == clicked_vertex_id):
                                edge_to_remove = i
                                break
                        if edge_to_remove != -1:
                            self.edges_data.pop(edge_to_remove)
                        self.statusBar().showMessage(f"Aresta entre {self.first_edge_vertex} e {clicked_vertex_id} removida.")
                    else:
                        self.statusBar().showMessage(f"Não há aresta entre {self.first_edge_vertex} e {clicked_vertex_id}.")
                self.first_edge_vertex = -1 # Reseta para a próxima
            self.update_map_display()
            return
            # --- MODO DE REMOVER VÉRTICE ---
        if self.editing_mode == "Remover Vértice":
            if clicked_vertex_id != -1:
                reply = QMessageBox.question(self, "Confirmar Remoção", 
                                             f"Tem certeza que deseja remover o vértice {clicked_vertex_id} e todas as arestas conectadas a ele?", 
                                             QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
                if reply == QMessageBox.Yes:
                    if lib.remover_vertice(clicked_vertex_id):
                        
                        self.refresh_graph_data_from_c()
                        self.statusBar().showMessage(f"Vértice {clicked_vertex_id} e arestas conectadas removidos.")
                        self.reset_selections() # Resetar seleções para evitar referências inválidas
                    else:
                       QMessageBox.warning(self, "Erro", "Não foi possível remover o vértice.")
            else:
                self.statusBar().showMessage("Clique em um vértice para removê-lo.")
            return 


        # --- MODO DE NAVEGAÇÃO (DEFAULT) ---
        if self.editing_mode == "Navegação":
            if self.selected_origin == -1 or self.selected_origin == clicked_vertex_id:
                self.selected_origin = -1 if self.selected_origin == clicked_vertex_id else clicked_vertex_id
            elif self.selected_destination == -1 or self.selected_destination == clicked_vertex_id:
                self.selected_destination = -1 if self.selected_destination == clicked_vertex_id else clicked_vertex_id
            else:
                self.selected_destination = clicked_vertex_id
            self.update_ui_selections()
            self.update_map_display()

    def update_ui_selections(self):
        self.origin_label.setText(f"Origem: {self.selected_origin}" if self.selected_origin != -1 else "Origem: Não selecionado")
        self.dest_label.setText(f"Destino: {self.selected_destination}" if self.selected_destination != -1 else "Destino: Não selecionado")
        self.calculate_path_btn.setEnabled(self.selected_origin != -1 and self.selected_destination != -1)
        if not self.calculate_path_btn.isEnabled():
            self.reset_dijkstra_results()

    def calculate_shortest_path(self):
        if self.selected_origin == -1 or self.selected_destination == -1: return

        self.statusBar().showMessage("Calculando menor caminho...")
        QApplication.processEvents()

        path_buffer = PathArray()
        try:
            result = lib.dijkstra_gui(self.selected_origin, self.selected_destination, path_buffer, MAX_PATH_LEN)
        except Exception as e:
            QMessageBox.critical(self, "Erro no Dijkstra", f"Erro ao chamar a função Dijkstra no C: {e}")
            self.statusBar().showMessage("Erro no cálculo do caminho."); return

        if result.distancia_total >= INF_C:
            QMessageBox.information(self, "Caminho Não Encontrado", f"Não foi possível encontrar um caminho entre {self.selected_origin} e {self.selected_destination}.")
            self.statusBar().showMessage("Caminho não encontrado.")
        else:
            self.total_dist_label.setText(f"Distância Total: {result.distancia_total:.2f} m") # unidade de medida
            self.num_vertices_path_label.setText(f"Vértices na Rota: {result.path_len}")
            self.current_path_ids = list(path_buffer[:result.path_len])
            self.path_sequence_label.setText(f"Rota: {' -> '.join(map(str, self.current_path_ids))}")
            self.processing_time_label.setText(f"Tempo de Processamento: {result.tempo_processamento:.6f} s")
            self.explored_nodes_label.setText(f"Nós Explorados: {result.num_nos_explorados}")
            self.update_map_display()
            self.statusBar().showMessage("Caminho calculado e exibido.")

    def reset_selections(self):
        self.selected_origin = -1
        self.selected_destination = -1
        self.update_ui_selections()
        self.update_map_display()
        self.statusBar().showMessage("Seleção e resultados reiniciados.")

    def reset_dijkstra_results(self):
        self.total_dist_label.setText("Distância Total: --")
        self.num_vertices_path_label.setText("Vértices na Rota: --")
        self.path_sequence_label.setText("Rota: --")
        self.processing_time_label.setText("Tempo de Processamento: --")
        self.explored_nodes_label.setText("Nós Explorados: --")
        self.current_path_ids = []
    
    def copy_map_to_clipboard(self):
        if self.graphics_scene.itemsBoundingRect().isEmpty():
            QMessageBox.warning(self, "Erro", "Não há conteúdo no mapa para copiar.")
            return
        pixmap = QPixmap(self.graphics_view.size())
        pixmap.fill(Qt.white)
        painter = QPainter(pixmap)
        self.graphics_scene.render(painter, target=self.graphics_scene.itemsBoundingRect(), source=self.graphics_scene.itemsBoundingRect())
        painter.end()
        QApplication.clipboard().setPixmap(pixmap)
        self.statusBar().showMessage("Imagem do mapa copiada para a área de transferência!")
    
# ================== NOVAS FUNÇÕES DE ZOOM ==================
    def zoom_in(self):
        self.zoom_level += 1
        self.apply_zoom()

    def zoom_out(self):
        self.zoom_level -= 1
        self.apply_zoom()

    def apply_zoom(self):
        # Calcula a escala com base no nível e fator de zoom
        scale = self.zoom_factor ** self.zoom_level
        
        # Cria uma matriz de transformação
        transform = QTransform()
        transform.scale(scale, scale)
        
        # Aplica a transformação à view
        self.graphics_view.setTransform(transform)

    def wheelEvent(self, event):
        # Sobrescreve o evento da roda do mouse da janela principal
        if event.angleDelta().y() > 0:
            self.zoom_in()
        else:
            self.zoom_out()
    # ==========================================================

    def set_editing_mode(self, mode):
        # Reseta seleções ao trocar de modo para evitar confusão
        self.first_edge_vertex = -1
        self.reset_selections()
        self.editing_mode = mode
        self.statusBar().showMessage(f"Modo de edição alterado para: {mode}")

    # Função para carregar um novo arquivo OSM
    def load_new_osm_file(self):
        file_dialog = QFileDialog(self)
        file_dialog.setNameFilter("OpenStreetMap Files (*.osm)")
        file_dialog.setDirectory(DATA_DIR) # Começa no diretório 'data'
        
        if file_dialog.exec():
            selected_files = file_dialog.selectedFiles()
            if selected_files:
                osm_file_path = selected_files[0]
                self.reset_selections() # Limpa as seleções anteriores antes de carregar um novo mapa
                self.reset_dijkstra_results() # Limpa resultados do Dijkstra
                self.load_map_data(osm_file_path) # Chama a função de carregamento com o novo arquivo
        else:
            self.statusBar().showMessage("Nenhum arquivo OSM selecionado.")

    # Função para recarregar os dados do grafo da biblioteca C
    def refresh_graph_data_from_c(self):
        self.vertices_coords = []
        self.edges_data = []
        self.total_vertices = lib.get_total_vertices()

        x_val, y_val = ctypes.c_double(), ctypes.c_double()
        for i in range(self.total_vertices):
            lib.get_vertex_coords(i, ctypes.byref(x_val), ctypes.byref(y_val))
            self.vertices_coords.append((x_val.value, y_val.value))
        
        total_edges_from_poly = lib.get_total_edges_from_poly()
        orig_id_c, dest_id_c = ctypes.c_int(), ctypes.c_int()
        weight_c = ctypes.c_double()
        oneway_c = ctypes.c_int()
        for i in range(total_edges_from_poly):
            lib.get_edge_info(i, ctypes.byref(orig_id_c), ctypes.byref(dest_id_c), ctypes.byref(weight_c), ctypes.byref(oneway_c))
            self.edges_data.append((orig_id_c.value, dest_id_c.value, weight_c.value, oneway_c.value))
        
        self.update_map_display()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = SistemaNavegacaoApp()
    window.show()
    sys.exit(app.exec())