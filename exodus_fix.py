#!/usr/bin/env python

import vtk
import sys

fname=sys.argv[1]

surface_ids = []
region_ids = []
r=vtk.vtkExodusIIReader()
r.SetFileName(fname)
r.UpdateInformation()
r.GenerateGlobalNodeIdArrayOn()
r.GenerateGlobalElementIdArrayOn()
if vtk.VTK_MAJOR_VERSION<6:
    r.ExodusModelMetadataOn()
    r.PackExodusModelOntoOutputOn()
for i in range(r.GetNumberOfSideSetArrays()):
    name=r.GetSideSetArrayName(i)
    r.SetSideSetArrayStatus(name,1)
    surface_ids.append(int(name.split(":")[-1],base=0))
for i in range(r.GetNumberOfElementBlockArrays()):
    name = r.GetElementBlockArrayName(i)
    for text in name.split(" "):
        try:
            id = int(text,base=0)
            region_ids.append(id)
            break
        except:
            continue
r.Update()


data=r.GetOutput()

node_dict={}
ele_edge_dict=[]
ele_face_dict=[]
ele_vol_dict=[]

n=1
def f():
    global n
    a=n
    n=n+1
    return a

for j in range(data.GetBlock(4).GetNumberOfBlocks()):
    ug=data.GetBlock(4).GetBlock(j)
    for k in range(ug.GetNumberOfPoints()):
        lnodes={}
        p=ug.GetPoint(k)
        N=node_dict.get(p)
        if N==None:
            node_dict[p]=n
            n+=1
        lnodes[k]=node_dict.get(p)
    for k in range(ug.GetNumberOfCells()):
        c=ug.GetCell(k)
        cp=ug.GetCell(k).GetPoints()
        if c.GetCellType()==vtk.VTK_LINE:
            ele_edge_dict.append((surface_ids[j],
                              node_dict[cp.GetPoint(0)],
                              node_dict[cp.GetPoint(1)]))
        elif c.GetCellType()==vtk.VTK_TRIANGLE:
            ele_face_dict.append((surface_ids[j],
                              node_dict[cp.GetPoint(0)],
                              node_dict[cp.GetPoint(1)],
                              node_dict[cp.GetPoint(2)]))
                                        

for j in range(data.GetBlock(0).GetNumberOfBlocks()):
    ug=data.GetBlock(0).GetBlock(j)
    for k in range(ug.GetNumberOfPoints()):
        lnodes={}
        p=ug.GetPoint(k)
        N=node_dict.get(p)
        if N==None:
            node_dict[p]=n
            n+=1
        lnodes[k]=node_dict.get(p)
    for k in range(ug.GetNumberOfCells()):
        ug.GetCell(k)
        c=ug.GetCell(k)
        cp=ug.GetCell(k).GetPoints()
        if c.GetCellType()==vtk.VTK_TRIANGLE:
            ele_face_dict.append((region_ids[j],
                              node_dict[cp.GetPoint(0)],
                              node_dict[cp.GetPoint(1)],
                              node_dict[cp.GetPoint(2)]))
        elif c.GetCellType()==vtk.VTK_TETRA:
            ele_vol_dict.append((region_ids[j],
                              node_dict[cp.GetPoint(0)],
                              node_dict[cp.GetPoint(1)],
                              node_dict[cp.GetPoint(2)],
                              node_dict[cp.GetPoint(3)]))
        
rnode_dict={}
for k,v in node_dict.items():
    rnode_dict[v]=k

newname=".".join(fname.split(".")[:-1])+'.msh'

file=open(newname,'w')

file.writelines(("$MeshFormat\n",
                 "2.2 0 8\n",
                 "$EndMeshFormat\n",
                 "$Nodes\n",
                "%d\n"%len(node_dict)))
for k in range(len(node_dict)):
    p=rnode_dict[k+1]
    file.write("%d %f %f %f\n"%(k+1,p[0],p[1],p[2]))
file.write("$EndNodes\n")
file.write("$Elements\n")
file.write("%d\n"%(len(ele_edge_dict)
                   +len(ele_face_dict)
                   +len(ele_vol_dict)))
for k,ele in enumerate(ele_edge_dict):
    file.write("%d 1 2 %d %d %d %d\n"%(k+1,ele[0],ele[0],ele[1],ele[2]))
N=len(ele_edge_dict)
for k,ele in enumerate(ele_face_dict):
    file.write("%d 2 2 %d %d %d %d %d\n"%(k+1+N,ele[0],ele[0],ele[1],ele[2],ele[3]))
N+=len(ele_face_dict)
for k,ele in enumerate(ele_vol_dict):
    file.write("%d 4 2 %d %d %d %d %d %d\n"%(k+1+N,ele[0],ele[0],ele[1],ele[2],ele[3],ele[4]))
file.write("$EndElements\n")
file.close()

