#-*- coding:utf-8 -*-
from xml.dom.minidom import Document
import os
import glob
import numpy as np

def run_sys_command(command_str,output_type=""):
    print("command_str", command_str)
    import subprocess
    
    if output_type == "FILE":
#         fdout = open("/tmp/tmpSysCommand.out", 'w')
        fderr = open("/tmp/tmpSysCommand.err", 'w')
    else:
        fderr = subprocess.PIPE
     
    p = subprocess.Popen(command_str, shell=True, stdout=subprocess.PIPE, stderr=fderr)  
    p.wait()
    stdout,stderr = p.communicate()
    print('stdout : ',stdout)
    print('stderr : ',stderr)

    return stdout

def build_voc_dirs(outdir):
    mkdir = lambda dir: os.makedirs(dir) if not os.path.exists(dir) else None
    mkdir(outdir)
    mkdir(os.path.join(outdir, 'Annotations'))
    mkdir(os.path.join(outdir, 'ImageSets'))
    mkdir(os.path.join(outdir, 'ImageSets', 'Layout'))
    mkdir(os.path.join(outdir, 'ImageSets', 'Main'))
    mkdir(os.path.join(outdir, 'ImageSets', 'Segmentation'))
    mkdir(os.path.join(outdir, 'JPEGImages'))
    mkdir(os.path.join(outdir, 'SegmentationClass'))
    mkdir(os.path.join(outdir, 'SegmentationObject'))
    return os.path.join(outdir, 'Annotations'), os.path.join(outdir, 'JPEGImages'), os.path.join(outdir, 'ImageSets', 'Main')

def generate_xml(stem, lines, img_size ):
    doc = Document()
    def append_xml_node_attr(child, parent=None, text=None):
        ele = doc.createElement(child)
        if not text is None:
            text_node = doc.createTextNode(text)
            ele.appendChild(text_node)
        parent = doc if parent is None else parent
        parent.appendChild(ele)
        return ele

    img_name=stem + '.jpg'
    #create header
    annotation = append_xml_node_attr('annotation')
    append_xml_node_attr('folder', parent=annotation, text='JPEGImages')
    append_xml_node_attr('filename',parent=annotation, text=img_name)
    source = append_xml_node_attr('source', parent=annotation)
    append_xml_node_attr('database', parent=source, text='Unknown')
    #append_xml_node_attr('annotation', parent=source, text='text')
    #ower = append_xml_node_attr('owner', parent=annotation)
    #append_xml_node_attr('name', parent=ower, text='lalala')
    size = append_xml_node_attr('size', parent=annotation)
    append_xml_node_attr('width', size, str(img_size[0]))
    append_xml_node_attr('height', size, str(img_size[1]))
    append_xml_node_attr('depth', size, str(img_size[2]))
    append_xml_node_attr('segmented', parent=annotation, text='0')

    # create objects
    for line in lines:
        #splitted_line = line.strip().split(" ")
        splitted_line = line
        cls = splitted_line[0]

        obj = append_xml_node_attr('object', parent=annotation)
        #x1, y1, x2, y2 = int(splitted_line[1]) + 1, int(splitted_line[2]) + 1, int(splitted_line[3]) + 1, int(splitted_line[4]) + 1
        x1, y1, x2, y2 = int(splitted_line[1]), int(splitted_line[2]), int(splitted_line[3]), int(splitted_line[4])
        truncated = int(0)
        difficult = int(0)
        append_xml_node_attr('name', parent=obj, text=cls)
        #append_xml_node_attr('pose', parent=obj, text='Unspecified')
        append_xml_node_attr('pose', parent=obj, text='Frontal')
        append_xml_node_attr('occluded', parent=obj, text='0')
        append_xml_node_attr('truncated', parent=obj, text=str(truncated))
        append_xml_node_attr('difficult', parent=obj, text=str(difficult))
        bndbox = append_xml_node_attr('bndbox', parent=obj)
        append_xml_node_attr('xmin', parent=bndbox, text=str(x1))
        append_xml_node_attr('ymin', parent=bndbox, text=str(y1))
        append_xml_node_attr('xmax', parent=bndbox, text=str(x2))
        append_xml_node_attr('ymax', parent=bndbox, text=str(y2))
    return doc
if __name__ == '__main__':
    binary_target = "generate_text_in_image"
    run_sys_command(("make %s")%(binary_target))
    img_size=[640, 480, 3]
    _outdir="TextDataset"
    _dest_label_dir, _dest_img_dir, _dest_set_dir = build_voc_dirs(_outdir)
    for stem in ["%04d" % i for i in range(7000)]:
        out_img_name=os.path.join(_dest_img_dir, stem + '.jpg')
        out_xml_name=os.path.join(_dest_label_dir, stem + '.xml')
        couts = run_sys_command(("./generate_text_in_image %s")%(out_img_name))
        couts = couts.strip().split()
        if len(couts) % 5 is not 0:
            raise Exception('Get generate_text_in_image error !')
        lines = []
        num_texts = len(couts) / 5
        for i in range(num_texts):
            lines.append(couts[i * 5 : i * 5 + 5])
        if(len(lines) < 1):
            raise Exception('generate_text_in_image error !')
        doc = generate_xml(stem, lines, img_size)
        with open( out_xml_name, 'w' ) as f:
            f.write(doc.toprettyxml(indent='    '))

