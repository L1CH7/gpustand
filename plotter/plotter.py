import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
from matplotlib import cm
import numpy as np
# from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Line3DCollection
from matplotlib.collections import LineCollection
from joblib import Parallel, delayed

def plot3d(plt_t: plt, fig_t: plt.Figure, grd_t: GridSpec, x, y, z, title, xlabel, ylabel, zlabel):
    fig_ax = fig_t.add_subplot(grd_t, projection='3d')
    
    fig_ax.set_title(title)
    fig_ax.set_xlabel(xlabel)
    fig_ax.set_ylabel(ylabel)
    fig_ax.set_zlabel(zlabel)
    fig_ax.plot_wireframe(x, y, z, rstride=1, cstride=0)


def draw_line(i, j, x, y, z):
    return [[x[i, j], y[i, j], 0], [x[i, j], y[i, j], z[i, j]]]
    
def plot3d_vertical(plt_t: plt, fig_t: plt.Figure, grd_t: GridSpec, x, y, z, title, xlabel, ylabel, zlabel):
    fig_ax = fig_t.add_subplot(grd_t, projection='3d')
        
    fig_ax.set_title(title)
    fig_ax.set_xlabel(xlabel)
    fig_ax.set_ylabel(ylabel)
    fig_ax.set_zlabel(zlabel)

    # Рисуем вертикальные линии
    lines = Parallel(n_jobs=-1)(delayed(draw_line)(i, j, x, y, z) for i in range(x.shape[0]) for j in range(x.shape[1]))
    
    # Создаем коллекцию линий
    lc = Line3DCollection(lines, colors='b')
    
    # Добавляем коллекцию на график
    fig_ax.add_collection3d(lc)

    # Установка пределов осей
    fig_ax.set_xlim(np.min(x), np.max(x))
    fig_ax.set_ylim(np.min(y), np.max(y))
    fig_ax.set_zlim(0, np.max(z) + 1)

def plot3dbars(plt_t: plt, fig_t: plt.Figure, grd_t: GridSpec, x, y, z, title, xlabel, ylabel, zlabel):
    fig_ax = fig_t.add_subplot(grd_t, projection='3d')

    viridis = cm.get_cmap('viridis', len(y))
    colors = viridis(np.linspace(0, 1, len(y)))
    # print(colors)
    for i in y:
        # print(i)
        # zs = z[i].T
        # fig_ax.bar(x[::20], z[i][::20], zs=i, color=colors[i], zdir='y', alpha=1)
        fig_ax.bar(x, z[i], zs=i, color=colors[i], zdir='y', alpha=1)
        # fig_ax.bar(x, zs)

    fig_ax.set_title(title)
    fig_ax.set_xlabel(xlabel)
    fig_ax.set_ylabel(ylabel)
    fig_ax.set_zlabel(zlabel)

def plot2d(plt_t: plt, fig_t: plt.Figure, grd_t: GridSpec, x, data, title, xlabel, ylabel, ylim=0):
    data = np.abs(data)
    if ylim==0:
        ylim=data.max()*1.1
    fig_ax = fig_t.add_subplot(grd_t)
    fig_ax.set_title(title)
    fig_ax.set_xlabel(xlabel)
    fig_ax.set_ylabel(ylabel)
    plt_t.ylim(0,ylim)
    plt_t.plot(x, data)