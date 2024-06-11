# -*- coding: utf-8 -*-
import os
import shutil
import xmind
import sys

# 获取命令行参数
# sys.argv[0] 是脚本的名称，后面的元素是传递给脚本的参数
# 例如，python script.py arg1 arg2
# sys.argv[0] = 'script.py', sys.argv[1] = 'arg1', sys.argv[2] = 'arg2'
arguments = sys.argv[1:]

current_dir_path = os.path.dirname(os.path.abspath(__file__))
ue_class_xmind = os.path.join(current_dir_path,"UEClassXmind.xmind")
ue_class_info = os.path.join(current_dir_path,"UEClassInfo.txt")
if os.path.exists(ue_class_xmind):
    os.remove(ue_class_xmind)

ue_class_tree = {
    'ue': {}
}
with open(ue_class_info,"r") as ue_class_file:
    lines = ue_class_file.readlines()
    for line in lines:
        if arguments[0] not in line:
            continue
        nodes = [node.replace('\n','') for node in line.split("->")]
        nodes.reverse()
        def insert_path(tree, nodes):
            current_level = ue_class_tree['ue']
            for node in nodes:
                if node not in current_level:
                    current_level[node] = {}
                current_level = current_level[node]
        insert_path(ue_class_tree, nodes)

xmind_instance = xmind.load(ue_class_xmind)  # 如果文件存在，加载现有的XMind文件
if xmind_instance is None:
    xmind_instance = xmind.XMindDocument()
sheet1 = xmind_instance.getPrimarySheet()
root_topic = sheet1.getRootTopic()
root_topic.setTitle("UEClass")

def insert_to_xmind(dict_tree, topic):
    for k,v in dict_tree.items():
        node_topic = topic.addSubTopic()
        node_topic.setTitle(k)
        insert_to_xmind(v,node_topic)
insert_to_xmind(ue_class_tree["ue"], root_topic)

import json
json_str = json.dumps(ue_class_tree["ue"],indent=4)
# print(json_str)
xmind.save(xmind_instance)



from graphviz import Digraph
def create_class_diagram(class_hierarchy, dot=None, parent=None):
    if dot is None:
        dot = Digraph()

    for child, grandchildren in class_hierarchy.items():
        dot.node(child, shape='rectangle')  # Change node shape to rectangle
        if parent is not None:
            dot.edge(parent, child, dir='back')  # Change edge direction to the opposite
        create_class_diagram(grandchildren, dot, child)

    return dot

dot = create_class_diagram(ue_class_tree["ue"])
dot.render('class_diagram', format='png', cleanup=True)

# Display the graph in a pop-up window
dot.view()

