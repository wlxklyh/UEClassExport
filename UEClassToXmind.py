# -*- coding: utf-8 -*-
import os
import shutil
import xmind

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



xmind.save(xmind_instance)